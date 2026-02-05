#include "arinc_apex_types.h"

#ifndef MODULE_SCHEDULER
#define MODULE_SCHEDULER


typedef
    struct 
    {
        PARTITION_NAME_TYPE name;
        PARTITION_ID_TYPE id;
        uint32_t start_tick;
        uint32_t duration_tick;
    } window_partition_type;
    

struct mscb_s {
    char* name;
    uint32_t major_frame_tick;
    window_partition_type* windows_partition;
    uint32_t nbr_windows;
    int32_t windows_idx;
};

extern void  module_scheduler_init(PARTITION_NAME_TYPE name, PARTITION_ID_TYPE id, uint32_t major_frame_tick, window_partition_type* windows_partition, uint32_t nbr_windows);
extern int32_t partition_scheduler(void);

#endif