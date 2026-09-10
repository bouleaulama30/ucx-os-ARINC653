/* file:          mutex.c
 * description:   mutex kernel implementation
 * date:          07/2026
 */

#include <ucx.h>

void krnl_acquire_mutex(/*in */ MUTEX_ID_TYPE            MUTEX_ID,
       /*in */ SYSTEM_TIME_TYPE         TIME_OUT,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE){

    struct pcb_s *partition = get_current_partition();
    int index = find_mutex_by_id(partition, MUTEX_ID);
    if (index == -1){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }


    if (TIME_OUT < INFINITE_TIME_VALUE || (TIME_OUT >= 0 && time_overflow(ucx_uptime() + (SYSTEM_TIME_TYPE)TIME_OUT))){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    struct node_s *current_process_node = partition->process_current;
    struct process_s *current_process = current_process_node->data;
    if (current_process->owned_mutex_id != NO_MUTEX_OWNED && current_process->owned_mutex_id != MUTEX_ID){
        *RETURN_CODE = INVALID_MODE;
        return;
    }
    
    if (current_process == partition->error_handler_process){
        *RETURN_CODE = INVALID_MODE;
        return;
    }
        
    struct mutex_s *mutex = &partition->mutexes[index];
    if (current_process->processus_status->CURRENT_PRIORITY > mutex->mutex_status.MUTEX_PRIORITY){
        printf("TEST\n");
        *RETURN_CODE = INVALID_MODE;
        return;
    }


    if (mutex->mutex_status.MUTEX_STATE == AVAILABLE){
        mutex->mutex_status.MUTEX_STATE = OWNED;
        current_process->owned_mutex_id = MUTEX_ID;
        mutex->mutex_status.MUTEX_OWNER = current_process->process_id;
        mutex->mutex_status.LOCK_COUNT ++;
        mutex->saved_owner_priority = current_process->processus_status->CURRENT_PRIORITY;
        current_process->processus_status->CURRENT_PRIORITY = mutex->mutex_status.MUTEX_PRIORITY;
        list_remove(partition->processes, current_process_node);
        struct node_s *new_process_node = list_push(partition->processes, current_process);
        partition->process_current = new_process_node;

        *RETURN_CODE = NO_ERROR;
    }
    else if (mutex->mutex_status.MUTEX_STATE == OWNED && mutex->mutex_status.MUTEX_OWNER == current_process->process_id){
        if (mutex->mutex_status.LOCK_COUNT == MAX_LOCK_LEVEL){
            *RETURN_CODE = INVALID_CONFIG;
            return;
        } else {
            mutex->mutex_status.LOCK_COUNT ++;
            *RETURN_CODE = NO_ERROR;
        }
    }
    else if (TIME_OUT == 0){
        *RETURN_CODE = NOT_AVAILABLE;
    }

    else if (TIME_OUT == INFINITE_TIME_VALUE){
        printf("ACQUIRE_MUTEX WAIT INFINITE \n");
        current_process->processus_status->PROCESS_STATE = WAITING;
        mutex->mutex_status.WAITING_PROCESSES++;
        current_process->waiting_mutex = mutex;
        if (mutex->queuing_discipline == PRIORITY){
            list_insert_sorted(mutex->waiting_processes, current_process);
        }
        else {
            list_pushback(mutex->waiting_processes, current_process);
        }
        yield_to_partition(partition, current_process);
        *RETURN_CODE = NO_ERROR;
    } else {
        current_process->processus_status->PROCESS_STATE = WAITING;
        mutex->mutex_status.WAITING_PROCESSES++;
        current_process->waiting_mutex = mutex;
        if (mutex->queuing_discipline == PRIORITY){
            list_insert_sorted(mutex->waiting_processes, current_process);
        }
        else {
            list_pushback(mutex->waiting_processes, current_process);
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

void krnl_release_mutex(/*in */ MUTEX_ID_TYPE            MUTEX_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE){
    struct pcb_s *partition = get_current_partition();
    int index = find_mutex_by_id(partition, MUTEX_ID);
    if (index == -1){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    struct node_s *current_process_node = partition->process_current;
    struct process_s *current_process = current_process_node->data;
    if (current_process->owned_mutex_id != MUTEX_ID){
        *RETURN_CODE = INVALID_MODE;
        return;
    }

    struct mutex_s *mutex = &partition->mutexes[index];
    mutex->mutex_status.LOCK_COUNT --;
    if (mutex->mutex_status.LOCK_COUNT == 0){
        mutex->mutex_status.MUTEX_STATE = AVAILABLE;
        current_process->owned_mutex_id = NO_MUTEX_OWNED;
        mutex->mutex_status.MUTEX_OWNER = NULL_PROCESS_ID;
        current_process->processus_status->CURRENT_PRIORITY = mutex->saved_owner_priority;
        list_remove(partition->processes, current_process_node);
        struct node_s *new_process_node = list_push(partition->processes, current_process);
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
            woken_process->owned_mutex_id = MUTEX_ID;
            mutex->saved_owner_priority = woken_process->processus_status->CURRENT_PRIORITY;
            woken_process->processus_status->CURRENT_PRIORITY = mutex->mutex_status.MUTEX_PRIORITY;
            woken_process->waiting_mutex = NULL;
            struct node_s *woken_process_node = is_process_id_existed(partition, woken_process->process_id);
            list_remove(partition->processes, woken_process_node);
            list_push(partition->processes, woken_process);
            woken_process->processus_status->PROCESS_STATE = READY;
        }
        yield_to_partition(partition, current_process);
    }
    *RETURN_CODE = NO_ERROR;
}
