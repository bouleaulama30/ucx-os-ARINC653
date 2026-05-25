#include "ucx.h"


void hm_init(struct hm_cb_s *hm_cb, char *log_buffer){
    hm_cb->log_buffer = log_buffer;
    hm_cb->write_index = 0;
    hm_cb->read_index = 0;
    hm_cb->nb_log_entries = 0;
    hm_cb->max_log_entries = MAX_LOG_ENTRIES;
    hm_cb->max_log_entry_size = MAX_LOG_ENTRY_SIZE;
#ifndef MULTICORE
    kcb->hm_cb = hm_cb;
#else
    kcb[_cpu_id()]->hm_cb = hm_cb;
#endif
}

struct hm_cb_s *get_hm_cb(){
#ifndef MULTICORE
    return kcb->hm_cb;
#else
    return kcb[_cpu_id()]->hm_cb;
#endif
}

void hm_log_event(PARTITION_ID_TYPE partition_id, char *msg){
    struct hm_cb_s *hm_cb = get_hm_cb();
    char *log_buffer;
    char *dest;
    int write_index;
    int max_log_entry_size;
    int max_entries;

    if (hm_cb == NULL || hm_cb->log_buffer == NULL) {
        return;
    }

    log_buffer = hm_cb->log_buffer;
    write_index = hm_cb->write_index;
    max_log_entry_size = hm_cb->max_log_entry_size;
    max_entries = hm_cb->max_log_entries;

    // Structure : [PARTITION_ID (4 bytes)] [ERROR_CODE (4 bytes)] [MSG (le reste)]
    dest = &log_buffer[write_index * max_log_entry_size];

    memcpy(dest, &partition_id, sizeof(PARTITION_ID_TYPE));
    dest += sizeof(PARTITION_ID_TYPE);


    int header_size = sizeof(PARTITION_ID_TYPE);
    int remaining_space = max_log_entry_size - header_size - 1;
    
    int i = 0;
    if (msg != NULL) {
        while (i < remaining_space && msg[i] != '\0') {
            dest[i] = msg[i];
            i++;
        }
    }
    dest[i] = '\0'; 

    // 4. Mise à jour de l'index d'écriture (Buffer Circulaire)
    hm_cb->write_index = (write_index + 1) % max_entries;
}

void hm_raise_error(ERROR_CODE_TYPE ERROR_CODE, MESSAGE_ADDR_TYPE MESSAGE_ADDR, ERROR_MESSAGE_SIZE_TYPE LENGTH, struct node_s *failed_process_node){
    struct pcb_s *partition = get_current_partition();
    struct process_s *process = failed_process_node->data;
    process->processus_status->PROCESS_STATE = FAULTED;

    if (LENGTH < 0 || LENGTH > MAX_ERROR_MESSAGE_SIZE){
        printf("Invalid error message length: %d\n", LENGTH);
    }

    ERROR_STATUS_TYPE error_status;
    error_status.ERROR_CODE = ERROR_CODE;
    error_status.FAILED_PROCESS_ID = process->process_id;
    error_status.LENGTH = LENGTH;
    memcpy(error_status.MESSAGE, MESSAGE_ADDR, LENGTH);

    if (process == partition->error_handler_process || partition->error_handler_process == NULL){
        printf("Passage du message: %s et de l'erreur: %d au niveau superieur\n", MESSAGE_ADDR, ERROR_CODE);
        hm_raise_partition_error(&error_status);
        longjmp(partition->partition_context, 1);
        return;
    }

    hm_write_error(partition->error_list_cb, &error_status);
    struct process_s *error_process = partition->error_handler_process;
    if (error_process && (error_process->processus_status->PROCESS_STATE == DORMANT || error_process->processus_status->PROCESS_STATE == WAITING)){
        error_process->processus_status->PROCESS_STATE = READY;
        _context_init(&error_process->tcb.context, (size_t)error_process->tcb.stack,error_process->tcb.stack_sz, (size_t)error_process->tcb.task);
    }
    longjmp(partition->partition_context, 1);
    return;
}

