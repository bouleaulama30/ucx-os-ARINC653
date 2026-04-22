#include <ucx.h>

static struct node_s *find_processes_by_name(struct node_s *node, void *arg){
    struct process_s *process = node->data;
    const char *name = (const char *) arg;
    
    if(strcmp(process->processus_status->ATTRIBUTES.NAME, name) == 0){
        return node;
    }
    return 0;
}

static struct node_s *find_process_node_by_name(struct pcb_s *partition, PROCESS_NAME_TYPE process_name){
    return list_foreach(partition->processes, find_processes_by_name, (void *)process_name);
}

struct node_s *find_processes_by_id(struct node_s *node, void *arg){
    struct process_s *process = node->data;
    PROCESS_ID_TYPE id = (PROCESS_ID_TYPE) arg;
    
    if(id == process->process_id){
        return node;
    }
    return 0;
}

struct node_s *is_process_id_existed(struct pcb_s *partition, PROCESS_ID_TYPE process_id){
    struct node_s *same_process_name_node = list_foreach(partition->processes, find_processes_by_id, (void *)process_id);
    if(same_process_name_node){
        return same_process_name_node;
    }
    return 0;
}

int is_process_waiting_in_waiting_queue(struct process_s *process){
    if (process->waiting_queuing_port != NULL || process->waiting_blackboard != NULL || process->waiting_buffer != NULL || process->waiting_mutex != NULL || process->waiting_semaphore != NULL || process->waiting_event != NULL){
        return 1;
    }
    return 0;
}

int is_process_waiting_in_mutex_queue(struct process_s *process){
    if (process->waiting_mutex != NULL){
        return 1;
    }
    return 0;
}

struct node_s *find_waiting_process_node(struct node_s *node, void *arg)
{
    struct process_s *process = node->data;
    struct process_s *target = arg;

    if (process == target)
        return node;

    return 0;
}

void remove_process_from_waiting_queue(struct process_s *process){
    if(process->waiting_queuing_port) {
        struct queuing_port_s *queuing_port = process->waiting_queuing_port;
        struct node_s *waiting_node = list_foreach(queuing_port->waiting_processes, find_waiting_process_node, process);
        if (waiting_node){ 
            list_remove(queuing_port->waiting_processes, waiting_node);
            queuing_port->queuing_port_status->WAITING_PROCESSES--;
        }
        process->waiting_queuing_port = NULL;
    }

    if(process->waiting_blackboard) {
        struct blackboard_s *bb = process->waiting_blackboard;
        struct node_s *waiting_node = list_foreach(bb->waiting_processes, find_waiting_process_node, process);
        if (waiting_node){ 
            list_remove(bb->waiting_processes, waiting_node);
            bb->blackboard_status.WAITING_PROCESSES--;
        }
        process->waiting_blackboard = NULL;
    }

    if(process->waiting_buffer) {
        struct buffer_s *buf = process->waiting_buffer;
        struct node_s *waiting_reader_node = list_foreach(buf->waiting_readers, find_waiting_process_node, process);
        if (waiting_reader_node){ 
            list_remove(buf->waiting_readers, waiting_reader_node);
            buf->buffer_status.WAITING_PROCESSES--;
        }

        struct node_s *waiting_writer_node = list_foreach(buf->waiting_writers, find_waiting_process_node, process);
        if (waiting_writer_node){ 
            list_remove(buf->waiting_writers, waiting_writer_node);
            buf->buffer_status.WAITING_PROCESSES--;
        }
        process->waiting_buffer = NULL;
    }

    if(process->waiting_semaphore) {
        struct semaphore_s *sem = process->waiting_semaphore;
        struct node_s *waiting_node = list_foreach(sem->waiting_processes, find_waiting_process_node, process);
        if (waiting_node){ 
            list_remove(sem->waiting_processes, waiting_node);
            sem->semaphore_status.WAITING_PROCESSES--;
        }
        process->waiting_semaphore = NULL;
    }

    if(process->waiting_event) {
        struct event_s *event = process->waiting_event;
        struct node_s *waiting_node = list_foreach(event->waiting_processes, find_waiting_process_node, process);
        if (waiting_node){ 
            list_remove(event->waiting_processes, waiting_node);
            event->event_status.WAITING_PROCESSES--;
        }
        process->waiting_event = NULL;
    }

    if (process->waiting_mutex) {
        struct mutex_s *mutex = process->waiting_mutex;
        struct node_s *waiting_node = list_foreach(mutex->waiting_processes, find_waiting_process_node, process);
        if (waiting_node){ 
            list_remove(mutex->waiting_processes, waiting_node);
            mutex->mutex_status.WAITING_PROCESSES--;
        }
        process->waiting_mutex = NULL;
    }
}

