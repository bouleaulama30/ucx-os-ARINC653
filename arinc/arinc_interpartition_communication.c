#include "ucx.h"


static struct node_s *find_sampling_port_by_name(struct node_s *node, void *arg){
    struct sampling_port_s *port = node->data;
    const char *name = (const char *) arg;

    if(strcmp(port->sampling_port_name, name) == 0){
        return node;
    }
    return 0;
}

static struct node_s *find_sampling_port_node_by_name(struct pcb_s *partition, SAMPLING_PORT_NAME_TYPE SAMPLING_PORT_NAME){
       return list_foreach(partition->communication_sampling_ports, find_sampling_port_by_name, (void *)SAMPLING_PORT_NAME);
}

static struct node_s *find_sampling_port_by_id(struct node_s *node, void *arg){
    struct sampling_port_s *port = node->data;
    SAMPLING_PORT_ID_TYPE id = (SAMPLING_PORT_ID_TYPE) arg;

    if(id == port->sampling_port_id){
       return node;
    }
    return 0;
}

static struct node_s *find_sampling_port_node_by_id(struct pcb_s *partition, SAMPLING_PORT_ID_TYPE SAMPLING_PORT_ID){
       return list_foreach(partition->communication_sampling_ports, find_sampling_port_by_id, (void *)SAMPLING_PORT_ID);
}

static struct node_s *find_queuing_port_by_name(struct node_s *node, void *arg){
    struct queuing_port_s *port = node->data;
    const char *name = (const char *) arg;

    if(strcmp(port->queuing_port_name, name) == 0){
        return node;
    }
    return 0;
}

static struct node_s *find_queuing_port_node_by_name(struct pcb_s *partition, QUEUING_PORT_NAME_TYPE QUEUING_PORT_NAME){
       return list_foreach(partition->communication_queuing_ports, find_queuing_port_by_name, (void *)QUEUING_PORT_NAME);
}

static struct node_s *find_queuing_port_by_id(struct node_s *node, void *arg){
    struct queuing_port_s *port = node->data;
    QUEUING_PORT_ID_TYPE id = (QUEUING_PORT_ID_TYPE) arg;

    if(id == port->queuing_port_id){
       return node;
    }
    return 0;
}

static struct node_s *find_queuing_port_node_by_id(struct pcb_s *partition, QUEUING_PORT_ID_TYPE QUEUING_PORT_ID){
       return list_foreach(partition->communication_queuing_ports, find_queuing_port_by_id, (void *)QUEUING_PORT_ID);
}


int check_port_in_conf_table(NAME_TYPE PORT_NAME){
       for(int i = 0; i<routing_table_size; i++){
              if(strcmp(PORT_NAME, system_port_table[i].port_name) == 0)
                     return i;
       }
       return -1;
}

int check_max_message_size_in_conf(int index_conf_table, MESSAGE_SIZE_TYPE MAX_MESSAGE_SIZE){
       if(MAX_MESSAGE_SIZE == system_port_table[index_conf_table].messageSizeBytes)
              return 1;
       return 0;
}

int check_port_direction_in_conf(int index_conf_table, PORT_DIRECTION_TYPE PORT_DIRECTION){
       if(PORT_DIRECTION == system_port_table[index_conf_table].port_direction)
              return 1;
       return 0;
}

