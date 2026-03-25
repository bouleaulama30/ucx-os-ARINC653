#include <ucx.h>

void partition_OS(void)
{
    struct pcb_s *partition = get_current_partition();    
    _mprv_activate();
    
start_over:
    setjmp(partition->partition_context);
	if(partition->status->OPERATING_MODE == COLD_START || partition->status->OPERATING_MODE == WARM_START){
        ((void (*)(struct pcb_s *))partition->entry_point)(partition);
	}
    while (1) {
        if(partition->status->OPERATING_MODE == COLD_START || partition->status->OPERATING_MODE == WARM_START){
            goto start_over;
        }

        uint32_t current_tick = ucx_ticks();
        if (current_tick != partition->last_tick) {
            partition->last_tick = current_tick;
            arinc_time_update_partition(partition);
        }
    
        if (!setjmp(partition->partition_context)) {
            process_schedule();

            if (partition->process_current != NULL && partition->status->OPERATING_MODE == NORMAL) {
                struct process_s *process = partition->process_current->data;
                _interrupt_tick_process();
                longjmp(process->tcb.context, 1); 
            } 
            else {
                asm volatile ("csrs mstatus, 8"); 
                _cpu_idle(); 
            }
        }
    }
}





void GET_PARTITION_STATUS (
    /*out*/ PARTITION_STATUS_TYPE      *PARTITION_STATUS,
    /*out*/ RETURN_CODE_TYPE           *RETURN_CODE ){
    struct pcb_s* my_partition = get_current_partition();    

    if (!my_partition->status) {
        *RETURN_CODE = NOT_AVAILABLE;
        return;
    }

    *PARTITION_STATUS = *(my_partition->status);
    *RETURN_CODE = NOT_AVAILABLE;       
}



static struct node_s *start_process(struct node_s *node, void *arg)
{
	struct process_s *process = node->data;
	SYSTEM_TIME_TYPE first_release_point = (SYSTEM_TIME_TYPE) arg;

    if (process->processus_status->PROCESS_STATE == WAITING) {
        if (process->processus_status->ATTRIBUTES.PERIOD == INFINITE_TIME_VALUE) { // Apériodique
            if (!process->saved_init_delay && !process->is_suspended) {
                process->processus_status->PROCESS_STATE = READY;
                update_process_deadline(process, (SYSTEM_TIME_TYPE)ucx_uptime());
                return 0;
            } else if (process->saved_init_delay) {
                process->release_point_time = ucx_uptime() + process->saved_init_delay;
            }
        } else { // Périodique
            process->release_point_time = first_release_point + process->saved_init_delay;
        }
    }
    // calculate the DEADLINE_TIME of all non-dormant processes in the partition;
    if(process->processus_status->PROCESS_STATE != DORMANT){    
        update_process_deadline(process, process->release_point_time);
        return 0;
    }



    return 0;
}


void SET_PARTITION_MODE (
       /*in */ OPERATING_MODE_TYPE        OPERATING_MODE,
       /*out*/ RETURN_CODE_TYPE           *RETURN_CODE ){

    struct pcb_s* my_partition = get_current_partition();
    // error
    if(OPERATING_MODE != IDLE && OPERATING_MODE != COLD_START && OPERATING_MODE != WARM_START && OPERATING_MODE != NORMAL){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    if(OPERATING_MODE == NORMAL && my_partition->status->OPERATING_MODE == NORMAL){
        *RETURN_CODE = NO_ACTION;
        return;
    }

    if(OPERATING_MODE == WARM_START && my_partition->status->OPERATING_MODE == COLD_START){
        *RETURN_CODE = INVALID_MODE;
        return;
    }

    // normal
    my_partition->status->OPERATING_MODE = OPERATING_MODE;
    
    if (OPERATING_MODE == IDLE)
    {   
        // printf("OPERATING MODE is IDLE\n");
        my_partition->process_current = NULL;
        signal_idle_current_partition();
        *RETURN_CODE = NO_ERROR;
        longjmp(my_partition->partition_context, 1);
    }

    if (OPERATING_MODE == WARM_START || OPERATING_MODE == COLD_START)
    {
        // inhibit process scheduling and switch back to initialization mode
        // printf("OPERATING MODE is WARM START or COLD START\n");
        my_partition->process_current = NULL;
        my_partition->nbr_processes = 0;
        my_partition->id_next = 0;
        my_partition->next_stack_addr = my_partition->memory_requirements->memory[DATA].base + PARTIION_OS_AND_MAIN_PROCESS_STACK_SIZE;

        // on vide la liste des processes de la partition
        while (my_partition->processes->length != 0)
        {
            struct process_s *process  = list_pop(my_partition->processes);
            free(process->processus_status);
            free(process);
        }
        *RETURN_CODE = NO_ERROR;
        longjmp(my_partition->partition_context, 1);
    }
    
    if (OPERATING_MODE == NORMAL)
    {
        // printf("OPERATING MODE is NORMAL\n");
        *RETURN_CODE = NO_ERROR;

        SYSTEM_TIME_TYPE first_release_point = arinc_time_find_first_release_point(my_partition);
        list_foreach(my_partition->processes, start_process, (void *)first_release_point);
        longjmp(my_partition->partition_context, 1);
    }
    
    *RETURN_CODE = NO_ERROR;

}
       
void GET_MY_PARTITION_ID(
               /*out*/ PARTITION_ID_TYPE          *PARTITION_ID,
              /*out*/ RETURN_CODE_TYPE           *RETURN_CODE )
{
    struct pcb_s* my_partition = get_current_partition();
    *PARTITION_ID = my_partition->status->IDENTIFIER;
    if(*PARTITION_ID)
        *RETURN_CODE = NO_ERROR;
    else
        *RETURN_CODE = NOT_AVAILABLE;       
}
