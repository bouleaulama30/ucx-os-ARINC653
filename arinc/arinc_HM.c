#include "ucx.h"

void REPORT_APPLICATION_MESSAGE (
       /*in */   MESSAGE_ADDR_TYPE        MESSAGE_ADDR,
       /*in */   MESSAGE_SIZE_TYPE        LENGTH,
       /*out*/   RETURN_CODE_TYPE         *RETURN_CODE ){
       struct pcb_s *partition = get_current_partition();

       if (LENGTH <= 0){
            *RETURN_CODE = INVALID_PARAM;
            return;
       }

       hm_log_event(partition->status->IDENTIFIER, (char *)MESSAGE_ADDR);
       *RETURN_CODE = NO_ERROR;
}

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
       status->PROCESS_STATE = DORMANT;
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

       // Set the error process list to empty;
       partition->error_handler_process = new_process;
       partition->error_list_cb->read_index = 0;
       partition->error_list_cb->write_index = 0;
       partition->error_list_cb->nb_errors = 0;
       
       *RETURN_CODE = NO_ERROR;
}

void GET_ERROR_STATUS (
       /*out*/   ERROR_STATUS_TYPE        *ERROR_STATUS,
       /*out*/   RETURN_CODE_TYPE         *RETURN_CODE ){
       struct pcb_s *partition = get_current_partition();
       struct process_s *current_process = partition->process_current->data;
       if (current_process != partition->error_handler_process){
            *RETURN_CODE = INVALID_MODE;
            return;
       }
       if (partition->error_list_cb->nb_errors == 0){
            *RETURN_CODE = NO_ACTION;
            return;
       }
       struct error_list_s *error_list_cb = partition->error_list_cb;
       hm_read_error(error_list_cb, ERROR_STATUS);
   
       if(error_list_cb->error_processes_waiting_queue->length > 0){
              struct node_s *error_waiting_process_node = error_list_cb->error_processes_waiting_queue->head->next;
              struct process_s *error_waiting_process = error_waiting_process_node->data;
              list_remove(error_list_cb->error_processes_waiting_queue, error_waiting_process_node);
              error_waiting_process->processus_status->PROCESS_STATE = READY;     

              // ajout de l'erreur dans du process qui etait en attente dans la liste des erreurs
              ERROR_STATUS_TYPE error = error_waiting_process->pending_error;
              hm_write_error(error_list_cb, &error);
       }
       *RETURN_CODE = NO_ERROR;
}

void RAISE_APPLICATION_ERROR (
       /*in */   ERROR_CODE_TYPE          ERROR_CODE,
       /*in */   MESSAGE_ADDR_TYPE        MESSAGE_ADDR,
       /*in */   ERROR_MESSAGE_SIZE_TYPE  LENGTH,
       /*out*/   RETURN_CODE_TYPE         *RETURN_CODE ){
       struct pcb_s *partition = get_current_partition();
       struct process_s *current_process = partition->process_current->data;
       if (LENGTH < 0 || LENGTH > MAX_ERROR_MESSAGE_SIZE){
            *RETURN_CODE = INVALID_PARAM;
            return;
       }
       if(ERROR_CODE != APPLICATION_ERROR){
            *RETURN_CODE = INVALID_PARAM;
            return;
       }

       if (current_process == partition->error_handler_process || partition->error_handler_process == NULL){
              printf("Passage du message: %s et de l'erreur: %d au niveau superieur", MESSAGE_ADDR, ERROR_CODE);
       }
       else {
              ERROR_STATUS_TYPE error_status;
              error_status.ERROR_CODE = ERROR_CODE;
              error_status.FAILED_PROCESS_ID = current_process->process_id;
              error_status.LENGTH = LENGTH;
              memcpy(error_status.MESSAGE, MESSAGE_ADDR, LENGTH);
              if(partition->error_list_cb->nb_errors == partition->error_list_cb->max_errors){
                     current_process->processus_status->PROCESS_STATE = WAITING;
                     current_process->pending_error = error_status;
                     list_push(partition->error_list_cb->error_processes_waiting_queue, current_process);
                     yield_to_partition(partition, current_process);
              }
              hm_write_error(partition->error_list_cb, &error_status);
              struct process_s *error_process = partition->error_handler_process;
              if (error_process && error_process->processus_status->PROCESS_STATE == DORMANT || error_process->processus_status->PROCESS_STATE == WAITING){
                     error_process->processus_status->PROCESS_STATE = READY;
                     _context_init(&error_process->tcb.context, (size_t)error_process->tcb.stack,error_process->tcb.stack_sz, (size_t)error_process->tcb.task);
                     yield_to_partition(partition, current_process);
              }
       }
       *RETURN_CODE = NO_ERROR;
}


// TODO when multicore
void CONFIGURE_ERROR_HANDLER (
       /*in */   ERROR_HANDLER_CONCURRENCY_CONTROL_TYPE  CONCURRENCY_CONTROL,
       /*in */   PROCESSOR_CORE_ID_TYPE                  PROCESSOR_CORE_ID,
       /*out*/   RETURN_CODE_TYPE                        *RETURN_CODE );
