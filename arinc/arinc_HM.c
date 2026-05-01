#include "ucx.h"


void REPORT_APPLICATION_MESSAGE (
       /*in */   MESSAGE_ADDR_TYPE        MESSAGE_ADDR,
       /*in */   MESSAGE_SIZE_TYPE        LENGTH,
       /*out*/   RETURN_CODE_TYPE         *RETURN_CODE );

void CREATE_ERROR_HANDLER (
       /*in */   SYSTEM_ADDRESS_TYPE      ENTRY_POINT,
       /*in */   STACK_SIZE_TYPE          STACK_SIZE,
       /*out*/   RETURN_CODE_TYPE         *RETURN_CODE ){
       struct pcb_s *partition = get_current_partition();
       if(partition->error_handler_process != NULL){
            *RETURN_CODE = NO_ACTION;
            return;
       }

       if ((partition->next_stack_addr + STACK_SIZE) > 
        (partition->memory_requirements->memory[DATA].base + partition->memory_requirements->memory[DATA].size))
       {
              *RETURN_CODE = INVALID_CONFIG;
              return;
       }

       if (STACK_SIZE <= 0){
            *RETURN_CODE = INVALID_CONFIG;
            return;
       }

       if (partition->status->OPERATING_MODE == NORMAL){
            *RETURN_CODE = INVALID_MODE;
            return;
       }

       partition->nbr_processes++;

       struct process_s *new_process;
       PROCESS_STATUS_TYPE *status;

       new_process = malloc(sizeof(struct process_s));
       status = malloc(sizeof(PROCESS_STATUS_TYPE));
       
       status->DEADLINE_TIME = INFINITE_TIME_VALUE;
       status->ATTRIBUTES.PERIOD = INFINITE_TIME_VALUE;
       status->ATTRIBUTES.DEADLINE = INFINITE_TIME_VALUE;
       status->ATTRIBUTES.BASE_PRIORITY = MAX_PRIORITY_VALUE;
       status->CURRENT_PRIORITY = MAX_PRIORITY_VALUE;
       status->PROCESS_STATE = READY;
       status->ATTRIBUTES.NAME[0] = '\0';
       new_process->processus_status = status;

       int32_t id = ucx_process_spawn(ENTRY_POINT, STACK_SIZE, new_process, partition, 1);
       new_process->process_index = partition->nbr_processes;
       new_process->processor_core_affinity = DEFAULT_PROCESS_CORE_AFFINITY;
       new_process->release_point_time = 0; 
       new_process->is_suspended = false;
       new_process->time_counter = 0;
       new_process->saved_init_delay = 0;
       new_process->owned_mutex_id = NO_MUTEX_OWNED;

       list_pushback(partition->processes, new_process);

       _context_init(&new_process->tcb.context, (size_t)new_process->tcb.stack,new_process->tcb.stack_sz, (size_t)new_process->tcb.task);

       
       *RETURN_CODE = NO_ERROR;
}

void GET_ERROR_STATUS (
       /*out*/   ERROR_STATUS_TYPE        *ERROR_STATUS,
       /*out*/   RETURN_CODE_TYPE         *RETURN_CODE );

void RAISE_APPLICATION_ERROR (
       /*in */   ERROR_CODE_TYPE          ERROR_CODE,
       /*in */   MESSAGE_ADDR_TYPE        MESSAGE_ADDR,
       /*in */   ERROR_MESSAGE_SIZE_TYPE  LENGTH,
       /*out*/   RETURN_CODE_TYPE         *RETURN_CODE );

void CONFIGURE_ERROR_HANDLER (
       /*in */   ERROR_HANDLER_CONCURRENCY_CONTROL_TYPE  CONCURRENCY_CONTROL,
       /*in */   PROCESSOR_CORE_ID_TYPE                  PROCESSOR_CORE_ID,
       /*out*/   RETURN_CODE_TYPE                        *RETURN_CODE );
