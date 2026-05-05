#include "../arinc/arinc_HM.h"


struct hm_cb_s {
    char *log_buffer;
    int write_index;
    int read_index;
    int nb_log_entries;
    int max_log_entries;
    int max_log_entry_size;
};

void hm_init(struct hm_cb_s *hm_cb, char *log_buffer);
struct hm_cb_s get_hm_cb();

void hm_log_event(PARTITION_ID_TYPE partition_id, char *msg);

void hm_write_error(struct error_list_s *error_list_cb, ERROR_STATUS_TYPE *error_status);

void hm_read_error(struct error_list_s *error_list_cb, ERROR_STATUS_TYPE *error_status);
