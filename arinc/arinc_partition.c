#include <ucx.h>

static struct node_s *check_available_resources_on_port(struct node_s *node, void *arg) {
    struct queuing_port_s *queuing_port = node->data;
    struct krnl_queuing_channel_s *channel = queuing_port->channel;

    if(channel->current_nb_messages < channel->max_nb_messages && queuing_port->queuing_port_status.MAX_MESSAGE_SIZE == SOURCE && queuing_port->waiting_processes->length != 0){
        struct process_s *waiting_process = list_pop(queuing_port->waiting_processes);
        waiting_process->processus_status->PROCESS_STATE = READY;
        queuing_port->queuing_port_status.WAITING_PROCESSES--;
        waiting_process->waiting_queuing_port = NULL;
    }

    if(channel->current_nb_messages > 0 && queuing_port->queuing_port_status.PORT_DIRECTION == DESTINATION && queuing_port->waiting_processes->length != 0){
        struct process_s *waiting_process = list_pop(queuing_port->waiting_processes);
        waiting_process->processus_status->PROCESS_STATE = READY;
        queuing_port->queuing_port_status.WAITING_PROCESSES--;
        waiting_process->waiting_queuing_port = NULL;
    }

    return 0;
}

void partition_OS(void)
{
    struct pcb_s *partition = get_current_partition();    
    _mprv_activate();

    // create a mutex for preemption lock
    int port_id;
    RETURN_CODE_TYPE return_code0, return_code1;
    CREATE_MUTEX("PREEMPTION_LOCK", MAX_PRIORITY_VALUE, FIFO, &port_id, &return_code0);
    printf("return code mutex %d, mutex id %d\n", return_code0, port_id);
    
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
            list_foreach(partition->communication_queuing_ports, check_available_resources_on_port, NULL);
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
    struct pcb_s* partition = get_current_partition();    

    if (!partition->status) {
        *RETURN_CODE = NOT_AVAILABLE;
        return;
    }

    *PARTITION_STATUS = *(partition->status);
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

    struct pcb_s* partition = get_current_partition();
    // error
    if(OPERATING_MODE != IDLE && OPERATING_MODE != COLD_START && OPERATING_MODE != WARM_START && OPERATING_MODE != NORMAL){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    if(OPERATING_MODE == NORMAL && partition->status->OPERATING_MODE == NORMAL){
        *RETURN_CODE = NO_ACTION;
        return;
    }

    if(OPERATING_MODE == WARM_START && partition->status->OPERATING_MODE == COLD_START){
        *RETURN_CODE = INVALID_MODE;
        return;
    }

    // normal
    partition->status->OPERATING_MODE = OPERATING_MODE;
    
    if (OPERATING_MODE == IDLE)
    {   
        // printf("OPERATING MODE is IDLE\n");
        partition->process_current = NULL;
        signal_idle_current_partition();
        *RETURN_CODE = NO_ERROR;
        longjmp(partition->partition_context, 1);
    }

    if (OPERATING_MODE == WARM_START || OPERATING_MODE == COLD_START)
    {
        // inhibit process scheduling and switch back to initialization mode
        // printf("OPERATING MODE is WARM START or COLD START\n");
        partition->process_current = NULL;
        partition->nbr_processes = 0;
        partition->id_next = 0;
        partition->next_stack_addr = partition->memory_requirements->memory[DATA].base + PARTIION_OS_AND_MAIN_PROCESS_STACK_SIZE;

        // on vide la liste des processes de la partition
        while (partition->processes->length != 0)
        {
            struct process_s *process  = list_pop(partition->processes);
            free(process->processus_status);
            free(process);
        }
        *RETURN_CODE = NO_ERROR;
        longjmp(partition->partition_context, 1);
    }
    
    if (OPERATING_MODE == NORMAL)
    {
        // printf("OPERATING MODE is NORMAL\n");
        *RETURN_CODE = NO_ERROR;

        SYSTEM_TIME_TYPE first_release_point = arinc_time_find_first_release_point(partition);
        list_foreach(partition->processes, start_process, (void *)first_release_point);
        longjmp(partition->partition_context, 1);
    }
    
    *RETURN_CODE = NO_ERROR;

}
       
void GET_MY_PARTITION_ID(
               /*out*/ PARTITION_ID_TYPE          *PARTITION_ID,
              /*out*/ RETURN_CODE_TYPE           *RETURN_CODE )
{
    struct pcb_s* partition = get_current_partition();
    *PARTITION_ID = partition->status->IDENTIFIER;
    if(*PARTITION_ID)
        *RETURN_CODE = NO_ERROR;
    else
        *RETURN_CODE = NOT_AVAILABLE;       
}
