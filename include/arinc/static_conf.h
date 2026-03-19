#ifndef ARINC_PARTITIONS_CONF_H
#define ARINC_PARTITIONS_CONF_H

#include "arinc/arinc_partition.h"
#include "arinc/module_scheduler.h"

extern void process_test0(void);
extern void process_test1(void);
extern void process_test2(void);
extern void process_test3(void);
extern void test_spatial_violation_p1(void);
extern void test_spatial_violation_p2(void);
extern void test_round_robin_A(void);
extern void test_round_robin_B(void);
extern void test_periodic_accuracy(void);
extern void test_states_slave(void);
extern void test_states_master(void);
extern void test_prio_A(void);
extern void test_prio_B(void);



// Déclaration des symboles du linker script du kernel
extern uint8_t _kernel_end[];

// Déclaration des symboles du linker script de la p1
extern uint8_t _p1_code_start[];
extern uint8_t _p1_code_end[];

extern uint8_t _p1_data_start[];
extern uint8_t _p1_data_end[];

// Déclaration des symboles du linker script de la p2
extern uint8_t _p2_code_start[];
extern uint8_t _p2_code_end[];

extern uint8_t _p2_data_start[];
extern uint8_t _p2_data_end[];


// Hardcoded partition configuration
struct PartitionConfig {
    SYSTEM_TIME_TYPE period;
    SYSTEM_TIME_TYPE duration;
    PARTITION_ID_TYPE identifier;
    NUM_CORES_TYPE num_assigned_cores;
    PARTITION_NAME_TYPE name;
    REGION_NAME_TYPE region_name_code_mem;
    SYSTEM_ADDRESS_TYPE base_code_mem;
    APEX_UNSIGNED size_code_mem;
    ACCESS_TYPE access_code_mem;
    REGION_NAME_TYPE region_name_data_mem;
    SYSTEM_ADDRESS_TYPE base_data_mem;
    APEX_UNSIGNED size_data_mem;
    ACCESS_TYPE access_data_mem;
    SYSTEM_ADDRESS_TYPE entry_point;
    BOOLEAN_TYPE is_system_partition;
};

// Default hardcoded partition configuration et voir le ldscript pour la conf mémoire
static const struct PartitionConfig DEFAULT_PARTITION_CONFIG = {
    .period = 50,                    // 1 second in nanoseconds
    .duration = 20,                   // 500ms
    .identifier = 1,
    .num_assigned_cores = 1,
    .name = "DefaultPartition",
    .region_name_code_mem = "p1_code",
    .access_code_mem = "RX",
    .region_name_data_mem = "p1_data",
    .access_data_mem = "RW",
    .is_system_partition = (BOOLEAN_TYPE)false,
};

static const struct PartitionConfig P2_CONFIG = {
    .period = 50,                    // 1 second in nanoseconds
    .duration = 20,                   // 500ms
    .identifier = 2,
    .num_assigned_cores = 1,
    .name = "P2",
    .region_name_code_mem = "p2_code",
    .access_code_mem = "RX",
    .region_name_data_mem = "p2_data",
    .access_data_mem = "RW",
    .is_system_partition = (BOOLEAN_TYPE)true,
};

// Static module scheduler configuration
static char const DEFAULT_MODULE_NAME[] = "DefaultModule";
#define DEFAULT_MAJOR_FRAME_TICK MS_TO_TICKS(100)

static const window_partition_type DEFAULT_WINDOWS[] = {
    {
        .name = "DefaultPartition",
        .id = 1,
        .start_tick = MS_TO_TICKS(0),
        .duration_tick = MS_TO_TICKS(40),
        .is_periodic_processes_start = (BOOLEAN_TYPE)true,
        // .is_periodic_processes_start = (BOOLEAN_TYPE)false,
    },
    {
        .name = "P2",
        .id = 2,
        .start_tick = MS_TO_TICKS(40),
        .duration_tick = MS_TO_TICKS(20),
        .is_periodic_processes_start = (BOOLEAN_TYPE)true,
        },
    {
        .name = "DefaultPartition",
        .id = 1,
        .start_tick = MS_TO_TICKS(60),
        .duration_tick = MS_TO_TICKS(20),
        .is_periodic_processes_start = (BOOLEAN_TYPE)false,
        // .is_periodic_processes_start = (BOOLEAN_TYPE)true,
    },
    {
            .name = "P2",
            .id = 2,
            .start_tick = MS_TO_TICKS(80),
            .duration_tick = MS_TO_TICKS(20),
            .is_periodic_processes_start = (BOOLEAN_TYPE)false,
        },
    };
    
static const uint32_t DEFAULT_WINDOWS_COUNT = sizeof(DEFAULT_WINDOWS) / sizeof(DEFAULT_WINDOWS[0]);


// Default process configuration
static const PROCESS_ATTRIBUTE_TYPE DEFAULT_PROCESS_CONFIG = {
    .PERIOD = INFINITE_TIME_VALUE,              // 20ms in nanoseconds
    .TIME_CAPACITY = 50,       // 10ms in nanoseconds
    .ENTRY_POINT = process_test0,             // To be set by partition initialization
    .STACK_SIZE = 4096,              // 4KB stack
    .BASE_PRIORITY = 2,            // Medium priority (1-239)
    .DEADLINE = SOFT,                // Soft deadline
    .NAME = "DefaultProcess"
};

// process 1 configuration
static const PROCESS_ATTRIBUTE_TYPE PROCESS_1_CONFIG = {
    .PERIOD = 100,              // 20ms in nanoseconds
    .TIME_CAPACITY = 50,       // 10ms in nanoseconds
    .ENTRY_POINT = process_test1,             // To be set by partition initialization
    .STACK_SIZE = 4096,              // 4KB stack
    .BASE_PRIORITY = 2,            // Medium priority (1-239)
    .DEADLINE = SOFT,                // Soft deadline
    .NAME = "Process 1"
};

// process 2 configuration
static const PROCESS_ATTRIBUTE_TYPE PROCESS_2_CONFIG = {
    .PERIOD = INFINITE_TIME_VALUE,              // 20ms in nanoseconds
    .TIME_CAPACITY = 10,       // 10ms in nanoseconds
    .ENTRY_POINT = process_test2,             // To be set by partition initialization
    .STACK_SIZE = 4096,              // 4KB stack
    .BASE_PRIORITY = 2,            // Medium priority (1-239)
    .DEADLINE = SOFT,                // Soft deadline
    .NAME = "Process 2"
};

// process 3 configuration
static const PROCESS_ATTRIBUTE_TYPE PROCESS_3_CONFIG = {
    .PERIOD = INFINITE_TIME_VALUE,              // 20ms in nanoseconds
    .TIME_CAPACITY = 10,       // 10ms in nanoseconds
    .ENTRY_POINT = process_test3,             // To be set by partition initialization
    .STACK_SIZE = 4096,              // 4KB stack
    .BASE_PRIORITY = 3,            // Medium priority (1-239)
    .DEADLINE = SOFT,                // Soft deadline
    .NAME = "Process 3"
};

#endif 