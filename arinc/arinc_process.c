#include <ucx.h>

static struct node_s *find_processes_by_name(struct node_s *node, void *arg){
    struct process_s *process = node->data;
    const char *name = (const char *) arg;
    
    if(strcmp(process->processus_status->ATTRIBUTES.NAME, name) == 0){
        return node;
    }
    return 0;
}

static struct node_s *is_process_name_existed(struct pcb_s *partition, PROCESS_NAME_TYPE process_name){
    struct node_s *same_process_name_node = list_foreach(partition->processes, find_processes_by_name, (void *)process_name);
    if(same_process_name_node){
        return same_process_name_node;
    }
    return 0;
}

static struct node_s *find_processes_by_id(struct node_s *node, void *arg){
    struct process_s *process = node->data;
    PROCESS_ID_TYPE id = (PROCESS_ID_TYPE) arg;
    
    if(id == process->process_id){
        return node;
    }
    return 0;
}

static struct node_s *is_process_id_existed(struct pcb_s *partition, PROCESS_ID_TYPE process_id){
    struct node_s *same_process_name_node = list_foreach(partition->processes, find_processes_by_id, (void *)process_id);
    if(same_process_name_node){
        return same_process_name_node;
    }
    return 0;
}

void CREATE_PROCESS (
       /*in */ PROCESS_ATTRIBUTE_TYPE   *ATTRIBUTES,
       /*out*/ PROCESS_ID_TYPE          *PROCESS_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){

#ifndef MULTICORE
    struct node_s *partition_node = kcb->partition_current;
#else
    struct node_s *partition_node = kcb[_cpu_id()]->partition_current;
#endif
    struct pcb_s *partition = partition_node->data;
    if(partition->nbr_processes >= MAX_NUMBER_OF_PROCESSES){
        *RETURN_CODE = INVALID_CONFIG;
    } 
    
    else if ((partition->next_stack_addr + ATTRIBUTES->STACK_SIZE) > 
        (partition->memory_requirements->memory[DATA].base + partition->memory_requirements->memory[DATA].size))
    {
        *RETURN_CODE = INVALID_CONFIG;
    }
    else if (is_process_name_existed(partition, ATTRIBUTES->NAME))
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
        
        // a changer
        status->DEADLINE_TIME = 0;
        status->CURRENT_PRIORITY = ATTRIBUTES->BASE_PRIORITY;
        status->PROCESS_STATE = DORMANT;
        status->ATTRIBUTES = *ATTRIBUTES;

        new_process->processus_status = status;

        int32_t id = ucx_process_spawn(ATTRIBUTES->ENTRY_POINT, ATTRIBUTES->STACK_SIZE, new_process, partition);
        new_process->process_id = id;
        new_process->process_index = partition->nbr_processes;
        new_process->processor_core_affinity = DEFAULT_PROCESS_CORE_AFFINITY;
       
        list_pushback(partition->processes, new_process);
        
        *PROCESS_ID = new_process->process_id;
        *RETURN_CODE = NO_ERROR;
    }
}

void SET_PRIORITY (
       /*in */ PROCESS_ID_TYPE          PROCESS_ID,
       /*in */ PRIORITY_TYPE            PRIORITY,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
#ifndef MULTICORE
    struct node_s *partition_node = kcb->partition_current;
#else
    struct node_s *partition_node = kcb[_cpu_id()]->partition_current;
#endif
    struct pcb_s *partition = partition_node->data;
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

//     normal
// if (the specified process owns a mutex) thenARINC SPECIFICATION 653 PART 1 – PAGE 61
// 3.0 SERVICE REQUIREMENTS
// -- current priority of the process cannot be modified without
// -- impacting mutex properties
// set the retained priority of the specified process to PRIORITY;
// else

    process->processus_status->CURRENT_PRIORITY = PRIORITY;
    *RETURN_CODE = NO_ERROR;


    // sauvegarde du context du process actuel et on reschedule
    struct process_s *current_process = partition->process_current->data;
    if (setjmp(current_process->tcb.context) == 0) {
        /* Retourner au contexte du kernel (partition_OS) */
        longjmp(partition->partition_context, 1);
    }
}

void SUSPEND_SELF (
       /*in */ SYSTEM_TIME_TYPE         TIME_OUT,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

void SUSPEND (
       /*in */ PROCESS_ID_TYPE          PROCESS_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

void RESUME (
       /*in */ PROCESS_ID_TYPE          PROCESS_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

void STOP_SELF (void);

void STOP (
       /*in */ PROCESS_ID_TYPE          PROCESS_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

void START (
       /*in */ PROCESS_ID_TYPE          PROCESS_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

void DELAYED_START (
       /*in */ PROCESS_ID_TYPE          PROCESS_ID,
       /*in */ SYSTEM_TIME_TYPE         DELAY_TIME,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

void LOCK_PREEMPTION (
       /*out*/ LOCK_LEVEL_TYPE          *LOCK_LEVEL,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

void UNLOCK_PREEMPTION (
       /*out*/ LOCK_LEVEL_TYPE          *LOCK_LEVEL,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );


// to do 
int is_executing_error_handler(){
    return 0;
}
void GET_MY_ID (
       /*out*/ PROCESS_ID_TYPE          *PROCESS_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE )
{
#ifndef MULTICORE
    struct pcb_s *partition = kcb->partition_current->data;
#else
    struct pcb_s *partition = kcb[_cpu_id()]->partition_current->data;
#endif
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
#ifndef MULTICORE
    struct node_s *partition_node = kcb->partition_current;
#else
    struct node_s *partition_node = kcb[_cpu_id()]->partition_current;
#endif
    struct pcb_s *partition = partition_node->data;
    struct node_s *process_node = is_process_name_existed(partition, PROCESS_NAME);
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
#ifndef MULTICORE
    struct node_s *partition_node = kcb->partition_current;
#else
    struct node_s *partition_node = kcb[_cpu_id()]->partition_current;
#endif
    struct pcb_s *partition = partition_node->data;
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
#ifndef MULTICORE
    struct node_s *partition_node = kcb->partition_current;
#else
    struct node_s *partition_node = kcb[_cpu_id()]->partition_current;
#endif
    struct pcb_s *partition = partition_node->data;
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
#ifndef MULTICORE
    struct pcb_s *partition = kcb->partition_current->data;
#else
    struct pcb_s *partition = kcb[_cpu_id()]->partition_current->data;
#endif
    struct process_s *process = partition->process_current->data;
    
    *PROCESSOR_CORE_ID = process->processor_core_affinity;
    *RETURN_CODE = NO_ERROR;
}

void GET_MY_INDEX (
       /*out*/ PROCESS_INDEX_TYPE       *PROCESS_INDEX,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );