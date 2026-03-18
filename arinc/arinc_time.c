#include "ucx.h"

extern void TIMED_WAIT (
       /*in */ SYSTEM_TIME_TYPE         DELAY_TIME,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
#ifndef MULTICORE
    struct node_s *partition_node = kcb->partition_current;
#else
    struct node_s *partition_node = kcb[_cpu_id()]->partition_current;
#endif
    struct pcb_s *partition = partition_node->data;
    struct node_s *current_process_node = partition->process_current; 
    struct process_s *current_process = current_process_node->data;
    
    uint64_t uptime = ucx_uptime();
    uint64_t max_system_time = 0x7fffffffffffffffULL;
    if (DELAY_TIME < -1 || uptime > (max_system_time - (uint64_t)DELAY_TIME)){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    if(DELAY_TIME == INFINITE_TIME_VALUE){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    if(!DELAY_TIME){
        current_process->processus_status->PROCESS_STATE = READY;
        list_remove(partition->processes, current_process_node);
        struct node_s *new_current_process_node = list_pushback(partition->processes, current_process);

        // il faut update le nouveau node au processus courant
        partition->process_current = new_current_process_node;
        
        struct process_s *current_process = partition->process_current->data;
        if (setjmp(current_process->tcb.context) == 0) {
            /* Retourner au contexte du kernel (partition_OS) */
            longjmp(partition->partition_context, 1);
        }
    }
    else{
        current_process->processus_status->PROCESS_STATE = WAITING;
        current_process->time_counter = (SYSTEM_TIME_TYPE)ucx_uptime() + DELAY_TIME;
        if (setjmp(current_process->tcb.context) == 0) {
        /* Retourner au contexte du kernel (partition_OS) */
            longjmp(partition->partition_context, 1);
        }

    }
    *RETURN_CODE = NO_ERROR;
}

extern void PERIODIC_WAIT (
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
#ifndef MULTICORE
    struct node_s *partition_node = kcb->partition_current;
#else
    struct node_s *partition_node = kcb[_cpu_id()]->partition_current;
#endif
    struct pcb_s *partition = partition_node->data;
    struct node_s *current_process_node = partition->process_current; 
    struct process_s *current_process = current_process_node->data;
    
    if(current_process->processus_status->ATTRIBUTES.PERIOD == INFINITE_TIME_VALUE){
        *RETURN_CODE = INVALID_MODE;
        return;
    }

    uint64_t uptime = ucx_uptime();
    uint64_t max_system_time = 0x7fffffffffffffffULL;
    uint64_t futur_deadline_time = (uint64_t)current_process->processus_status->ATTRIBUTES.PERIOD + (uint64_t)current_process->release_point_time + (uint64_t)current_process->processus_status->ATTRIBUTES.TIME_CAPACITY;
    if ((max_system_time - futur_deadline_time) < 0){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    current_process->processus_status->PROCESS_STATE = WAITING;
    current_process->release_point_time += current_process->processus_status->ATTRIBUTES.PERIOD;
    if(current_process->processus_status->ATTRIBUTES.TIME_CAPACITY == INFINITE_TIME_VALUE)
        current_process->processus_status->DEADLINE_TIME = INFINITE_TIME_VALUE;
    else     
        current_process->processus_status->DEADLINE_TIME = current_process->release_point_time + current_process->processus_status->ATTRIBUTES.TIME_CAPACITY; 

    if (setjmp(current_process->tcb.context) == 0) {
    /* Retourner au contexte du kernel (partition_OS) */
        longjmp(partition->partition_context, 1);
    }
    *RETURN_CODE = NO_ERROR;

}

extern void GET_TIME (
       /*out*/ SYSTEM_TIME_TYPE         *SYSTEM_TIME,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
       uint64_t current_time = ucx_uptime() * 1000000;
       *SYSTEM_TIME = (SYSTEM_TIME_TYPE) current_time;
       *RETURN_CODE = NO_ERROR;
}

extern void REPLENISH (
       /*in */ SYSTEM_TIME_TYPE         BUDGET_TIME,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
#ifndef MULTICORE
    struct node_s *partition_node = kcb->partition_current;
#else
    struct node_s *partition_node = kcb[_cpu_id()]->partition_current;
#endif
    struct pcb_s *partition = partition_node->data;
    struct node_s *current_process_node = partition->process_current; 
    struct process_s *current_process = current_process_node->data;
    

    uint64_t uptime = ucx_uptime();
    uint64_t max_system_time = 0x7fffffffffffffffULL;
    if (BUDGET_TIME < -1 || uptime > (max_system_time - (uint64_t)BUDGET_TIME)){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    if(current_process->processus_status->ATTRIBUTES.PERIOD != INFINITE_TIME_VALUE && (BUDGET_TIME == INFINITE_TIME_VALUE || ((SYSTEM_TIME_TYPE)uptime + BUDGET_TIME) > current_process->release_point_time)){
        *RETURN_CODE = INVALID_MODE;
        return;
    }

    if(current_process->processus_status->ATTRIBUTES.TIME_CAPACITY == INFINITE_TIME_VALUE)
        current_process->processus_status->DEADLINE_TIME = INFINITE_TIME_VALUE;
    else if(current_process->processus_status->ATTRIBUTES.PERIOD == INFINITE_TIME_VALUE && BUDGET_TIME == INFINITE_TIME_VALUE)
        current_process->processus_status->DEADLINE_TIME = INFINITE_TIME_VALUE;
    else{
        current_process->processus_status->DEADLINE_TIME = (SYSTEM_TIME_TYPE)ucx_uptime() + BUDGET_TIME;
    
    }

    *RETURN_CODE = NO_ERROR;
}
