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

int find_buffer_by_name(struct pcb_s *partition, char* name){
    for (int i = 0; i < partition->buffer_count; i++){
        struct buffer_s *buf = &partition->buffers[i];
        if (strcmp(buf->buffer_name, name) == 0){
            return i;
        }
    }
    return -1;  
}

int find_buffer_by_id(struct pcb_s *partition, BUFFER_ID_TYPE id){
    for (int i = 0; i < partition->buffer_count; i++){
        struct buffer_s *buf = &partition->buffers[i];
        if (buf->buffer_id == id){
            return i;
        }
    }
    return -1;  
}

int find_semaphore_by_name(struct pcb_s *partition, char* name){
    for (int i = 0; i < partition->semaphore_count; i++){
        struct semaphore_s *sem = &partition->semaphores[i];
        if (strcmp(sem->semaphore_name, name) == 0){
            return i;
        }
    }
    return -1;  
}

int find_semaphore_by_id(struct pcb_s *partition, SEMAPHORE_ID_TYPE id){
    for (int i = 0; i < partition->semaphore_count; i++){
        struct semaphore_s *sem = &partition->semaphores[i];
        if (sem->semaphore_id == id){
            return i;
        }
    }
    return -1;  
}

int find_event_by_name(struct pcb_s *partition, char* name){
    for (int i = 0; i < partition->event_count; i++){
        struct event_s *event = &partition->events[i];
        if (strcmp(event->event_name, name) == 0){
            return i;
        }
    }
    return -1;  
}

int find_event_by_id(struct pcb_s *partition, EVENT_ID_TYPE id){
    for (int i = 0; i < partition->event_count; i++){
        struct event_s *event = &partition->events[i];
        if (event->event_id == id){
            return i;
        }
    }
    return -1;  
}

int find_mutex_by_name(struct pcb_s *partition, char* name){
    for (int i = 0; i < partition->mutex_count; i++){
        struct mutex_s *mutex = &partition->mutexes[i];
        if (strcmp(mutex->mutex_name, name) == 0){
            return i;
        }
    }
    return -1;  
}

int find_mutex_by_id(struct pcb_s *partition, MUTEX_ID_TYPE id){
    for (int i = 0; i < partition->mutex_count; i++){
        struct mutex_s *mutex = &partition->mutexes[i];
        if (mutex->mutex_id == id){
            return i;
        }
    }
    return -1;  
}

static struct node_s *release_waiting_bb_process(struct node_s *node, void *arg){
    struct list_s *waiting_processes = (struct list_s *) arg;
    struct process_s *process = node->data;
    if (process->time_counter != 0){
        // on considère que le processus a un timeout lié à la lecture du blackboard, on le désactive
        process->time_counter = INFINITE_TIME_VALUE;
    }

