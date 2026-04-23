
#include "../arinc/arinc_partition.h"
#include "../arinc/arinc_apex_types.h"

#ifndef PARTITION
#define PARTITION
extern int32_t partition_init(SYSTEM_TIME_TYPE PERIOD, 
                        SYSTEM_TIME_TYPE DURATION,
                        PARTITION_ID_TYPE IDENTIFIER,
                        NUM_CORES_TYPE    NUM_ASSIGNED_CORES,
                        const PARTITION_NAME_TYPE name,
                        const REGION_NAME_TYPE   region_name_code_mem,
                        SYSTEM_ADDRESS_TYPE base_code_mem,
                        APEX_UNSIGNED      size_code_mem,
                        const ACCESS_TYPE        access_code_mem,
                        const REGION_NAME_TYPE   region_name_data_mem,
                        SYSTEM_ADDRESS_TYPE base_data_mem,
                        APEX_UNSIGNED      size_data_mem,
                        const ACCESS_TYPE        access_data_mem,
                        SYSTEM_ADDRESS_TYPE entry_point,
                        BOOLEAN_TYPE is_system_partition,
                        
                        struct sampling_port_s *sampling_ports,
                        APEX_INTEGER max_sampling_ports,
                        APEX_INTEGER sampling_port_count,
                        APEX_INTEGER max_sampling_port_data_size,

                        struct blackboard_s *blackboards,
                        APEX_INTEGER max_blackboards,
                        APEX_INTEGER blackboard_count,
                        APEX_INTEGER max_blackboard_data_size,
                        uint8_t *blackboards_data,
                        uint32_t *blackboards_size_data,

                        struct buffer_s *buffers,
                        APEX_INTEGER max_buffers,
                        APEX_INTEGER buffer_count,
                        APEX_INTEGER max_buffer_data_size,
                        uint8_t *buffers_data,
                        uint32_t *buffers_size_data,

                        struct semaphore_s *semaphores,
                        APEX_INTEGER max_semaphores,
                        APEX_INTEGER semaphore_count,
                        volatile int32_t *semaphores_counter,

                        struct event_s *events,
                        APEX_INTEGER max_events,
                        APEX_INTEGER event_count,

                        struct mutex_s *mutexes,
                        APEX_INTEGER max_mutexes,
                        APEX_INTEGER mutex_count
                        );

extern int32_t krnl_partition_switch(PARTITION_ID_TYPE IDENTIFIER);


static inline struct pcb_s* get_current_partition(){
#ifndef MULTICORE
    struct node_s *partition_node = kcb->partition_current;
#else
    struct node_s *partition_node = kcb[_cpu_id()]->partition_current;
#endif
    struct pcb_s *partition = partition_node->data;
    return partition;
}
#endif