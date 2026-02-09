#include <ucx.h>


void module_scheduler_init(const char* name, uint32_t major_frame_tick, const window_partition_type* windows_partition, uint32_t nbr_windows){
    
    struct mscb_s* module_scheduler;
    
    module_scheduler = ucx_malloc(sizeof(struct mscb_s));
    module_scheduler->name = name;
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

    // il vaut 1 car la tache est lancee via main durant l'init
    static uint32_t relative_tick = 1;
    
    int32_t* windows_idx = &module_scheduler->windows_idx;
    uint32_t partition_duration_tick = module_scheduler->windows_partition[*windows_idx].duration_tick;
    uint32_t partition_start_tick = module_scheduler->windows_partition[*windows_idx].start_tick;
    uint32_t partition_end_tick = partition_start_tick + partition_duration_tick;
    PARTITION_ID_TYPE partition_id = module_scheduler->windows_partition[*windows_idx].id;

    printf("scheduler perso, relative tick %d, partition end tick %d, major frame tick %d\n", relative_tick, partition_end_tick, module_scheduler->major_frame_tick);

    if(relative_tick >= module_scheduler->major_frame_tick){
        relative_tick = 1;
        (*windows_idx) = 0;
        partition_id = module_scheduler->windows_partition[*windows_idx].id;
        activate_partition(partition_id);
        return partition_id;
    }

    if(relative_tick >= partition_end_tick){
        if(*windows_idx == module_scheduler->nbr_windows - 1){
            return -1;
        }
        (*windows_idx)++;
        partition_id = module_scheduler->windows_partition[*windows_idx].id;
        activate_partition(partition_id);
    }
    
    relative_tick++;
    return partition_id;

}