void hm_raise_partition_error(ERROR_STATUS_TYPE *error_status){
    struct pcb_s *partition = get_current_partition();
    OPERATING_MODE_TYPE operating_mode = partition->status->OPERATING_MODE;
    uint32_t action_index;
    // switch (error_status->ERROR_CODE)
    // {
    // case DEADLINE_MISSED:
    //     action_index = 0;
    //     break;
    // case APPLICATION_ERROR:
    //     action_index = 1;
    //     break;
    // case NUMERIC_ERROR:
    //     action_index = 2;
    //     break;
    // default:
    //     action_index = 3;
    //     break;
    // }

    ERROR_ACTION_TYPE action = partition->partition_hm_table[operating_mode][error_status->ERROR_CODE];
    switch (action)
    {
    case PROCESS_RESTART:
        printf("HM Partition Action: PROCESS_RESTART\n");
        // RETURN_CODE_TYPE return_code;
        // SET_PARTITION_MODE(COLD_START, &return_code);
        // struct node_s *process_node = is_process_id_existed(partition, error_status->FAILED_PROCESS_ID);
        // struct process_s *process = process_node->data;
        // process->processus_status->PROCESS_STATE = DORMANT;
        // printf("HM Partition stop process %d rc=%d\n", error_status->FAILED_PROCESS_ID, return_code);
        break;
    case PROCESS_REPLENISH:
        printf("HM Partition Action: PROCESS_REPLENISH\n");
        break;
    case PARTITION_STOP:
        printf("HM Partition Action: PARTITION_STOP\n");
        break;
    case IGNORE:
        printf("HM Partition Action: IGNORE\n");
        struct node_s *process_node = is_process_id_existed(partition, error_status->FAILED_PROCESS_ID);
        if (process_node) {
            struct process_s *process = process_node->data;
            process->processus_status->PROCESS_STATE = READY;
        }
        break;
    default:
        printf("HM Partition Action: Passage au niveau du module\n");
        hm_raise_module_error(error_status);
        break;
    }
    return;
}

void hm_raise_module_error(ERROR_STATUS_TYPE *error_status){
    uint32_t action_index;
    // switch (error_status->ERROR_CODE)
    // {
    // case DEADLINE_MISSED:
    //     action_index = 0;
    //     break;
    // case APPLICATION_ERROR:
    //     action_index = 1;
    //     break;
    // case NUMERIC_ERROR:
    //     action_index = 2;
    //     break;
    // default:
    //     action_index = 3;
    //     break;
    // }

    ERROR_ACTION_TYPE action = hm_table_module[0][error_status->ERROR_CODE];
    switch (action)
    {
    case PROCESS_RESTART:
        printf("HM Module Action: PROCESS_RESTART\n");
        break;
    case PROCESS_REPLENISH:
        printf("HM Module Action: PROCESS_REPLENISH\n");
        break;
    case PARTITION_STOP:
        printf("HM Module Action: PARTITION_STOP\n");
        break;
    default:
        printf("HM Module Action: Passage au niveau du module error code: %d action: %d\n", error_status->ERROR_CODE, action);
        break;
    }
    return;
}

void hm_write_error(struct error_list_s *error_list_cb, ERROR_STATUS_TYPE *error_status){
    error_list_cb->process_error_list[error_list_cb->write_index] = *error_status;
    error_list_cb->write_index = (error_list_cb->write_index + 1) % error_list_cb->max_errors;
    error_list_cb->nb_errors++;
}

void hm_read_error(struct error_list_s *error_list_cb, ERROR_STATUS_TYPE *error_status){
    *error_status = error_list_cb->process_error_list[error_list_cb->read_index];
    error_list_cb->read_index = (error_list_cb->read_index + 1) % error_list_cb->max_errors;
    error_list_cb->nb_errors--;
}



