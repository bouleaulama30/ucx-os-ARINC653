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
        BOOLEAN_TYPE is_periodic_processes_start;
    } window_partition_type;


struct mscb_s {
    const char* name;
    uint32_t major_frame_tick;
    const window_partition_type* windows_partition;
    uint32_t nbr_windows;
    int32_t windows_idx;
    BOOLEAN_TYPE idle_current_partition;
    uint32_t major_frame_count;
};      

extern void  module_scheduler_init(const char* name, uint32_t major_frame_tick, const window_partition_type* windows_partition, uint32_t nbr_windows);
extern void arinc_start_scheduling(void);
extern void signal_idle_current_partition(void);
extern int32_t partition_scheduler(void);

#endif