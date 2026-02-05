#include <ucx.h>



void module_scheduler_init(char* name, uint32_t major_frame_tick, window_partition_type* windows_partition, uint32_t nbr_windows){
    struct mscb_s* module_scheduler;
    uint32_t nbr_windows_partition;

    module_scheduler = ucx_malloc(sizeof(struct mscb_s));
    strcpy(module_scheduler->name, name);
    module_scheduler->major_frame_tick = major_frame_tick;
    module_scheduler->windows_partition = windows_partition;
    module_scheduler->nbr_windows = nbr_windows;
    module_scheduler->windows_idx = 0;
    
    // on associe le module scheduler et le scheduler au kernel
#ifndef MULTICORE
    kcb->module_scheduler = module_scheduler;
    kcb->rt_sched = partition_scheduler;
#else
    kcb[_cpu_id()]->module_scheduler = module_scheduler;
    kcb[_cpu_id()]->rt_sched = partition_scheduler;
#endif
}

int32_t partition_scheduler(void){

#ifndef MULTICORE
    struct mscb_s* module_scheduler = kcb->module_scheduler;
#else
    struct mscb_s* module_scheduler = kcb[_cpu_id()]->module_scheduler;
#endif
    static uint32_t relative_tick = 0;
    int32_t windows_idx = module_scheduler->windows_idx;

    // définit statiquement
    uint32_t partition_start_tick = module_scheduler->windows_partition[windows_idx].start_tick;
    uint32_t partition_duration_tick = module_scheduler->windows_partition[windows_idx].duration_tick;
    uint32_t partition_end_tick = partition_start_tick + partition_duration_tick;
    PARTITION_ID_TYPE id = 1;
    
    if(relative_tick == module_scheduler->major_frame_tick){
        relative_tick = 0;
        partition_start_tick = 2;
        partition_duration_tick = 2;
        activate_partition(id);
    }

    if(relative_tick > partition_end_tick){
        relative_tick++;
        activate_partition(id);
        partition_start_tick = 2;
        partition_duration_tick = 2;
        return id;
    }

    relative_tick++;
    return -1;
}