void CREATE_PROCESS (
       /*in */ PROCESS_ATTRIBUTE_TYPE   *ATTRIBUTES,
       /*out*/ PROCESS_ID_TYPE          *PROCESS_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){

    struct pcb_s *partition = get_current_partition();
    if(partition->nbr_processes >= MAX_NUMBER_OF_PROCESSES){
        *RETURN_CODE = INVALID_CONFIG;
    } 
    
    else if ((partition->next_stack_addr + ATTRIBUTES->STACK_SIZE) > 
        (partition->memory_requirements->memory[DATA].base + partition->memory_requirements->memory[DATA].size))
    {
        *RETURN_CODE = INVALID_CONFIG;
    }
    else if (find_process_node_by_name(partition, ATTRIBUTES->NAME))
    {
        *RETURN_CODE = NO_ACTION;
    }
    else if (ATTRIBUTES->STACK_SIZE > partition->memory_requirements->memory[DATA].size){
        *RETURN_CODE = INVALID_PARAM;
    }
    else if (ATTRIBUTES->BASE_PRIORITY < MIN_PRIORITY_VALUE || ATTRIBUTES->BASE_PRIORITY > MAX_PRIORITY_VALUE)
    {
        *RETURN_CODE = INVALID_PARAM;
    }
    else if (ATTRIBUTES->PERIOD < -1)
    {
        *RETURN_CODE = INVALID_PARAM;
    }
    else if (ATTRIBUTES->PERIOD >= 0 && (ATTRIBUTES->PERIOD % partition->status->PERIOD != 0))
    {
        *RETURN_CODE = INVALID_CONFIG;
    }
    else if (ATTRIBUTES->TIME_CAPACITY < -1)
    {
        *RETURN_CODE = INVALID_CONFIG;
    }
    else if (ATTRIBUTES->PERIOD >= 0 && ATTRIBUTES->TIME_CAPACITY >= 0 && ATTRIBUTES->TIME_CAPACITY >= 0 && ATTRIBUTES->TIME_CAPACITY > ATTRIBUTES->PERIOD)
    {
        *RETURN_CODE = INVALID_PARAM;
    }
    else if (partition->status->OPERATING_MODE == NORMAL){
        *RETURN_CODE = INVALID_MODE;
    }
    else{
        // coherence de l'etat de la partition
        partition->nbr_processes++;

        struct process_s *new_process;
        PROCESS_STATUS_TYPE *status;

        new_process = malloc(sizeof(struct process_s));
        status = malloc(sizeof(PROCESS_STATUS_TYPE));
        
        status->DEADLINE_TIME = ATTRIBUTES->TIME_CAPACITY;
        status->CURRENT_PRIORITY = ATTRIBUTES->BASE_PRIORITY;
        status->PROCESS_STATE = DORMANT;
        status->ATTRIBUTES = *ATTRIBUTES;

        new_process->processus_status = status;

        int32_t id = ucx_process_spawn(ATTRIBUTES->ENTRY_POINT, ATTRIBUTES->STACK_SIZE, new_process, partition);
        new_process->process_id = id;
        new_process->process_index = partition->nbr_processes;
        new_process->processor_core_affinity = DEFAULT_PROCESS_CORE_AFFINITY;
        new_process->release_point_time = 0; 
        new_process->is_suspended = false;
        new_process->time_counter = 0;
        new_process->saved_init_delay = 0;
        new_process->owned_mutex_id = NO_MUTEX_OWNED;

       
        list_pushback(partition->processes, new_process);
        
        *PROCESS_ID = new_process->process_id;
        *RETURN_CODE = NO_ERROR;
    }
}

