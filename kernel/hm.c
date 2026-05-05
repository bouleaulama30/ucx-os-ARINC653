#include "ucx.h"

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