void list_insert_sorted(struct list_s *list, struct process_s *process) {
       struct node_s *new_node = malloc(sizeof(struct node_s));
       new_node->data = process;
       new_node->next = NULL;

       struct node_s *current = list->head;
       while (current->next != list->tail && ((struct process_s *)current->next->data)->processus_status->CURRENT_PRIORITY >= process->processus_status->CURRENT_PRIORITY) {
              current = current->next;
       }
       new_node->next = current->next;
       current->next = new_node;
       list->length++;
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

       int index_conf_table = check_port_in_conf_table(SAMPLING_PORT_NAME);
       if (index_conf_table == -1){
              *RETURN_CODE = INVALID_CONFIG;
              return;
       }


       if (find_sampling_port_node_by_name(partition, SAMPLING_PORT_NAME)){
              *RETURN_CODE = NO_ACTION;
              return;
       }

       if (MAX_MESSAGE_SIZE <= 0 || !check_max_message_size_in_conf(index_conf_table, MAX_MESSAGE_SIZE)){
              *RETURN_CODE = INVALID_CONFIG;
              return;
       }

       if ((PORT_DIRECTION != SOURCE && PORT_DIRECTION != DESTINATION) || !check_port_direction_in_conf(index_conf_table, PORT_DIRECTION)){
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
       
       sampling_port->channel = system_port_table[index_conf_table].sampling_channel;
       
       list_pushback(partition->communication_sampling_ports, sampling_port);
       *SAMPLING_PORT_ID = port_ID;
       *RETURN_CODE = NO_ERROR;

}

void WRITE_SAMPLING_MESSAGE (
       /*in */ SAMPLING_PORT_ID_TYPE      SAMPLING_PORT_ID,
       /*in */ MESSAGE_ADDR_TYPE          MESSAGE_ADDR,     /* by reference */
       /*in */ MESSAGE_SIZE_TYPE          LENGTH,
       /*out*/ RETURN_CODE_TYPE           *RETURN_CODE ){

       struct pcb_s* partition = get_current_partition();
       
       struct node_s *sampling_port_node = find_sampling_port_node_by_id(partition, SAMPLING_PORT_ID); 
       if (!sampling_port_node){
              *RETURN_CODE = INVALID_PARAM;
              return;
       }

       struct sampling_port_s *sampling_port = sampling_port_node->data;

       if (LENGTH > sampling_port->sampling_port_status->MAX_MESSAGE_SIZE){
              *RETURN_CODE = INVALID_CONFIG;
              return;
       }

       if (LENGTH <= 0){
              *RETURN_CODE = INVALID_PARAM;
              return;
       }

       if (sampling_port->sampling_port_status->PORT_DIRECTION != SOURCE){
              *RETURN_CODE = INVALID_MODE;
              return;
       }

       struct krnl_sampling_channel *channel = sampling_port->channel;
       
       memcpy(channel->buffer, MESSAGE_ADDR, LENGTH);
       channel->current_message_size = LENGTH;
       channel->last_update_time = (SYSTEM_TIME_TYPE)ucx_uptime();

       *RETURN_CODE = NO_ERROR;
}

void READ_SAMPLING_MESSAGE (
       /*in */ SAMPLING_PORT_ID_TYPE      SAMPLING_PORT_ID,
       /*in */ MESSAGE_ADDR_TYPE          MESSAGE_ADDR,
               /* The message address is passed IN, although */
               /* the respective message is passed OUT       */
       /*out*/ MESSAGE_SIZE_TYPE          *LENGTH,
       /*out*/ VALIDITY_TYPE              *VALIDITY,
       /*out*/ RETURN_CODE_TYPE           *RETURN_CODE ){
       struct pcb_s* partition = get_current_partition();
       
       struct node_s *sampling_port_node = find_sampling_port_node_by_id(partition, SAMPLING_PORT_ID); 
       if (!sampling_port_node){
              *RETURN_CODE = INVALID_PARAM;
              return;
       }

       struct sampling_port_s* sampling_port = sampling_port_node->data;
       if(sampling_port->sampling_port_status->PORT_DIRECTION != DESTINATION){
              *RETURN_CODE = INVALID_MODE;
              return;
       }

       struct krnl_sampling_channel *channel = sampling_port->channel;
       if(!channel->current_message_size){
              *LENGTH = 0;
              *VALIDITY = INVALID;
              *RETURN_CODE = NO_ACTION;
       }
       else {
              memcpy(MESSAGE_ADDR, channel->buffer, channel->current_message_size);
              *LENGTH = channel->current_message_size;
              SYSTEM_TIME_TYPE age_message = (SYSTEM_TIME_TYPE)ucx_uptime() - channel->last_update_time;
              if(age_message <= sampling_port->sampling_port_status->REFRESH_PERIOD)
                     *VALIDITY = VALID;
              else
                     *VALIDITY = INVALID;
              *RETURN_CODE = NO_ERROR;
       sampling_port->sampling_port_status->LAST_MSG_VALIDITY = *VALIDITY;
       }
}

void GET_SAMPLING_PORT_ID (
       /*in */ SAMPLING_PORT_NAME_TYPE    SAMPLING_PORT_NAME,
       /*out*/ SAMPLING_PORT_ID_TYPE      *SAMPLING_PORT_ID,
       /*out*/ RETURN_CODE_TYPE           *RETURN_CODE ){

       struct pcb_s* partition = get_current_partition();
       
       struct node_s *sampling_port_node = find_sampling_port_node_by_name(partition, SAMPLING_PORT_NAME); 
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
       
       struct node_s *sampling_port_node = find_sampling_port_node_by_id(partition, SAMPLING_PORT_ID); 
       if (!sampling_port_node){
              *RETURN_CODE = INVALID_PARAM;
              return;
       }

       struct sampling_port_s *sampling_port = sampling_port_node->data;
       *SAMPLING_PORT_STATUS = *sampling_port->sampling_port_status;
       *RETURN_CODE = NO_ERROR;
}


void CREATE_QUEUING_PORT (
       /*in */ QUEUING_PORT_NAME_TYPE   QUEUING_PORT_NAME,
       /*in */ MESSAGE_SIZE_TYPE        MAX_MESSAGE_SIZE,
       /*in */ MESSAGE_RANGE_TYPE       MAX_NB_MESSAGE,
       /*in */ PORT_DIRECTION_TYPE      PORT_DIRECTION,
       /*in */ QUEUING_DISCIPLINE_TYPE  QUEUING_DISCIPLINE,
       /*out*/ QUEUING_PORT_ID_TYPE     *QUEUING_PORT_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){

       struct pcb_s *partition = get_current_partition();
       if(partition->queuing_port_count + 1 > MAX_NUMBER_OF_QUEUING_PORTS){
              *RETURN_CODE = INVALID_CONFIG;
              return;
       }

       int index_conf_table = check_port_in_conf_table(QUEUING_PORT_NAME);
       if (index_conf_table == -1){
              *RETURN_CODE = INVALID_CONFIG;
              return;
       }


       if (find_queuing_port_node_by_name(partition, QUEUING_PORT_NAME)){
              *RETURN_CODE = NO_ACTION;
              return;
       }

       if (MAX_MESSAGE_SIZE <= 0 || !check_max_message_size_in_conf(index_conf_table, MAX_MESSAGE_SIZE)){
              *RETURN_CODE = INVALID_CONFIG;
              return;
       }

       if (MAX_NB_MESSAGE <= 0 || MAX_NB_MESSAGE > system_port_table[index_conf_table].max_nb_message){
              *RETURN_CODE = INVALID_CONFIG;
              return;
       }

       if ((PORT_DIRECTION != SOURCE && PORT_DIRECTION != DESTINATION) || !check_port_direction_in_conf(index_conf_table, PORT_DIRECTION)){
              *RETURN_CODE = INVALID_CONFIG;
              return;
       }

       if (QUEUING_DISCIPLINE != FIFO && QUEUING_DISCIPLINE != PRIORITY){
              *RETURN_CODE = INVALID_CONFIG;
              return;
       }

       if (partition->status->OPERATING_MODE == NORMAL){
              *RETURN_CODE = INVALID_CONFIG;
              return;
       }

       QUEUING_PORT_ID_TYPE port_ID = ++(partition->queuing_port_count);

       struct queuing_port_s *queuing_port = malloc(sizeof(struct queuing_port_s));
       QUEUING_PORT_STATUS_TYPE *queuing_port_status = malloc(sizeof(QUEUING_PORT_STATUS_TYPE));

       queuing_port_status->MAX_MESSAGE_SIZE = MAX_MESSAGE_SIZE;
       queuing_port_status->MAX_NB_MESSAGE = MAX_NB_MESSAGE;
       queuing_port_status->PORT_DIRECTION = PORT_DIRECTION;
 
       queuing_port->queuing_port_status = queuing_port_status;
       queuing_port->partition_id = system_port_table[index_conf_table].partition_id;
       queuing_port->queuing_port_id = port_ID;
       queuing_port->QUEUING_DISCIPLINE = QUEUING_DISCIPLINE;
       queuing_port->waiting_processes = list_create();

       strncpy(queuing_port->queuing_port_name, QUEUING_PORT_NAME, sizeof(queuing_port->queuing_port_name) - 1);
       queuing_port->queuing_port_name[sizeof(queuing_port->queuing_port_name) - 1] = '\0';
       
       queuing_port->channel = system_port_table[index_conf_table].queuing_channel;
       
       list_pushback(partition->communication_queuing_ports, queuing_port);
       *QUEUING_PORT_ID = port_ID;
       *RETURN_CODE = NO_ERROR;
}

void SEND_QUEUING_MESSAGE (
       /*in */ QUEUING_PORT_ID_TYPE     QUEUING_PORT_ID,
       /*in */ MESSAGE_ADDR_TYPE        MESSAGE_ADDR,       /* by reference */
       /*in */ MESSAGE_SIZE_TYPE        LENGTH,
       /*in */ SYSTEM_TIME_TYPE         TIME_OUT,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE){

       struct pcb_s *partition = get_current_partition();
       struct node_s *queuing_port_node = find_queuing_port_node_by_id(partition, QUEUING_PORT_ID);
       if (!queuing_port_node){
              *RETURN_CODE = INVALID_PARAM;
              return;
       }

       struct queuing_port_s *queuing_port = queuing_port_node->data;

       if(TIME_OUT < 0 || time_overflow(ucx_uptime() + (SYSTEM_TIME_TYPE)TIME_OUT)){
              *RETURN_CODE = INVALID_PARAM;
              return;
       }

       if(LENGTH >= queuing_port->queuing_port_status->MAX_MESSAGE_SIZE){
              *RETURN_CODE = INVALID_CONFIG;
              return;
       }

       if(LENGTH <= 0){
              *RETURN_CODE = INVALID_PARAM;
              return;
       }

       if(queuing_port->queuing_port_status->PORT_DIRECTION != SOURCE){
              *RETURN_CODE = INVALID_MODE;
              return;
       }

       struct process_s *current_process = partition->process_current->data;
       struct krnl_queuing_channel_s *channel = queuing_port->channel;
       if(channel->current_nb_messages < channel->max_nb_messages && queuing_port->waiting_processes->length == 0){
              channel->current_nb_messages++;
              uint32_t write_index = channel->write_index;
              memcpy(channel->buffer_data + write_index * channel->max_message_size, MESSAGE_ADDR, LENGTH);
              channel->buffer_sizes[write_index] = LENGTH;
              channel->write_index = (write_index + 1) % channel->max_nb_messages;
              *RETURN_CODE = NO_ERROR;
       }
       else if (TIME_OUT == 0){
              *RETURN_CODE = NOT_AVAILABLE;
       }
       // cd current process own mutex
       // else if ()
       else {
              if (TIME_OUT != INFINITE_TIME_VALUE){
                     current_process->time_counter = (SYSTEM_TIME_TYPE)ucx_uptime() + (SYSTEM_TIME_TYPE)TIME_OUT;
              }
              current_process->processus_status->PROCESS_STATE = WAITING;
              // to do implementer selon la discipline de la file d'attente
              if (queuing_port->QUEUING_DISCIPLINE == PRIORITY){
                     // to do insert process in waiting_processes list according to its priority
                     list_insert_sorted(queuing_port->waiting_processes, current_process);
              }
              else {
                     list_pushback(queuing_port->waiting_processes, current_process);
              }
              current_process->waiting_queuing_port = queuing_port;
              yield_to_partition(partition, current_process);
              if(current_process->time_counter == 0){
                     *RETURN_CODE = TIMED_OUT;
              } else {
                     channel->current_nb_messages++;
                     uint32_t write_index = channel->write_index;
                     memcpy(channel->buffer_data + write_index * channel->max_message_size, MESSAGE_ADDR, LENGTH);
                     channel->buffer_sizes[write_index] = LENGTH;
                     channel->write_index = (write_index + 1) % channel->max_nb_messages;

                     if(TIME_OUT != INFINITE_TIME_VALUE){
                            current_process->time_counter = 0;
                     }
                     *RETURN_CODE = NO_ERROR;
              }
       }
}

void RECEIVE_QUEUING_MESSAGE (
       /*in */ QUEUING_PORT_ID_TYPE     QUEUING_PORT_ID,
       /*in */ SYSTEM_TIME_TYPE         TIME_OUT,
       /*in */ MESSAGE_ADDR_TYPE        MESSAGE_ADDR,
               /* The message address is passed IN, although */
               /* the respective message is passed OUT       */
       /*out*/ MESSAGE_SIZE_TYPE        *LENGTH,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){

       struct pcb_s *partition = get_current_partition();
       struct node_s *queuing_port_node = find_queuing_port_node_by_id(partition, QUEUING_PORT_ID);
       if (!queuing_port_node){
              *RETURN_CODE = INVALID_PARAM;
              return;
       }

       struct queuing_port_s *queuing_port = queuing_port_node->data;

       if(TIME_OUT < 0 || time_overflow(ucx_uptime() + (SYSTEM_TIME_TYPE)TIME_OUT)){
              *RETURN_CODE = INVALID_PARAM;
              return;
       }

       if(queuing_port->queuing_port_status->PORT_DIRECTION != DESTINATION){
              *RETURN_CODE = INVALID_MODE;
              return;
       }

       struct process_s *current_process = partition->process_current->data;
       struct krnl_queuing_channel_s *channel = queuing_port->channel;
       if(channel->current_nb_messages > 0){
              channel->current_nb_messages--;
              uint32_t read_index = channel->read_index;
              memcpy(MESSAGE_ADDR, channel->buffer_data + read_index * channel->max_message_size, channel->buffer_sizes[read_index]);
              *LENGTH = channel->buffer_sizes[read_index];
              channel->read_index = (read_index + 1) % channel->max_nb_messages;
              *RETURN_CODE = NO_ERROR;
       }
       else if (TIME_OUT == 0){
              *LENGTH = 0;
              *RETURN_CODE = NOT_AVAILABLE;
       }
       // cd current process own mutex
       // else if ()
       else {
              if (TIME_OUT != INFINITE_TIME_VALUE){
                     current_process->time_counter = (SYSTEM_TIME_TYPE)ucx_uptime() + (SYSTEM_TIME_TYPE)TIME_OUT;
              }
              current_process->processus_status->PROCESS_STATE = WAITING;
              if (queuing_port->QUEUING_DISCIPLINE == PRIORITY){
                     // to do insert process in waiting_processes list according to its priority
                     list_insert_sorted(queuing_port->waiting_processes, current_process);

              }
              else {
                     list_pushback(queuing_port->waiting_processes, current_process);
              }
              current_process->waiting_queuing_port = queuing_port;
              yield_to_partition(partition, current_process);
              if(current_process->time_counter == 0){
                     *LENGTH = 0;
                     *RETURN_CODE = TIMED_OUT;
              } else {
                     if(TIME_OUT != INFINITE_TIME_VALUE){
                            current_process->time_counter = 0;
                     }
                     channel->current_nb_messages--;
                     uint32_t read_index = channel->read_index;
                     memcpy(MESSAGE_ADDR, channel->buffer_data + read_index * channel->max_message_size, channel->buffer_sizes[read_index]);
                     *LENGTH = channel->buffer_sizes[read_index];
                     channel->read_index = (read_index + 1) % channel->max_nb_messages;

                     *RETURN_CODE = NO_ERROR;
              }
       }
}

void GET_QUEUING_PORT_ID (
       /*in */ QUEUING_PORT_NAME_TYPE   QUEUING_PORT_NAME,
       /*out*/ QUEUING_PORT_ID_TYPE     *QUEUING_PORT_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){

       struct pcb_s* partition = get_current_partition();
       
       struct node_s *queuing_port_node = find_queuing_port_node_by_name(partition, QUEUING_PORT_NAME); 
       if (!queuing_port_node){
              *RETURN_CODE = INVALID_CONFIG;
              return;
       }

       struct queuing_port_s* queuing_port = queuing_port_node->data;
       *QUEUING_PORT_ID = queuing_port->queuing_port_id;

       *RETURN_CODE = NO_ERROR;
}

void GET_QUEUING_PORT_STATUS (
       /*in */ QUEUING_PORT_ID_TYPE     QUEUING_PORT_ID,
       /*out*/ QUEUING_PORT_STATUS_TYPE *QUEUING_PORT_STATUS,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
       struct pcb_s* partition = get_current_partition();
       
       struct node_s *queuing_port_node = find_queuing_port_node_by_id(partition, QUEUING_PORT_ID); 
       if (!queuing_port_node){
              *RETURN_CODE = INVALID_PARAM;
              return;
       }

       struct queuing_port_s *queuing_port = queuing_port_node->data;
       *QUEUING_PORT_STATUS = *queuing_port->queuing_port_status;
       *RETURN_CODE = NO_ERROR;
}


void CLEAR_QUEUING_PORT (
       /*in */ QUEUING_PORT_ID_TYPE     QUEUING_PORT_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
       
       struct pcb_s* partition = get_current_partition();
       
       struct node_s *queuing_port_node = find_queuing_port_node_by_id(partition, QUEUING_PORT_ID); 
       if (!queuing_port_node){
              *RETURN_CODE = INVALID_PARAM;
              return;
       }

       struct queuing_port_s *queuing_port = queuing_port_node->data;

       if(queuing_port->queuing_port_status->PORT_DIRECTION != DESTINATION){
              *RETURN_CODE = INVALID_MODE;
              return;
       }

       struct krnl_queuing_channel_s *channel = queuing_port->channel;

       channel->current_nb_messages = 0;
       channel->read_index = 0;
       channel->write_index = 0;
       *RETURN_CODE = NO_ERROR;
}