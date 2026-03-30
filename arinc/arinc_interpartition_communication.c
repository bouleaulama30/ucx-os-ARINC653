#include "ucx.h"


static struct node_s *find_port_by_name(struct node_s *node, void *arg){
    struct sampling_port_s *port = node->data;
    const char *name = (const char *) arg;

    if(strcmp(port->sampling_port_name, name) == 0){
        return node;
    }
    return 0;
}

static struct node_s *find_port_node_by_name(struct pcb_s *partition, SAMPLING_PORT_NAME_TYPE SAMPLING_PORT_NAME){
       return list_foreach(partition->communication_ports, find_port_by_name, (void *)SAMPLING_PORT_NAME);
}

static struct node_s *find_port_by_id(struct node_s *node, void *arg){
    struct sampling_port_s *port = node->data;
    SAMPLING_PORT_ID_TYPE id = (SAMPLING_PORT_ID_TYPE) arg;

    if(id == port->sampling_port_id){
        return node;
    }
    return 0;
}

static struct node_s *find_port_node_by_id(struct pcb_s *partition, SAMPLING_PORT_ID_TYPE SAMPLING_PORT_ID){
       return list_foreach(partition->communication_ports, find_port_by_id, (void *)SAMPLING_PORT_ID);
}

int check_port_in_conf_table(){
       return 1;
}

int check_max_message_size_in_conf(int index_conf_table, MESSAGE_SIZE_TYPE MAX_MESSAGE_SIZE){
       return 1;
}

int check_max_port_direction_in_conf(int index_conf_table, PORT_DIRECTION_TYPE PORT_DIRECTION){
       return 1;
}

void CREATE_SAMPLING_PORT (
       /*in */ SAMPLING_PORT_NAME_TYPE    SAMPLING_PORT_NAME,
       /*in */ MESSAGE_SIZE_TYPE          MAX_MESSAGE_SIZE,
       /*in */ PORT_DIRECTION_TYPE        PORT_DIRECTION,
       /*in */ SYSTEM_TIME_TYPE           REFRESH_PERIOD,
       /*out*/ SAMPLING_PORT_ID_TYPE      *SAMPLING_PORT_ID,
       /*out*/ RETURN_CODE_TYPE           *RETURN_CODE ){
       
       struct pcb_s *partition = get_current_partition();
       if(partition->sampling_port_count + 1 > MAX_NUMBER_OF_SAMPLING_PORTS){
              *RETURN_CODE = INVALID_CONFIG;
              return;
       }

       int index_conf_table = check_port_in_conf_table();
       if (index_conf_table == -1){
              *RETURN_CODE = INVALID_CONFIG;
              return;
       }


       if (find_port_node_by_name(partition, SAMPLING_PORT_NAME)){
              *RETURN_CODE = NO_ACTION;
              return;
       }

       if (MAX_MESSAGE_SIZE <= 0 || !check_max_message_size_in_conf(index_conf_table, MAX_MESSAGE_SIZE)){
              *RETURN_CODE = INVALID_CONFIG;
              return;
       }

       if ((PORT_DIRECTION != SOURCE && PORT_DIRECTION != DESTINATION) || !check_max_port_direction_in_conf(index_conf_table, PORT_DIRECTION)){
              *RETURN_CODE = INVALID_CONFIG;
              return;
       }

       if (REFRESH_PERIOD < 0 || time_overflow(ucx_uptime() + (SYSTEM_TIME_TYPE)REFRESH_PERIOD)){
              *RETURN_CODE = INVALID_CONFIG;
              return;
       }

       if (partition->status->OPERATING_MODE == NORMAL){
              *RETURN_CODE = INVALID_CONFIG;
              return;
       }

       SAMPLING_PORT_ID_TYPE port_ID = ++(partition->sampling_port_count);

       struct sampling_port_s *sampling_port = malloc(sizeof(struct sampling_port_s));
       SAMPLING_PORT_STATUS_TYPE *sampling_port_status = malloc(sizeof(SAMPLING_PORT_STATUS_TYPE));

       sampling_port_status->MAX_MESSAGE_SIZE = MAX_MESSAGE_SIZE;
       sampling_port_status->PORT_DIRECTION = PORT_DIRECTION;
       sampling_port_status->REFRESH_PERIOD = REFRESH_PERIOD;
 
       sampling_port->sampling_port_status = sampling_port_status;
       sampling_port->partition_id = system_port_table[index_conf_table].partition_id;
       sampling_port->sampling_port_id = port_ID;

       strncpy(sampling_port->sampling_port_name, SAMPLING_PORT_NAME, sizeof(sampling_port->sampling_port_name) - 1);
       sampling_port->sampling_port_name[sizeof(sampling_port->sampling_port_name) - 1] = '\0';
       
       sampling_port->channel = system_port_table[index_conf_table].channel;
       
       list_pushback(partition->communication_ports, sampling_port);
       *SAMPLING_PORT_ID = port_ID;
       *RETURN_CODE = NO_ERROR;

}

void WRITE_SAMPLING_MESSAGE (
       /*in */ SAMPLING_PORT_ID_TYPE      SAMPLING_PORT_ID,
       /*in */ MESSAGE_ADDR_TYPE          MESSAGE_ADDR,     /* by reference */
       /*in */ MESSAGE_SIZE_TYPE          LENGTH,
       /*out*/ RETURN_CODE_TYPE           *RETURN_CODE );

void READ_SAMPLING_MESSAGE (
       /*in */ SAMPLING_PORT_ID_TYPE      SAMPLING_PORT_ID,
       /*in */ MESSAGE_ADDR_TYPE          MESSAGE_ADDR,
               /* The message address is passed IN, although */
               /* the respective message is passed OUT       */
       /*out*/ MESSAGE_SIZE_TYPE          *LENGTH,
       /*out*/ VALIDITY_TYPE              *VALIDITY,
       /*out*/ RETURN_CODE_TYPE           *RETURN_CODE );

void GET_SAMPLING_PORT_ID (
       /*in */ SAMPLING_PORT_NAME_TYPE    SAMPLING_PORT_NAME,
       /*out*/ SAMPLING_PORT_ID_TYPE      *SAMPLING_PORT_ID,
       /*out*/ RETURN_CODE_TYPE           *RETURN_CODE ){

       struct pcb_s* partition = get_current_partition();
       
       struct node_s *sampling_port_node = find_port_node_by_name(partition, SAMPLING_PORT_NAME); 
       if (!sampling_port_node){
              *RETURN_CODE = INVALID_CONFIG;
              return;
       }

       struct sampling_port_s* sampling_port = sampling_port_node->data;
       *SAMPLING_PORT_ID = sampling_port->sampling_port_id;

       *RETURN_CODE = NO_ERROR;
}

void GET_SAMPLING_PORT_STATUS (
       /*in */ SAMPLING_PORT_ID_TYPE      SAMPLING_PORT_ID,
       /*out*/ SAMPLING_PORT_STATUS_TYPE  *SAMPLING_PORT_STATUS,
       /*out*/ RETURN_CODE_TYPE           *RETURN_CODE ){
       
       struct pcb_s* partition = get_current_partition();
       
       struct node_s *sampling_port_node = find_port_node_by_id(partition, SAMPLING_PORT_ID); 
       if (!sampling_port_node){
              *RETURN_CODE = INVALID_PARAM;
              return;
       }

       struct sampling_port_s *sampling_port = sampling_port_node->data;
       *SAMPLING_PORT_STATUS = *sampling_port->sampling_port_status;
       *RETURN_CODE = NO_ERROR;
}