void SET_PRIORITY (
       /*in */ PROCESS_ID_TYPE          PROCESS_ID,
       /*in */ PRIORITY_TYPE            PRIORITY,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
    struct pcb_s *partition = get_current_partition();
    struct node_s *process_node = is_process_id_existed(partition, PROCESS_ID);
    if(!process_node){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    if (PRIORITY < MIN_PRIORITY_VALUE || PRIORITY > MAX_PRIORITY_VALUE)
    {
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    struct process_s *process = process_node->data;
    if (process->processus_status->PROCESS_STATE == DORMANT){
        *RETURN_CODE = INVALID_MODE;
        return;
    }

    if (process->owned_mutex_id != NO_MUTEX_OWNED){
        int index = find_mutex_by_id(partition, process->owned_mutex_id);
        struct mutex_s *mutex = &partition->mutexes[index];
        mutex->saved_owner_priority = PRIORITY;
    }
    else {
        process->processus_status->CURRENT_PRIORITY = PRIORITY;
    }

    // sauvegarde du context du process actuel et on reschedule
    if (process->processus_status->PROCESS_STATE == READY || process->processus_status->PROCESS_STATE == RUNNING){
        // respect the shortest elapsed time (i.e., other processes at the same priority eligible for the same processor core(s) will be selected to run before this process)
        list_remove(partition->processes, process_node);
        struct node_s *new_process_node = list_pushback(partition->processes, process);

        // si le processus qu'on change est le processus courant alors il faut update le nouveau node au processus courant
        struct process_s *new_process = new_process_node->data;
        if(new_process->processus_status->PROCESS_STATE == RUNNING)
            partition->process_current = new_process_node;
        
        struct process_s *current_process = partition->process_current->data;
        yield_to_partition(partition, current_process);
    }
    *RETURN_CODE = NO_ERROR;
}

void SUSPEND_SELF (
       /*in */ SYSTEM_TIME_TYPE         TIME_OUT,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
       
    struct pcb_s *partition = get_current_partition();
    struct node_s *process_node = partition->process_current;
    struct process_s *current_process = process_node->data;
    
    // TODO error handler
    if (current_process->owned_mutex_id != NO_MUTEX_OWNED){
        *RETURN_CODE = INVALID_MODE;
        return;
    }
    uint64_t uptime = ucx_uptime();
    if (TIME_OUT < INFINITE_TIME_VALUE ||
        (TIME_OUT >= 0 && time_overflow(uptime + (uint64_t)TIME_OUT))){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    if(current_process->processus_status->ATTRIBUTES.PERIOD != INFINITE_TIME_VALUE){
        *RETURN_CODE = INVALID_MODE;
        return;
    }

    if(TIME_OUT == 0){
        *RETURN_CODE = NO_ERROR;
    }
    else {
        current_process->processus_status->PROCESS_STATE = WAITING;
        current_process->is_suspended = true;
        if (TIME_OUT != INFINITE_TIME_VALUE){
            current_process->time_counter = (SYSTEM_TIME_TYPE)uptime + TIME_OUT;
        }

        yield_to_partition(partition, current_process);
        if(current_process->time_counter == 0)
            *RETURN_CODE = TIMED_OUT;
        else            
            *RETURN_CODE = NO_ERROR;
    }
}

void SUSPEND (
       /*in */ PROCESS_ID_TYPE          PROCESS_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
    struct pcb_s *partition = get_current_partition();
    struct process_s *current_process = partition->process_current->data;
    struct node_s *process_node = is_process_id_existed(partition, PROCESS_ID);
    struct process_s *process = process_node->data;
    if(process->owned_mutex_id != NO_MUTEX_OWNED || is_process_waiting_in_mutex_queue(process)){
        *RETURN_CODE = INVALID_MODE;
        return;
    }
    
    if(!process_node || process->process_id == current_process->process_id){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    if(process->processus_status->PROCESS_STATE == DORMANT || process->processus_status->PROCESS_STATE == FAULTED){
        *RETURN_CODE = INVALID_MODE;
        return;
    }

    if(process->processus_status->ATTRIBUTES.PERIOD != INFINITE_TIME_VALUE){
        *RETURN_CODE = INVALID_MODE;
        return;
    }


    if(process->is_suspended){
        *RETURN_CODE = NO_ACTION;
    }
    else {
        process->is_suspended = true;
        process->processus_status->PROCESS_STATE = WAITING;
        *RETURN_CODE = NO_ERROR;
    }
}


void RESUME (
       /*in */ PROCESS_ID_TYPE          PROCESS_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
    struct pcb_s *partition = get_current_partition();
    struct process_s *current_process = partition->process_current->data;
    struct node_s *process_node = is_process_id_existed(partition, PROCESS_ID);
    struct process_s *process = process_node->data;
    if(!process_node || process->process_id == current_process->process_id){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }
    
    if(process->processus_status->PROCESS_STATE == DORMANT){
        *RETURN_CODE = INVALID_MODE;
        return;
    }

    if(process->processus_status->ATTRIBUTES.PERIOD != INFINITE_TIME_VALUE && process->processus_status->PROCESS_STATE != FAULTED){
        *RETURN_CODE = INVALID_MODE;
        return;
    }

    if(!process->is_suspended && process->processus_status->PROCESS_STATE != FAULTED){
        *RETURN_CODE = NO_ACTION;
    }

    if(process->is_suspended){
        process->is_suspended = false;
        process->time_counter = 0;
    }

    // checker if (the specified process is not waiting on a process queue or TIMED_WAIT
    // time delay or DELAYED_START time delay) then
    if(!process->time_counter || !is_process_waiting_in_waiting_queue(process)){
        process->processus_status->PROCESS_STATE = READY;
        // rescheduling
        yield_to_partition(partition, current_process);
    }

    
    *RETURN_CODE = NO_ERROR;

}

void STOP_SELF (void){
    struct pcb_s *partition = get_current_partition();
    struct node_s *process_node = partition->process_current;
    struct process_s *current_process = process_node->data;

    current_process->processus_status->DEADLINE_TIME = INFINITE_TIME_VALUE;

    if (current_process->owned_mutex_id != NO_MUTEX_OWNED){
        int index = find_mutex_by_id(partition, current_process->owned_mutex_id);
        struct mutex_s *mutex = &partition->mutexes[index];
        mutex->mutex_status.LOCK_COUNT = 0;
        if (mutex->mutex_id == PREEMPTION_LOCK_MUTEX)
            partition->status->LOCK_LEVEL = 0;

        mutex->mutex_status.MUTEX_STATE = AVAILABLE;
        current_process->owned_mutex_id = NO_MUTEX_OWNED;
        mutex->mutex_status.MUTEX_OWNER = NULL_PROCESS_ID;
        current_process->processus_status->CURRENT_PRIORITY = mutex->saved_owner_priority;
        
        list_remove(partition->processes, process_node);
        struct node_s *new_process_node = list_push(partition->processes, current_process);
        struct process_s *new_process = new_process_node->data;
        if(new_process->processus_status->PROCESS_STATE == RUNNING)
                partition->process_current = new_process_node;        
                
        if(mutex->waiting_processes->length > 0){
            struct node_s *first_node = mutex->waiting_processes->head->next;
            list_remove(mutex->waiting_processes, first_node);
            struct process_s *woken_process = first_node->data;
            mutex->mutex_status.WAITING_PROCESSES--;
            if (woken_process->time_counter != 0) {
                woken_process->time_counter = INFINITE_TIME_VALUE;
            }
            mutex->mutex_status.MUTEX_STATE = OWNED;
            mutex->mutex_status.LOCK_COUNT ++;
            mutex->mutex_status.MUTEX_OWNER = woken_process->process_id;
            woken_process->owned_mutex_id = mutex->mutex_id;
            mutex->saved_owner_priority = woken_process->processus_status->CURRENT_PRIORITY;
            woken_process->processus_status->CURRENT_PRIORITY = mutex->mutex_status.MUTEX_PRIORITY;
            woken_process->waiting_mutex = NULL;

            struct node_s *woken_process_node = is_process_id_existed(partition, woken_process->process_id);
            list_remove(partition->processes, woken_process_node);
            list_push(partition->processes, woken_process);
            woken_process->processus_status->PROCESS_STATE = READY;
        }
    }
    current_process->processus_status->PROCESS_STATE = DORMANT;
    current_process->time_counter = 0;
    current_process->is_suspended = false;
    // on reschedule        
    longjmp(partition->partition_context, 1);
}

void STOP (
       /*in */ PROCESS_ID_TYPE          PROCESS_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
    struct pcb_s *partition = get_current_partition();
    struct process_s *current_process = partition->process_current->data;
    struct node_s *process_node = is_process_id_existed(partition, PROCESS_ID);
    struct process_s *process = process_node->data;
    if(!process_node || process->process_id == current_process->process_id){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    if (process->processus_status->PROCESS_STATE == DORMANT)
    {
        *RETURN_CODE = NO_ACTION;
        return;
    }

    if (is_process_waiting_in_waiting_queue(process)){
        remove_process_from_waiting_queue(process);
    }

    process->processus_status->PROCESS_STATE = DORMANT;
    process->time_counter = 0;
    process->is_suspended = false;
    process->processus_status->DEADLINE_TIME = INFINITE_TIME_VALUE;
    
    yield_to_partition(partition, current_process);

    if (current_process->owned_mutex_id != NO_MUTEX_OWNED){
        *RETURN_CODE = INVALID_MODE;
    } else {
        *RETURN_CODE = NO_ERROR;
    }
}


void START (
       /*in */ PROCESS_ID_TYPE          PROCESS_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
    struct pcb_s *partition = get_current_partition();
    struct node_s *process_node = is_process_id_existed(partition, PROCESS_ID);
    struct process_s *process = process_node->data;
    
    if(!process_node){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    if (process->processus_status->PROCESS_STATE != DORMANT)
    {
        *RETURN_CODE = NO_ACTION;
        return;
    }

    // when (DEADLINE_TIME calculation is out of range) => INVALID_CONFIG
    SYSTEM_TIME_TYPE time_capacity = process->processus_status->ATTRIBUTES.TIME_CAPACITY;
    uint64_t uptime = ucx_uptime();
    if (time_capacity < INFINITE_TIME_VALUE ||
        (time_capacity >= 0 && time_overflow(uptime + (uint64_t)time_capacity))){
        *RETURN_CODE = INVALID_CONFIG;
        return;
    }

    // le process est aperiodic
    if (process->processus_status->ATTRIBUTES.PERIOD == INFINITE_TIME_VALUE){
        process->processus_status->CURRENT_PRIORITY = process->processus_status->ATTRIBUTES.BASE_PRIORITY;
        _context_init(&process->tcb.context, (size_t)process->tcb.stack,process->tcb.stack_sz, (size_t)process->tcb.task);

        if(partition->status->OPERATING_MODE == NORMAL){

            process->processus_status->PROCESS_STATE = READY;
            //calculer la deadline 
            update_process_deadline(process,(SYSTEM_TIME_TYPE)uptime);
            //check for rescheduling
            struct process_s *current_process = partition->process_current->data;
            yield_to_partition(partition, current_process);

        }
        else{
            process->processus_status->PROCESS_STATE = WAITING;
        }
        *RETURN_CODE = NO_ERROR;

    }
    // le process est periodic
    else{
        process->processus_status->CURRENT_PRIORITY = process->processus_status->ATTRIBUTES.BASE_PRIORITY;
        _context_init(&process->tcb.context, (size_t)process->tcb.stack,process->tcb.stack_sz, (size_t)process->tcb.task);
        if(partition->status->OPERATING_MODE == NORMAL){
            process->processus_status->PROCESS_STATE = WAITING;
            // trouver le fisrt release point
            process->release_point_time = arinc_time_find_first_release_point(partition);
            //calculer la deadline 
            update_process_deadline(process, process->release_point_time);
            //gerer les trucs avec releases point
        }
        else{
            process->processus_status->PROCESS_STATE = WAITING;
        }
        *RETURN_CODE = NO_ERROR;
    }



}

void DELAYED_START (
       /*in */ PROCESS_ID_TYPE          PROCESS_ID,
       /*in */ SYSTEM_TIME_TYPE         DELAY_TIME,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
    struct pcb_s *partition = get_current_partition();
    struct node_s *process_node = is_process_id_existed(partition, PROCESS_ID);
    struct process_s *process = process_node->data;
    
    if(!process_node){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    if (process->processus_status->PROCESS_STATE != DORMANT)
    {
        *RETURN_CODE = NO_ACTION;
        return;
    }

    uint64_t uptime = ucx_uptime();
    if (DELAY_TIME < INFINITE_TIME_VALUE ||
        (DELAY_TIME >= 0 && time_overflow(uptime + (uint64_t)DELAY_TIME))){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    if (DELAY_TIME == INFINITE_TIME_VALUE){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    if(process->processus_status->ATTRIBUTES.PERIOD != INFINITE_TIME_VALUE && DELAY_TIME >= process->processus_status->ATTRIBUTES.PERIOD){
        *RETURN_CODE = INVALID_PARAM;
        return;    
    }
    
    // when (DEADLINE_TIME calculation is out of range) => INVALID_CONFIG
    SYSTEM_TIME_TYPE time_capacity = process->processus_status->ATTRIBUTES.TIME_CAPACITY;
    if (time_capacity < INFINITE_TIME_VALUE ||
        (time_capacity >= 0 && time_overflow(uptime + (uint64_t)time_capacity + (uint64_t)DELAY_TIME))){
        *RETURN_CODE = INVALID_CONFIG;
        return;
    }

    // le process est aperiodic
    if (process->processus_status->ATTRIBUTES.PERIOD == INFINITE_TIME_VALUE){
        process->processus_status->CURRENT_PRIORITY = process->processus_status->ATTRIBUTES.BASE_PRIORITY;
        _context_init(&process->tcb.context, (size_t)process->tcb.stack,process->tcb.stack_sz, (size_t)process->tcb.task);

        if(partition->status->OPERATING_MODE == NORMAL){
            if (!DELAY_TIME){
                process->processus_status->PROCESS_STATE = READY;
                update_process_deadline(process, (SYSTEM_TIME_TYPE)uptime);
            }
            else {            
                process->processus_status->PROCESS_STATE = WAITING;
                //calculer la deadline 
                update_process_deadline(process, (SYSTEM_TIME_TYPE)uptime + DELAY_TIME);

                process->time_counter = (SYSTEM_TIME_TYPE)uptime + DELAY_TIME;
            }
            //check for rescheduling
            struct process_s *current_process = partition->process_current->data;
            yield_to_partition(partition, current_process);

        }
        else{
            process->processus_status->PROCESS_STATE = WAITING;
            process->saved_init_delay = DELAY_TIME;
        }
        *RETURN_CODE = NO_ERROR;

    }
    // le process est periodic
    else{
        process->processus_status->CURRENT_PRIORITY = process->processus_status->ATTRIBUTES.BASE_PRIORITY;
        _context_init(&process->tcb.context, (size_t)process->tcb.stack,process->tcb.stack_sz, (size_t)process->tcb.task);
        if(partition->status->OPERATING_MODE == NORMAL){
            process->processus_status->PROCESS_STATE = WAITING;
            // trouver le fisrt release point
            process->release_point_time = arinc_time_find_first_release_point(partition) + DELAY_TIME;
            //calculer la deadline 
            update_process_deadline(process, process->release_point_time);

        }
        else{
            process->processus_status->PROCESS_STATE = WAITING;
            process->saved_init_delay = DELAY_TIME;
        }
        *RETURN_CODE = NO_ERROR;
    }
}

// to do pendant la partie intra communication
void LOCK_PREEMPTION (
       /*out*/ LOCK_LEVEL_TYPE          *LOCK_LEVEL,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
    // to do
    struct pcb_s *partition = get_current_partition();
    if (partition->status->OPERATING_MODE != NORMAL){
        *LOCK_LEVEL = partition->status->LOCK_LEVEL;
        *RETURN_CODE = NO_ACTION;
        return;
    }

    else {
        krnl_acquire_mutex(PREEMPTION_LOCK_MUTEX, INFINITE_TIME_VALUE, RETURN_CODE);

        RETURN_CODE_TYPE return_code;
        MUTEX_STATUS_TYPE mutex_status;
        GET_MUTEX_STATUS(PREEMPTION_LOCK_MUTEX, &mutex_status, &return_code);
        if (return_code == NO_ERROR){
            partition->status->LOCK_LEVEL = mutex_status.LOCK_COUNT;
        }
        *LOCK_LEVEL = partition->status->LOCK_LEVEL;
    }
}

// to do pendant la partie intra communication
void UNLOCK_PREEMPTION (
       /*out*/ LOCK_LEVEL_TYPE          *LOCK_LEVEL,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
    // to do 
    struct pcb_s *partition = get_current_partition();
    struct process_s *current_process = partition->process_current->data;
    if (partition->status->OPERATING_MODE != NORMAL && partition->status->LOCK_LEVEL == 0){
        *RETURN_CODE = NO_ACTION;
        *LOCK_LEVEL = partition->status->LOCK_LEVEL;
        return;
    }
    else if (current_process->owned_mutex_id != PREEMPTION_LOCK_MUTEX){
        *LOCK_LEVEL = partition->status->LOCK_LEVEL;
        *RETURN_CODE = INVALID_MODE;
        return;
    }
    else {
        krnl_release_mutex(PREEMPTION_LOCK_MUTEX, RETURN_CODE);

        RETURN_CODE_TYPE return_code;
        MUTEX_STATUS_TYPE mutex_status;
        GET_MUTEX_STATUS(PREEMPTION_LOCK_MUTEX, &mutex_status, &return_code);
        if (return_code == NO_ERROR){
            partition->status->LOCK_LEVEL = mutex_status.LOCK_COUNT;
        }
        *LOCK_LEVEL = partition->status->LOCK_LEVEL;
    }
}


// to do 
int is_executing_error_handler(){
    return 0;
}
void GET_MY_ID (
       /*out*/ PROCESS_ID_TYPE          *PROCESS_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE )
{
    struct pcb_s *partition = get_current_partition();
    struct process_s *process = partition->process_current->data;

    // 2. Vérifier si le code actuel est le Error Handler (tâche spéciale)
    if (is_executing_error_handler()) {
        *RETURN_CODE = INVALID_MODE;
        return;
    }
    
    *PROCESS_ID = process->process_id;
    *RETURN_CODE = NO_ERROR;
}


void GET_PROCESS_ID (
       /*in */ PROCESS_NAME_TYPE        PROCESS_NAME,
       /*out*/ PROCESS_ID_TYPE          *PROCESS_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
    struct pcb_s *partition = get_current_partition();
    struct node_s *process_node = find_process_node_by_name(partition, PROCESS_NAME);
    if(!process_node){
        *RETURN_CODE = INVALID_CONFIG;
        return;
    }
    struct process_s *process = process_node->data;
    *PROCESS_ID = process->process_id;
    *RETURN_CODE = NO_ERROR;
}

void GET_PROCESS_STATUS (
       /*in */ PROCESS_ID_TYPE          PROCESS_ID,
       /*out*/ PROCESS_STATUS_TYPE      *PROCESS_STATUS,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
    struct pcb_s *partition = get_current_partition();
    struct node_s *process_node = is_process_id_existed(partition, PROCESS_ID);
    if(!process_node){
        *RETURN_CODE = INVALID_CONFIG;
        return;
    }
    struct process_s *process = process_node->data;
    *PROCESS_STATUS = *process->processus_status;
    *RETURN_CODE = NO_ERROR;
}

void INITIALIZE_PROCESS_CORE_AFFINITY (
       /*in */ PROCESS_ID_TYPE          PROCESS_ID,
       /*in */ PROCESSOR_CORE_ID_TYPE   PROCESSOR_CORE_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
    struct pcb_s *partition = get_current_partition();
    struct node_s *process_node = is_process_id_existed(partition, PROCESS_ID);
    if(!process_node){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    // when (PROCESSOR_CORE_ID does not identify a processor core assigned to this partition) =>
    if (PROCESSOR_CORE_ID != DEFAULT_PROCESS_CORE_AFFINITY){
        *RETURN_CODE = INVALID_CONFIG;
        return;
    }

    if (partition->status->OPERATING_MODE == NORMAL){
        *RETURN_CODE = INVALID_MODE;
        return;
    }
    struct process_s *process = process_node->data;
    process->processor_core_affinity = PROCESSOR_CORE_ID;
    *RETURN_CODE = NO_ERROR;
}

void GET_MY_PROCESSOR_CORE_ID (
       /*out*/ PROCESSOR_CORE_ID_TYPE   *PROCESSOR_CORE_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){

    struct pcb_s *partition = get_current_partition();
    struct process_s *process = partition->process_current->data;
    
    *PROCESSOR_CORE_ID = process->processor_core_affinity;
    *RETURN_CODE = NO_ERROR;
}

void GET_MY_INDEX (
       /*out*/ PROCESS_INDEX_TYPE       *PROCESS_INDEX,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){

    struct pcb_s *partition = get_current_partition();
    struct process_s *process = partition->process_current->data;

    // 2. Vérifier si le code actuel est le Error Handler (tâche spéciale) ou si le main process la appele
    if (is_executing_error_handler()) {
        *RETURN_CODE = INVALID_MODE;
        return;
    }
    
    *PROCESS_INDEX = process->process_index;
    *RETURN_CODE = NO_ERROR;
    
}