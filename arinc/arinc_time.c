#include "ucx.h"

static struct node_s *check_and_release_periodic_waiting_processes(struct node_s *node, void *arg)
{
    struct process_s *process = node->data;
    uint64_t current_time = (uint64_t)ucx_uptime();
    uint64_t rp_time = (uint64_t)process->release_point_time;
    
    // periodic waiting processes or aperiodic delayed processes
    if (process->processus_status->PROCESS_STATE == WAITING &&
        ((process->processus_status->ATTRIBUTES.PERIOD != INFINITE_TIME_VALUE) ||
         (process->saved_init_delay && process->processus_status->ATTRIBUTES.PERIOD == INFINITE_TIME_VALUE))) {
        
        printf("check_periodic proc: %d, uptime: %u, release point: %u\n", 
                process->process_id, 
                (unsigned)current_time, 
                (unsigned)rp_time);
        
        if(current_time >= rp_time) {
            printf("=> REVEIL ! release point time: %u\n", (unsigned)rp_time);
            process->processus_status->PROCESS_STATE = READY;
            
            if(process->saved_init_delay && process->processus_status->ATTRIBUTES.PERIOD == INFINITE_TIME_VALUE) {
                process->release_point_time = 0;
                process->saved_init_delay = 0;
            }
        }
    }
    return 0;
}

static struct node_s *check_timeouts(struct node_s *node, void *arg) {
    struct process_s *process = node->data;
    SYSTEM_TIME_TYPE current_time = (SYSTEM_TIME_TYPE)ucx_uptime();

    // Si le processus  a un chronomètre actif (différent de 0)
    if (process->time_counter != 0) {
        if (current_time >= process->time_counter) {
                        
            process->is_suspended = false;
            process->time_counter = 0;

            process->processus_status->PROCESS_STATE = READY;
        }
    }
    return 0;
}

static struct node_s *check_deadlines(struct node_s *node, void *arg) {
    struct process_s *process = node->data;
    SYSTEM_TIME_TYPE current_time = (SYSTEM_TIME_TYPE)ucx_uptime();

    if (current_time >= process->processus_status->DEADLINE_TIME && process->processus_status->DEADLINE_TIME != INFINITE_TIME_VALUE) {
        printf("ATTENTION LE PROCESS %d A DEPASSE SA DEADLINE QUI ETAIT DE %d\n", process->process_id, process->processus_status->DEADLINE_TIME);
    }
    return 0;
}

void arinc_time_update_partition(struct pcb_s *partition) {
        list_foreach(partition->processes, check_and_release_periodic_waiting_processes, (void *)0);
        list_foreach(partition->processes, check_timeouts, (void *)0);
        list_foreach(partition->processes, check_deadlines, (void *)0);
}


SYSTEM_TIME_TYPE  arinc_time_find_first_release_point(struct pcb_s *current_partition){
#ifndef MULTICORE
    struct mscb_s* ms = kcb->module_scheduler;
#else
    struct mscb_s* ms = kcb[_cpu_id()]->module_scheduler;
#endif
    window_partition_type window_partition ;
    SYSTEM_TIME_TYPE first_release_point;
    int windows_idx = (int) ms->windows_idx;
    int nbr_windows = (int) ms->nbr_windows;
    for(int i = windows_idx; i < nbr_windows; i++ ){
        window_partition = ms->windows_partition[i];
        if(current_partition->status->IDENTIFIER == window_partition.id){
            if(window_partition.is_periodic_processes_start){
                first_release_point = ms->major_frame_count*TICKS_TO_MS(ms->major_frame_tick) + TICKS_TO_MS(window_partition.start_tick);
                return first_release_point;
            }
        }
    }
    for(int i = 0; i < windows_idx; i++ ){
        window_partition = ms->windows_partition[i];
        if(current_partition->status->IDENTIFIER == window_partition.id){
            if(window_partition.is_periodic_processes_start){
                first_release_point = (ms->major_frame_count+1)*TICKS_TO_MS(ms->major_frame_tick) + TICKS_TO_MS(window_partition.start_tick);
                return first_release_point;
            }
        }
    }
}

extern void TIMED_WAIT (
       /*in */ SYSTEM_TIME_TYPE         DELAY_TIME,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){
    struct pcb_s *partition = get_current_partition();
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
    struct pcb_s *partition = get_current_partition();
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
    struct pcb_s *partition = get_current_partition();
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
