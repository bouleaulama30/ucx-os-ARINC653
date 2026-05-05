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