    list_remove(waiting_processes, node);
    process->processus_status->PROCESS_STATE = READY;
    process->waiting_blackboard = NULL;
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
    partition->blackboards_size_data[index] = LENGTH;
    int is_waiting_processes = bb->blackboard_status.WAITING_PROCESSES > 0;
    while (bb->waiting_processes->length > 0) {
        // 1. Prendre le premier noeud (la tête)
        struct node_s *first_node = bb->waiting_processes->head->next;
        struct process_s *woken_process = first_node->data;
        
        // 2. Annuler son timer
        if (woken_process->time_counter != 0) {
            woken_process->time_counter = INFINITE_TIME_VALUE;
        }
        
        // 3. Le retirer proprement de la liste
        list_remove(bb->waiting_processes, first_node);
        
        // 4. Le réveiller
        woken_process->processus_status->PROCESS_STATE = READY;
        bb->blackboard_status.WAITING_PROCESSES--;
        woken_process->waiting_blackboard = NULL;
    }
    if (is_waiting_processes) {
        struct process_s *current_process = partition->process_current->data;
        yield_to_partition(partition, current_process);
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
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
    struct pcb_s *partition = get_current_partition();
    int index = find_blackboard_by_id(partition, BLACKBOARD_ID);
    if (index == -1){
        *RETURN_CODE = INVALID_CONFIG;
        return;
    }

    if(TIME_OUT < 0 || time_overflow(ucx_uptime() + (SYSTEM_TIME_TYPE)TIME_OUT)){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    struct process_s *current_process = partition->process_current->data;
    struct blackboard_s *bb = &partition->blackboards[index];
    if (bb->blackboard_status.EMPTY_INDICATOR == OCCUPIED){
        memcpy(MESSAGE_ADDR, partition->blackboards_data + (index * partition->max_blackboard_data_size), partition->blackboards_size_data[index]);
        *LENGTH = partition->blackboards_size_data[index];
        *RETURN_CODE = NO_ERROR;
    } else if (TIME_OUT == 0){
        *LENGTH = 0;
        *RETURN_CODE = NOT_AVAILABLE;
    }
    // to do mutex or error handler
    else if (TIME_OUT == INFINITE_TIME_VALUE) {
        current_process->processus_status->PROCESS_STATE = WAITING;
        bb->blackboard_status.WAITING_PROCESSES++;
        list_pushback(bb->waiting_processes, current_process);
        yield_to_partition(partition, current_process);
        memcpy(MESSAGE_ADDR, partition->blackboards_data + (index * partition->max_blackboard_data_size), partition->blackboards_size_data[index]);
        *LENGTH = partition->blackboards_size_data[index];
        *RETURN_CODE = NO_ERROR;
    } else {
        current_process->processus_status->PROCESS_STATE = WAITING;
        current_process->waiting_blackboard = bb;
        bb->blackboard_status.WAITING_PROCESSES++;
        list_pushback(bb->waiting_processes, current_process);
        current_process->time_counter = (SYSTEM_TIME_TYPE)ucx_uptime() + (SYSTEM_TIME_TYPE)TIME_OUT;
        yield_to_partition(partition, current_process);
        if(current_process->time_counter == 0){
            *LENGTH = 0;
            *RETURN_CODE = TIMED_OUT;
        }
        else {
            memcpy(MESSAGE_ADDR, partition->blackboards_data + (index * partition->max_blackboard_data_size), partition->blackboards_size_data[index]);
            *LENGTH = partition->blackboards_size_data[index];
            *RETURN_CODE = NO_ERROR;
        }
    }
}


void CLEAR_BLACKBOARD (
       /*in */ BLACKBOARD_ID_TYPE       BLACKBOARD_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
    
    struct pcb_s *partition = get_current_partition();
    int index = find_blackboard_by_id(partition, BLACKBOARD_ID);
    if (index == -1){
        *RETURN_CODE = INVALID_CONFIG;
        return;
    }
    struct blackboard_s *bb = &partition->blackboards[index];
    bb->blackboard_status.EMPTY_INDICATOR = EMPTY;
    *RETURN_CODE = NO_ERROR;
}

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

void CREATE_BUFFER (
       /*in */ BUFFER_NAME_TYPE         BUFFER_NAME,
       /*in */ MESSAGE_SIZE_TYPE        MAX_MESSAGE_SIZE,
       /*in */ MESSAGE_RANGE_TYPE       MAX_NB_MESSAGE,
       /*in */ QUEUING_DISCIPLINE_TYPE  QUEUING_DISCIPLINE,
       /*out*/ BUFFER_ID_TYPE           *BUFFER_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
       
    struct pcb_s *partition = get_current_partition();
    if (partition->buffer_count + 1 > partition->max_buffers || partition->max_buffer_data_size < MAX_MESSAGE_SIZE * MAX_NB_MESSAGE){
        *RETURN_CODE = INVALID_CONFIG;
        return;
    }

    if (find_buffer_by_name(partition, BUFFER_NAME) != -1){
        *RETURN_CODE = NO_ACTION;
        return;
    }

    if (MAX_MESSAGE_SIZE <= 0){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    if (MAX_NB_MESSAGE <= 0){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    if (QUEUING_DISCIPLINE != FIFO && QUEUING_DISCIPLINE != PRIORITY){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    if (partition->status->OPERATING_MODE == NORMAL){
         *RETURN_CODE = INVALID_MODE;
         return;
    }

    struct buffer_s *buf = &partition->buffers[partition->buffer_count++];
    strncpy(buf->buffer_name, BUFFER_NAME, sizeof(buf->buffer_name) - 1);
    buf->buffer_name[sizeof(buf->buffer_name) - 1] = '\0';
    buf->buffer_id = partition->buffer_count;
    buf->partition_id = partition->status->IDENTIFIER;
    buf->waiting_readers = list_create();
    buf->waiting_writers = list_create();
    buf->read_index = 0;
    buf->write_index = 0;
    buf->buffer_status.NB_MESSAGE = 0;
    buf->buffer_status.MAX_NB_MESSAGE = MAX_NB_MESSAGE;
    buf->buffer_status.MAX_MESSAGE_SIZE = MAX_MESSAGE_SIZE;
    buf->buffer_status.WAITING_PROCESSES = 0; 
    buf->queuing_discipline = QUEUING_DISCIPLINE;

    *BUFFER_ID = buf->buffer_id;
    *RETURN_CODE = NO_ERROR;
}

void SEND_BUFFER (
       /*in */ BUFFER_ID_TYPE           BUFFER_ID,
       /*in */ MESSAGE_ADDR_TYPE        MESSAGE_ADDR,       /* by reference */
       /*in */ MESSAGE_SIZE_TYPE        LENGTH,
       /*in */ SYSTEM_TIME_TYPE         TIME_OUT,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
       
    struct pcb_s *partition = get_current_partition();
    int index = find_buffer_by_id(partition, BUFFER_ID);
    if (index == -1){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    struct buffer_s *buf = &partition->buffers[index];
    uint32_t data_base = (uint32_t)index * (uint32_t)partition->max_buffer_data_size;
    uint32_t size_base = (uint32_t)index * (uint32_t)buf->buffer_status.MAX_NB_MESSAGE;

    if(LENGTH >= buf->buffer_status.MAX_MESSAGE_SIZE){
            *RETURN_CODE = INVALID_CONFIG;
            return;
    }

    if(LENGTH <= 0){
            *RETURN_CODE = INVALID_PARAM;
            return;
    }

    if(TIME_OUT < 0 || time_overflow(ucx_uptime() + (SYSTEM_TIME_TYPE)TIME_OUT)){
            *RETURN_CODE = INVALID_PARAM;
            return;
    }

    struct process_s *current_process = partition->process_current->data;
    if(buf->buffer_status.NB_MESSAGE < buf->buffer_status.MAX_NB_MESSAGE){
            if (buf->waiting_readers->length == 0){
                buf->buffer_status.NB_MESSAGE++;
                uint32_t write_index = buf->write_index;
                memcpy(&partition->buffers_data[data_base + write_index * buf->buffer_status.MAX_MESSAGE_SIZE], MESSAGE_ADDR, LENGTH);
                partition->buffers_size_data[size_base + write_index] = LENGTH;
                buf->write_index = (write_index + 1) % buf->buffer_status.MAX_NB_MESSAGE;
            } else {
                // réveiller un lecteur en attente
                struct node_s *first_reader_node = buf->waiting_readers->head->next;
                struct process_s *reader_process = first_reader_node->data;
                list_remove(buf->waiting_readers, first_reader_node);
                buf->buffer_status.WAITING_PROCESSES--;
                // écrire le message directement dans le buffer du lecteur MESS ADDR 
                memcpy(reader_process->waiting_message_addr, MESSAGE_ADDR, LENGTH);
                reader_process->waiting_message_size = LENGTH;
                if (reader_process->time_counter != 0) {
                    reader_process->time_counter = INFINITE_TIME_VALUE;
                }
                reader_process->processus_status->PROCESS_STATE = READY;
                reader_process->waiting_buffer = NULL;
                yield_to_partition(partition, current_process);
            }
            *RETURN_CODE = NO_ERROR;
    }
    else if (TIME_OUT == 0){
        *RETURN_CODE = NOT_AVAILABLE;
    }
    // cd current process own mutex
    // else if ()
    else if (TIME_OUT == INFINITE_TIME_VALUE){
        current_process->processus_status->PROCESS_STATE = WAITING;
        buf->buffer_status.WAITING_PROCESSES++;
        current_process->waiting_buffer = buf;
        current_process->waiting_message_addr = MESSAGE_ADDR;
        current_process->waiting_message_size = LENGTH;
        // to do implementer selon la discipline de la file d'attente
        if (buf->queuing_discipline == PRIORITY){
            // to do insert process in waiting_processes list according to its priority
            list_insert_sorted(buf->waiting_writers, current_process);
        }
        else {
            list_pushback(buf->waiting_writers, current_process);
        }
        yield_to_partition(partition, current_process);
        *RETURN_CODE = NO_ERROR;

    } else {
        printf("REVEIL LECTEUR ENVOI DIRECT \n");   
        current_process->processus_status->PROCESS_STATE = WAITING;
        buf->buffer_status.WAITING_PROCESSES++;
        current_process->waiting_buffer = buf;
        current_process->waiting_message_addr = MESSAGE_ADDR;
        current_process->waiting_message_size = LENGTH;
        // to do implementer selon la discipline de la file d'attente
        if (buf->queuing_discipline == PRIORITY){
            // to do insert process in waiting_processes list according to its priority
            list_insert_sorted(buf->waiting_writers, current_process);
        }
        else {
            list_pushback(buf->waiting_writers, current_process);
        }
        current_process->time_counter = (SYSTEM_TIME_TYPE)ucx_uptime() + (SYSTEM_TIME_TYPE)TIME_OUT;
        yield_to_partition(partition, current_process);
        if(current_process->time_counter == 0){
            *RETURN_CODE = TIMED_OUT;
        }
        else {
            *RETURN_CODE = NO_ERROR;
        }
    }
}

void RECEIVE_BUFFER (
       /*in */ BUFFER_ID_TYPE           BUFFER_ID,
       /*in */ SYSTEM_TIME_TYPE         TIME_OUT,
       /*in */ MESSAGE_ADDR_TYPE        MESSAGE_ADDR,
               /* The message address is passed IN, although */
               /* the respective message is passed OUT       */
       /*out*/ MESSAGE_SIZE_TYPE        *LENGTH,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
    struct pcb_s *partition = get_current_partition();
    int index = find_buffer_by_id(partition, BUFFER_ID);
    if (index == -1){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    if (TIME_OUT < 0 || time_overflow(ucx_uptime() + (SYSTEM_TIME_TYPE)TIME_OUT)){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    struct buffer_s *buf = &partition->buffers[index];
    struct process_s *current_process = partition->process_current->data;
    uint32_t data_base = (uint32_t)index * (uint32_t)partition->max_buffer_data_size;
    uint32_t size_base = (uint32_t)index * (uint32_t)buf->buffer_status.MAX_NB_MESSAGE;

    if(partition->buffers[index].buffer_status.NB_MESSAGE > 0){
        buf->buffer_status.NB_MESSAGE--;
        uint32_t read_index = buf->read_index;
        memcpy(MESSAGE_ADDR, &partition->buffers_data[data_base + read_index * buf->buffer_status.MAX_MESSAGE_SIZE], partition->buffers_size_data[size_base + read_index]);
        *LENGTH = partition->buffers_size_data[size_base + read_index];
        buf->read_index = (read_index + 1) % buf->buffer_status.MAX_NB_MESSAGE;

        if (buf->waiting_writers->length > 0){
            // réveiller un écrivain en attente
            struct node_s *first_writer_node = buf->waiting_writers->head->next;
            struct process_s *writer_process = first_writer_node->data;
            list_remove(buf->waiting_writers, first_writer_node);
            buf->buffer_status.WAITING_PROCESSES--;
            // put the message associated with this sending process in the FIFO
            memcpy(&partition->buffers_data[data_base + buf->write_index * buf->buffer_status.MAX_MESSAGE_SIZE], writer_process->waiting_message_addr, writer_process->waiting_message_size);
            partition->buffers_size_data[size_base + buf->write_index] = writer_process->waiting_message_size;
            buf->buffer_status.NB_MESSAGE++;
            buf->write_index = (buf->write_index + 1) % buf->buffer_status.MAX_NB_MESSAGE;
            if (writer_process->time_counter != 0) {
                writer_process->time_counter = INFINITE_TIME_VALUE;
            }
            writer_process->processus_status->PROCESS_STATE = READY;
            writer_process->waiting_buffer = NULL;
            yield_to_partition(partition, current_process);
        }
        *RETURN_CODE = NO_ERROR;
    }
    else if (TIME_OUT == 0){
        *LENGTH = 0;
        *RETURN_CODE = NOT_AVAILABLE;
    }
    // to do mutex or error handler
    else if (TIME_OUT == INFINITE_TIME_VALUE){
        current_process->processus_status->PROCESS_STATE = WAITING;
        buf->buffer_status.WAITING_PROCESSES++;
        current_process->waiting_buffer = buf;
        current_process->waiting_message_addr = MESSAGE_ADDR;
        if (buf->queuing_discipline == PRIORITY){
            // to do insert process in waiting_processes list according to its priority
            list_insert_sorted(buf->waiting_readers, current_process);
        }
        else {
            list_pushback(buf->waiting_readers, current_process);
        }
        yield_to_partition(partition, current_process);
        *LENGTH = current_process->waiting_message_size;
        *RETURN_CODE = NO_ERROR;
    } else {
        current_process->processus_status->PROCESS_STATE = WAITING;
        buf->buffer_status.WAITING_PROCESSES++;
        current_process->waiting_buffer = buf;
        current_process->waiting_message_addr = MESSAGE_ADDR;
        if (buf->queuing_discipline == PRIORITY){
            // to do insert process in waiting_processes list according to its priority
            list_insert_sorted(buf->waiting_readers, current_process);
        }
        else {
            list_pushback(buf->waiting_readers, current_process);
        }
        current_process->time_counter = (SYSTEM_TIME_TYPE)ucx_uptime() + (SYSTEM_TIME_TYPE)TIME_OUT;
        yield_to_partition(partition, current_process);

        if(current_process->time_counter == 0){
            *LENGTH = 0;
            *RETURN_CODE = TIMED_OUT;
        }
        else {
            *LENGTH = current_process->waiting_message_size;
            *RETURN_CODE = NO_ERROR;
        }
    }

}

void GET_BUFFER_ID (
       /*in */ BUFFER_NAME_TYPE         BUFFER_NAME,
       /*out*/ BUFFER_ID_TYPE           *BUFFER_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
    struct pcb_s *partition = get_current_partition();
    int index = find_buffer_by_name(partition, BUFFER_NAME);
    if (index == -1){
        *RETURN_CODE = INVALID_CONFIG;
        return;
    }
    struct buffer_s *buf = &partition->buffers[index];
    *BUFFER_ID = buf->buffer_id;
    *RETURN_CODE = NO_ERROR;
}

void GET_BUFFER_STATUS (
       /*in */ BUFFER_ID_TYPE           BUFFER_ID,
       /*out*/ BUFFER_STATUS_TYPE       *BUFFER_STATUS,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
    struct pcb_s *partition = get_current_partition();
    int index = find_buffer_by_id(partition, BUFFER_ID);
    if (index == -1){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }
    struct buffer_s *buf = &partition->buffers[index];

    *BUFFER_STATUS = buf->buffer_status;
    *RETURN_CODE = NO_ERROR;
}

void CREATE_SEMAPHORE (
       /*in */ SEMAPHORE_NAME_TYPE      SEMAPHORE_NAME,
       /*in */ SEMAPHORE_VALUE_TYPE     CURRENT_VALUE,
       /*in */ SEMAPHORE_VALUE_TYPE     MAXIMUM_VALUE,
       /*in */ QUEUING_DISCIPLINE_TYPE  QUEUING_DISCIPLINE,
       /*out*/ SEMAPHORE_ID_TYPE        *SEMAPHORE_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
    struct pcb_s *partition = get_current_partition();
    if (partition->semaphore_count + 1 > partition->max_semaphores){
        *RETURN_CODE = INVALID_CONFIG;
        return;
    }

    if (find_semaphore_by_name(partition, SEMAPHORE_NAME) != -1){
        *RETURN_CODE = NO_ACTION;
        return;
    }

    if (CURRENT_VALUE < 0){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    if (MAXIMUM_VALUE <= 0){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    if (CURRENT_VALUE > MAXIMUM_VALUE){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    if (QUEUING_DISCIPLINE != FIFO && QUEUING_DISCIPLINE != PRIORITY){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    if (partition->status->OPERATING_MODE == NORMAL){
         *RETURN_CODE = INVALID_MODE;
         return;
    } 

    struct semaphore_s *sem = &partition->semaphores[partition->semaphore_count++];
    strncpy(sem->semaphore_name, SEMAPHORE_NAME, sizeof(sem->semaphore_name) - 1);
    sem->semaphore_name[sizeof(sem->semaphore_name) - 1] = '\0';
    sem->semaphore_id = partition->semaphore_count;
    sem->partition_id = partition->status->IDENTIFIER;
    sem->waiting_processes = list_create();
    sem->semaphore_status.CURRENT_VALUE = CURRENT_VALUE;
    sem->semaphore_status.MAXIMUM_VALUE = MAXIMUM_VALUE;
    sem->semaphore_status.WAITING_PROCESSES = 0;
    sem->queuing_discipline = QUEUING_DISCIPLINE;

    *SEMAPHORE_ID = sem->semaphore_id;
    *RETURN_CODE = NO_ERROR;
}

void WAIT_SEMAPHORE (
       /*in */ SEMAPHORE_ID_TYPE        SEMAPHORE_ID,
       /*in */ SYSTEM_TIME_TYPE         TIME_OUT,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
    
    struct pcb_s *partition = get_current_partition();
    int index = find_semaphore_by_id(partition, SEMAPHORE_ID);
    if (index == -1){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    if (TIME_OUT < 0 || time_overflow(ucx_uptime() + (SYSTEM_TIME_TYPE)TIME_OUT)){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    struct semaphore_s *sem = &partition->semaphores[index];
    if (sem->semaphore_status.CURRENT_VALUE > 0){
        sem->semaphore_status.CURRENT_VALUE--;
        *RETURN_CODE = NO_ERROR;
    }
    else if (TIME_OUT == 0){
        *RETURN_CODE = NOT_AVAILABLE;
    }
    // to do mutex or error handler
    else if (TIME_OUT == INFINITE_TIME_VALUE){
        struct process_s *current_process = partition->process_current->data;
        current_process->processus_status->PROCESS_STATE = WAITING;
        sem->semaphore_status.WAITING_PROCESSES++;
        // to do implementer selon la discipline de la file d'attente
        if (sem->queuing_discipline == PRIORITY){
            // to do insert process in waiting_processes list according to its priority
            list_insert_sorted(sem->waiting_processes, current_process);
        }
        else {
            list_pushback(sem->waiting_processes, current_process);
        }
        yield_to_partition(partition, current_process);
        *RETURN_CODE = NO_ERROR;
    } else {
        struct process_s *current_process = partition->process_current->data;
        current_process->processus_status->PROCESS_STATE = WAITING;
        sem->semaphore_status.WAITING_PROCESSES++;
        current_process->waiting_semaphore = sem;
        // to do implementer selon la discipline de la file d'attente
        if (sem->queuing_discipline == PRIORITY){
            // to do insert process in waiting_processes list according to its priority
            list_insert_sorted(sem->waiting_processes, current_process);
        }
        else {
            list_pushback(sem->waiting_processes, current_process);
        }
        current_process->time_counter = (SYSTEM_TIME_TYPE)ucx_uptime() + (SYSTEM_TIME_TYPE)TIME_OUT;
        yield_to_partition(partition, current_process);

        if(current_process->time_counter == 0){
            *RETURN_CODE = TIMED_OUT;
        }
        else {
            *RETURN_CODE = NO_ERROR;
        }
    }
}

void SIGNAL_SEMAPHORE (
       /*in */ SEMAPHORE_ID_TYPE        SEMAPHORE_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
    struct pcb_s *partition = get_current_partition();
    int index = find_semaphore_by_id(partition, SEMAPHORE_ID);
    if (index == -1){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    struct semaphore_s *sem = &partition->semaphores[index];
    if (sem->semaphore_status.CURRENT_VALUE == sem->semaphore_status.MAXIMUM_VALUE){
        *RETURN_CODE = INVALID_CONFIG;
        return;
    }
    
    if (sem->waiting_processes->length == 0){
        sem->semaphore_status.CURRENT_VALUE++;
        *RETURN_CODE = NO_ERROR;
    }
    else {
        // réveiller un processus en attente
        struct process_s *current_process = partition->process_current->data;
        struct node_s *first_node = sem->waiting_processes->head->next;
        struct process_s *woken_process = first_node->data;
        list_remove(sem->waiting_processes, first_node);
        sem->semaphore_status.WAITING_PROCESSES--;
        woken_process->processus_status->PROCESS_STATE = READY;
        woken_process->waiting_semaphore = NULL;
        if(woken_process->time_counter != 0){
            woken_process->time_counter = INFINITE_TIME_VALUE;
        }
        yield_to_partition(partition, current_process);
        *RETURN_CODE = NO_ERROR;
    }
}

void GET_SEMAPHORE_ID (
       /*in */ SEMAPHORE_NAME_TYPE      SEMAPHORE_NAME,
       /*out*/ SEMAPHORE_ID_TYPE        *SEMAPHORE_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
    
    struct pcb_s *partition = get_current_partition();
    int index = find_semaphore_by_name(partition, SEMAPHORE_NAME);
    if (index == -1){   
        *RETURN_CODE = INVALID_CONFIG;
        return;
    }
    struct semaphore_s *sem = &partition->semaphores[index];
    *SEMAPHORE_ID = sem->semaphore_id;
    *RETURN_CODE = NO_ERROR;
}


void GET_SEMAPHORE_STATUS (
       /*in */ SEMAPHORE_ID_TYPE        SEMAPHORE_ID,
       /*out*/ SEMAPHORE_STATUS_TYPE    *SEMAPHORE_STATUS,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){

    struct pcb_s *partition = get_current_partition();
    int index = find_semaphore_by_id(partition, SEMAPHORE_ID);
    if (index == -1){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }
    struct semaphore_s *sem = &partition->semaphores[index];

    *SEMAPHORE_STATUS = sem->semaphore_status;
    *RETURN_CODE = NO_ERROR;
}

void CREATE_EVENT (
       /*in */ EVENT_NAME_TYPE          EVENT_NAME,
       /*out*/ EVENT_ID_TYPE            *EVENT_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
    struct pcb_s *partition = get_current_partition();
    if (partition->event_count + 1 > partition->max_events){
        *RETURN_CODE = INVALID_CONFIG;
        return;
    }

    if (find_event_by_name(partition, EVENT_NAME) != -1){
        *RETURN_CODE = NO_ACTION;
        return;
    }

    if (partition->status->OPERATING_MODE == NORMAL){
         *RETURN_CODE = INVALID_MODE;
         return;
    }

    struct event_s *event = &partition->events[partition->event_count++];
    strncpy(event->event_name, EVENT_NAME, sizeof(event->event_name) - 1);
    event->event_name[sizeof(event->event_name) - 1] = '\0';
    event->event_id = partition->event_count;
    event->partition_id = partition->status->IDENTIFIER;
    event->waiting_processes = list_create();
    event->event_status.EVENT_STATE = DOWN;
    event->event_status.WAITING_PROCESSES = 0;

    *EVENT_ID = event->event_id;
    *RETURN_CODE = NO_ERROR;
}

void SET_EVENT (
       /*in */ EVENT_ID_TYPE            EVENT_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

void RESET_EVENT (
       /*in */ EVENT_ID_TYPE            EVENT_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

void WAIT_EVENT (
       /*in */ EVENT_ID_TYPE            EVENT_ID,
       /*in */ SYSTEM_TIME_TYPE         TIME_OUT,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

void GET_EVENT_ID (
       /*in */ EVENT_NAME_TYPE          EVENT_NAME,
       /*out*/ EVENT_ID_TYPE            *EVENT_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
    struct pcb_s *partition = get_current_partition();
    int index = find_event_by_name(partition, EVENT_NAME);
    if (index == -1){
        *RETURN_CODE = INVALID_CONFIG;
        return; 
    }
    struct event_s *event = &partition->events[index];
    *EVENT_ID = event->event_id;
    *RETURN_CODE = NO_ERROR;
}

void GET_EVENT_STATUS (
       /*in */ EVENT_ID_TYPE            EVENT_ID,
       /*out*/ EVENT_STATUS_TYPE        *EVENT_STATUS,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
    struct pcb_s *partition = get_current_partition();
    int index = find_event_by_id(partition, EVENT_ID);
    if (index == -1){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }
    struct event_s *event = &partition->events[index];
    *EVENT_STATUS = event->event_status;
    *RETURN_CODE = NO_ERROR;
}
void APERIODIC_WAIT_EVENT (
       /*in */ EVENT_ID_TYPE            EVENT_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

void CREATE_MUTEX (
       /*in */ MUTEX_NAME_TYPE          MUTEX_NAME,
       /*in */ PRIORITY_TYPE            MUTEX_PRIORITY,
       /*in */ QUEUING_DISCIPLINE_TYPE  QUEUING_DISCIPLINE,
       /*out*/ MUTEX_ID_TYPE            *MUTEX_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
    struct pcb_s *partition = get_current_partition();
    if (partition->mutex_count + 1 > partition->max_mutexes){
        *RETURN_CODE = INVALID_CONFIG;
        return;
    }

    if (find_mutex_by_name(partition, MUTEX_NAME) != -1){
        *RETURN_CODE = NO_ACTION;
        return;
    }

    if (MUTEX_PRIORITY < 0){
         *RETURN_CODE = INVALID_PARAM;
         return;
    }

    if (QUEUING_DISCIPLINE != FIFO && QUEUING_DISCIPLINE != PRIORITY){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    if (partition->status->OPERATING_MODE == NORMAL){
         *RETURN_CODE = INVALID_MODE;
         return;
    }

    struct mutex_s *mutex = &partition->mutexes[partition->mutex_count++];
    strncpy(mutex->mutex_name, MUTEX_NAME, sizeof(mutex->mutex_name) - 1);
    mutex->mutex_name[sizeof(mutex->mutex_name) - 1] = '\0';
    mutex->mutex_id = partition->mutex_count;
    mutex->partition_id = partition->status->IDENTIFIER;
    mutex->waiting_processes = list_create();
    mutex->mutex_status.MUTEX_STATE = AVAILABLE;
    mutex->mutex_status.MUTEX_PRIORITY = MUTEX_PRIORITY;
    mutex->mutex_status.WAITING_PROCESSES = 0;
    mutex->mutex_status.LOCK_COUNT = 0;
    mutex->queuing_discipline = QUEUING_DISCIPLINE;

    *MUTEX_ID = mutex->mutex_id;
    *RETURN_CODE = NO_ERROR;
}

void ACQUIRE_MUTEX (
       /*in */ MUTEX_ID_TYPE            MUTEX_ID,
       /*in */ SYSTEM_TIME_TYPE         TIME_OUT,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

void RELEASE_MUTEX (
       /*in */ MUTEX_ID_TYPE            MUTEX_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

void RESET_MUTEX (
       /*in */ MUTEX_ID_TYPE            MUTEX_ID,
       /*in */ PROCESS_ID_TYPE          PROCESS_ID, 
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

void GET_MUTEX_ID (
       /*in */ MUTEX_NAME_TYPE          MUTEX_NAME,
       /*out*/ MUTEX_ID_TYPE            *MUTEX_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
    struct pcb_s *partition = get_current_partition();
    int index = find_mutex_by_name(partition, MUTEX_NAME);
    if (index == -1){  
        *RETURN_CODE = INVALID_CONFIG;
        return;
    }
    struct mutex_s *mutex = &partition->mutexes[index];
    *MUTEX_ID = mutex->mutex_id;
    *RETURN_CODE = NO_ERROR;
}

void GET_MUTEX_STATUS (
       /*in */ MUTEX_ID_TYPE            MUTEX_ID,
       /*out*/ MUTEX_STATUS_TYPE        *MUTEX_STATUS,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
    struct pcb_s *partition = get_current_partition();
    int index = find_mutex_by_id(partition, MUTEX_ID);
    if (index == -1){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }
    struct mutex_s *mutex = &partition->mutexes[index];
    MUTEX_STATUS_TYPE status;
    status.MUTEX_STATE = mutex->mutex_status.MUTEX_STATE;
    status.MUTEX_PRIORITY = mutex->mutex_status.MUTEX_PRIORITY;
    status.WAITING_PROCESSES = mutex->mutex_status.WAITING_PROCESSES;
    status.LOCK_COUNT = mutex->mutex_status.LOCK_COUNT;
    if (mutex->mutex_status.MUTEX_STATE == AVAILABLE){
        status.MUTEX_OWNER = NULL_PROCESS_ID;
    }

    else if (mutex->mutex_status.MUTEX_OWNER == MAIN_PROCESS_ID){
        status.MUTEX_OWNER = MAIN_PROCESS_ID;
    }
    else {
        // to do get the process id of the owner of the mutex
        status.MUTEX_OWNER = mutex->mutex_status.MUTEX_OWNER;
    } 
    *MUTEX_STATUS = status;
    *RETURN_CODE = NO_ERROR;
}

void GET_PROCESS_MUTEX_STATE (
       /*in */ PROCESS_ID_TYPE          PROCESS_ID,
       /*out*/ MUTEX_ID_TYPE            *MUTEX_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );