#include "ucx.h"

int find_blackboard_by_name(struct pcb_s *partition, char* name){
    for (int i = 0; i < partition->blackboard_count; i++){
        struct blackboard_s *bb = &partition->blackboards[i];
        if (strcmp(bb->blackboard_name, name) == 0){
            return i;
        }
    }
    return -1;  
}

int find_blackboard_by_id(struct pcb_s *partition, BLACKBOARD_ID_TYPE id){
    for (int i = 0; i < partition->blackboard_count; i++){
        struct blackboard_s *bb = &partition->blackboards[i];
        if (bb->blackboard_id == id){
            return i;
        }
    }
    return -1;  
}

static struct node_s *release_waiting_bb_process(struct node_s *node, void *arg){
    struct list_s *waiting_processes = (struct list_s *) arg;
    struct process_s *process = node->data;
    if (process->time_counter != 0){
        process->time_counter = 0;
    }

    list_remove(waiting_processes, node);
    process->processus_status->PROCESS_STATE = READY;
    return 0;
}

void CREATE_BLACKBOARD (
       /*in */ BLACKBOARD_NAME_TYPE     BLACKBOARD_NAME,
       /*in */ MESSAGE_SIZE_TYPE        MAX_MESSAGE_SIZE,
       /*out*/ BLACKBOARD_ID_TYPE       *BLACKBOARD_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){

    struct pcb_s *partition = get_current_partition();
    
    if (partition->blackboard_count + 1 > partition->max_blackboards || partition->max_blackboard_data_size < MAX_MESSAGE_SIZE){
        *RETURN_CODE = INVALID_CONFIG;
        return;
    }
    
    if (find_blackboard_by_name(partition, BLACKBOARD_NAME) != -1){
        *RETURN_CODE = NO_ACTION;
        return;
    }

    if (MAX_MESSAGE_SIZE <= 0){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    if (partition->status->OPERATING_MODE == NORMAL){
         *RETURN_CODE = INVALID_MODE;
         return;
    }

    struct blackboard_s *bb = &partition->blackboards[partition->blackboard_count++];
    strncpy(bb->blackboard_name, BLACKBOARD_NAME, sizeof(bb->blackboard_name) - 1);
    bb->blackboard_name[sizeof(bb->blackboard_name) - 1] = '\0';
    bb->blackboard_id = partition->blackboard_count;
    bb->partition_id = partition->status->IDENTIFIER;
    bb->waiting_processes = list_create();
    bb->blackboard_status.EMPTY_INDICATOR = EMPTY;
    bb->blackboard_status.MAX_MESSAGE_SIZE = MAX_MESSAGE_SIZE;
    bb->blackboard_status.WAITING_PROCESSES = 0; 

    *BLACKBOARD_ID = bb->blackboard_id;
    *RETURN_CODE = NO_ERROR;
}

void DISPLAY_BLACKBOARD (
       /*in */ BLACKBOARD_ID_TYPE       BLACKBOARD_ID,
       /*in */ MESSAGE_ADDR_TYPE        MESSAGE_ADDR,       /* by reference */
       /*in */ MESSAGE_SIZE_TYPE        LENGTH,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
    struct pcb_s *partition = get_current_partition();
    int index = find_blackboard_by_id(partition, BLACKBOARD_ID);
    if (index == -1){
        *RETURN_CODE = INVALID_CONFIG;
        return;
    }

    struct blackboard_s *bb = &partition->blackboards[index];
    if (LENGTH > bb->blackboard_status.MAX_MESSAGE_SIZE){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }
    
    if (LENGTH <= 0){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    bb->blackboard_status.EMPTY_INDICATOR = OCCUPIED;
    memcpy(partition->blackboards_data + (index * partition->max_blackboard_data_size), MESSAGE_ADDR, LENGTH);
    if(bb->waiting_processes->length > 0){
        list_foreach(bb->waiting_processes, release_waiting_bb_process, bb->waiting_processes);
        yield_to_partition(partition, NULL);
    }
    *RETURN_CODE = NO_ERROR;
}

void READ_BLACKBOARD (
       /*in */ BLACKBOARD_ID_TYPE       BLACKBOARD_ID,
       /*in */ SYSTEM_TIME_TYPE         TIME_OUT,
       /*in */ MESSAGE_ADDR_TYPE        MESSAGE_ADDR,
               /* The message address is passed IN, although */
               /* the respective message is passed OUT       */
       /*out*/ MESSAGE_SIZE_TYPE        *LENGTH,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

void CLEAR_BLACKBOARD (
       /*in */ BLACKBOARD_ID_TYPE       BLACKBOARD_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

void GET_BLACKBOARD_ID (
       /*in */ BLACKBOARD_NAME_TYPE     BLACKBOARD_NAME,
       /*out*/ BLACKBOARD_ID_TYPE       *BLACKBOARD_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
    
    struct pcb_s *partition = get_current_partition();
    int index = find_blackboard_by_name(partition, BLACKBOARD_NAME);
    if (index == -1){
        *RETURN_CODE = INVALID_CONFIG;
        return;
    }

    *BLACKBOARD_ID = partition->blackboards[index].blackboard_id;
    *RETURN_CODE = NO_ERROR;
}

void GET_BLACKBOARD_STATUS (
       /*in */ BLACKBOARD_ID_TYPE       BLACKBOARD_ID,
       /*out*/ BLACKBOARD_STATUS_TYPE   *BLACKBOARD_STATUS,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){

    struct pcb_s *partition = get_current_partition();
    int index = find_blackboard_by_id(partition, BLACKBOARD_ID);
    if (index == -1){
        *RETURN_CODE = INVALID_CONFIG;
        return;
    }

    *BLACKBOARD_STATUS = partition->blackboards[index].blackboard_status;
    *RETURN_CODE = NO_ERROR;
}