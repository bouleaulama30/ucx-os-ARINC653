#include <ucx.h>


void module_scheduler_init(const char* name, uint32_t major_frame_tick, const window_partition_type* windows_partition, uint32_t nbr_windows){
    
    struct mscb_s* ms;
    
    ms = ucx_malloc(sizeof(struct mscb_s));
    if (!ms){
        krnl_panic(ERR_MSCB_ALLOC);
    }

    ms->name = name;
    ms->major_frame_tick = major_frame_tick;
    ms->windows_partition = windows_partition;
    ms->nbr_windows = nbr_windows;
    ms->windows_idx = 0;
    ms->idle_current_partition = false;

    
    // on associe le module scheduler et le scheduler au kernel
#ifndef MULTICORE
    kcb->module_scheduler = ms;
    kcb->rt_sched = partition_scheduler;
#else
    kcb[_cpu_id()]->module_scheduler = ms;
    kcb[_cpu_id()]->rt_sched = partition_scheduler;
#endif
}


void arinc_start_scheduling(void) {
#ifndef MULTICORE
    struct mscb_s* ms = kcb->module_scheduler;
#else
    struct mscb_s* ms = kcb[_cpu_id()]->module_scheduler;
#endif

    if(ms == NULL){
        krnl_panic(ERR_SCHED_CONFIG); 
    }

    if (ms->windows_partition == NULL || ms->nbr_windows == 0) {
        krnl_panic(ERR_SCHED_CONFIG);
    }

    int32_t first_window_idx = 0;
    PARTITION_ID_TYPE first_id = ms->windows_partition[first_window_idx].id;

    activate_partition(first_id);

    ms->windows_idx = 0;
    

#ifndef MULTICORE
    kcb->ticks = 0;
#else
    kcb[_cpu_id()]->ticks = 0;
#endif

    printf("[SCHED] Starting ARINC schedule with Partition %d\n", first_id);

#ifndef MULTICORE
    struct pcb_s* first_pcb = (struct pcb_s*)kcb->task_current->data;
#else
    struct pcb_s* first_pcb = (struct pcb_s*)kcb[_cpu_id()]->task_current->data;
#endif

    if (first_pcb == NULL){
        krnl_panic(ERR_FAIL);
    }

    _dispatch_init(first_pcb->tcb.context);
}

void signal_idle_current_partition(void){
#ifndef MULTICORE
    struct mscb_s* ms = kcb->module_scheduler;
#else
    struct mscb_s* ms = kcb[_cpu_id()]->module_scheduler;
#endif
    if(ms == NULL){
        krnl_panic(ERR_SCHED_CONFIG); 
    }
    ms->idle_current_partition = true;
}


int32_t partition_scheduler(void){
    
#ifndef MULTICORE
    struct mscb_s* ms = kcb->module_scheduler;
#else
    struct mscb_s* ms = kcb[_cpu_id()]->module_scheduler;
#endif
    if (ms == NULL) {
        krnl_panic((uint32_t)ERR_FAIL);
    }

    if (ms->windows_partition == NULL || ms->nbr_windows == 0) {
        krnl_panic(ERR_SCHED_CONFIG);
    }

    // il vaut 1 car la tache est lancee via main durant l'init
    static uint32_t relative_tick = 1;
    
    int32_t* windows_idx = &ms->windows_idx;
    uint32_t partition_duration_tick = ms->windows_partition[*windows_idx].duration_tick;
    uint32_t partition_start_tick = ms->windows_partition[*windows_idx].start_tick;
    uint32_t partition_end_tick = partition_start_tick + partition_duration_tick;
    PARTITION_ID_TYPE partition_id = ms->windows_partition[*windows_idx].id;

    printf("scheduler perso, relative tick %d, partition end tick %d, major frame tick %d\n", relative_tick, partition_end_tick, ms->major_frame_tick);

    if(ms->idle_current_partition){
        partition_id = activate_partition(IDLE_PARTITION_ID);
        ms->idle_current_partition = false;
    }

    if(relative_tick >= ms->major_frame_tick){
        relative_tick = 1;
        (*windows_idx) = 0;
        partition_id = ms->windows_partition[*windows_idx].id;
        return activate_partition(partition_id);
    }

    if(relative_tick >= partition_end_tick){
        if(*windows_idx == ms->nbr_windows - 1){
            relative_tick++;
            return activate_partition(IDLE_PARTITION_ID);
        }
        (*windows_idx)++;
        partition_id = ms->windows_partition[*windows_idx].id;
        activate_partition(partition_id);
    }
    
    relative_tick++;
    return partition_id;
}