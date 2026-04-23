#ifndef ARINC_PARTITIONS_CONF_H
#define ARINC_PARTITIONS_CONF_H

#include "arinc/arinc_partition.h"
#include "arinc/module_scheduler.h"
#include "kernel/interpartition_communication.h"


#define SAMPLING_PORT_MAX_MESSAGE_SIZE 512

#define BLACKBOARD_MAX_MESSAGE_SIZE 512

#define BUFFER_MAX_MESSAGE_SIZE 64
#define BUFFER_MAX_NB_MESSAGE 3

extern void p1_process1(void);
extern void p1_process2(void);
extern void p1_process3(void);
extern void p2_process1(void);
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

static struct sampling_port_s p1_sampling_ports[MAX_NUMBER_OF_SAMPLING_PORTS];
static struct sampling_port_s p2_sampling_ports[MAX_NUMBER_OF_SAMPLING_PORTS];

static struct blackboard_s p1_blackboards[MAX_NUMBER_OF_BLACKBOARDS];
static uint8_t p1_blackboards_data[MAX_NUMBER_OF_BLACKBOARDS * BLACKBOARD_MAX_MESSAGE_SIZE]; // 512 bytes par blackboard
static uint32_t p1_blackboards_size_data[MAX_NUMBER_OF_BLACKBOARDS];

static struct buffer_s p1_buffers[MAX_NUMBER_OF_BUFFERS];
static uint8_t p1_buffers_data[MAX_NUMBER_OF_BUFFERS * BUFFER_MAX_MESSAGE_SIZE * BUFFER_MAX_NB_MESSAGE]; // 512 bytes par buffer
static uint32_t p1_buffers_size_data[MAX_NUMBER_OF_BUFFERS * BUFFER_MAX_NB_MESSAGE]; // 512 bytes par buffer

static struct semaphore_s p1_semaphores[MAX_NUMBER_OF_SEMAPHORES];
static volatile int32_t p1_semaphores_counter[MAX_NUMBER_OF_SEMAPHORES];

static struct event_s p1_events[MAX_NUMBER_OF_EVENTS];

static struct mutex_s p1_mutexes[MAX_NUMBER_OF_MUTEXES];
static struct mutex_s p2_mutexes[MAX_NUMBER_OF_MUTEXES];

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

    struct sampling_port_s *sampling_ports;
    APEX_INTEGER max_sampling_ports;
    APEX_INTEGER sampling_port_count;
    APEX_INTEGER max_sampling_port_data_size;


    struct blackboard_s *blackboards;
    APEX_INTEGER max_blackboards;
    APEX_INTEGER blackboard_count;
    APEX_INTEGER max_blackboard_data_size;
    uint8_t *blackboards_data;
    uint32_t *blackboards_size_data;
    
    struct buffer_s *buffers;
    APEX_INTEGER max_buffers;
    APEX_INTEGER buffer_count;
    APEX_INTEGER max_buffer_data_size;
    uint8_t *buffers_data;
    uint32_t *buffers_size_data;

    struct semaphore_s *semaphores;
    APEX_INTEGER max_semaphores;
    APEX_INTEGER semaphore_count;
    volatile int32_t *semaphores_counter;

    struct event_s *events;
    APEX_INTEGER max_events;
    APEX_INTEGER event_count;

    struct mutex_s *mutexes;
    APEX_INTEGER max_mutexes;
    APEX_INTEGER mutex_count;
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

    .sampling_ports = p1_sampling_ports,
    .max_sampling_ports = MAX_NUMBER_OF_SAMPLING_PORTS,
    .sampling_port_count = 0,
    .max_sampling_port_data_size = SAMPLING_PORT_MAX_MESSAGE_SIZE,

    .blackboards = p1_blackboards,
    .max_blackboards = MAX_NUMBER_OF_BLACKBOARDS,
    .blackboard_count = 0,
    .max_blackboard_data_size = BLACKBOARD_MAX_MESSAGE_SIZE, // 512 bytes par blackboard
    .blackboards_data = p1_blackboards_data,
    .blackboards_size_data = p1_blackboards_size_data,

    .buffers = p1_buffers,
    .max_buffers = MAX_NUMBER_OF_BUFFERS,
    .buffer_count = 0,
    .max_buffer_data_size = BUFFER_MAX_MESSAGE_SIZE * BUFFER_MAX_NB_MESSAGE, // 512 bytes par buffer
    .buffers_data = p1_buffers_data,
    .buffers_size_data = p1_buffers_size_data,

    .semaphores = p1_semaphores,
    .max_semaphores = MAX_NUMBER_OF_SEMAPHORES,
    .semaphore_count = 0,
    .semaphores_counter = p1_semaphores_counter,

    .events = p1_events,
    .max_events = MAX_NUMBER_OF_EVENTS,
    .event_count = 0,

    .mutexes = p1_mutexes,
    .max_mutexes = MAX_NUMBER_OF_MUTEXES,
    .mutex_count = 0,
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
    
    .sampling_ports = p2_sampling_ports,
    .max_sampling_ports = MAX_NUMBER_OF_SAMPLING_PORTS,
    .sampling_port_count = 0,
    .max_sampling_port_data_size = SAMPLING_PORT_MAX_MESSAGE_SIZE,
    

    .blackboards = NULL,
    .max_blackboards = 0,
    .blackboard_count = 0,
    .max_blackboard_data_size = 0,
    .blackboards_data = NULL,
    .blackboards_size_data = NULL,

    .buffers = NULL,
    .max_buffers = 0,
    .buffer_count = 0,
    .max_buffer_data_size = 0,
    .buffers_data = NULL,
    .buffers_size_data = NULL,
    
    .semaphores = NULL,
    .max_semaphores = 0,
    .semaphore_count = 0,
    .semaphores_counter = NULL,

    .events = NULL,
    .max_events = 0,
    .event_count = 0,

    .mutexes = p2_mutexes,
    .max_mutexes = MAX_NUMBER_OF_MUTEXES,
    .mutex_count = 0,
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


// P1 process 1 configuration
static const PROCESS_ATTRIBUTE_TYPE P1_PROCESS_1_CONFIG = {
    .PERIOD = INFINITE_TIME_VALUE,              // 20ms in nanoseconds
    .TIME_CAPACITY = INFINITE_TIME_VALUE,       // 10ms in nanoseconds
    .ENTRY_POINT = p1_process1,             // To be set by partition initialization
    .STACK_SIZE = 4096,              // 4KB stack
    .BASE_PRIORITY = 1,            // Medium priority (1-239)
    .DEADLINE = SOFT,                // Soft deadline
    .NAME = "Process 1"
};

// P1 process 2 configuration
static const PROCESS_ATTRIBUTE_TYPE P1_PROCESS_2_CONFIG = {
    .PERIOD = INFINITE_TIME_VALUE,              // 20ms in nanoseconds
    .TIME_CAPACITY = INFINITE_TIME_VALUE,       // 10ms in nanoseconds
    .ENTRY_POINT = p1_process2,             // To be set by partition initialization
    .STACK_SIZE = 4096,              // 4KB stack
    .BASE_PRIORITY = 1,            // Medium priority (1-239)
    .DEADLINE = SOFT,                // Soft deadline
    .NAME = "Process 2"
};

// P1 process 3 configuration
static const PROCESS_ATTRIBUTE_TYPE P1_PROCESS_3_CONFIG = {
    .PERIOD = INFINITE_TIME_VALUE,              // 20ms in nanoseconds
    .TIME_CAPACITY = INFINITE_TIME_VALUE,       // 10ms in nanoseconds
    .ENTRY_POINT = p1_process3,             // To be set by partition initialization
    .STACK_SIZE = 4096,              // 4KB stack
    .BASE_PRIORITY = 3,            // Medium priority (1-239)
    .DEADLINE = SOFT,                // Soft deadline
    .NAME = "Process 3"
};

// P2 process 1 configuration
static const PROCESS_ATTRIBUTE_TYPE P2_PROCESS_1_CONFIG = {
    .PERIOD = INFINITE_TIME_VALUE,              // 20ms in nanoseconds
    .TIME_CAPACITY = 10,       // 10ms in nanoseconds
    .ENTRY_POINT = p2_process1,             // To be set by partition initialization
    .STACK_SIZE = 4096,              // 4KB stack
    .BASE_PRIORITY = 3,            // Medium priority (1-239)
    .DEADLINE = SOFT,                // Soft deadline
    .NAME = "Process 1"
};


static uint8_t buffer_temperature[64];

static struct krnl_sampling_channel channel_temperature = {
    .name = "channelTemperature",
    .buffer = buffer_temperature,
    .max_message_size = 64,
    .current_message_size = 0,
    .last_update_time = 0,
};

// L'allocation de la RAM du noyau
static uint8_t q_channel_data[10 * 32]; 
static uint32_t q_channel_sizes[10]; // Tableau pour stocker les 10 tailles

static struct krnl_queuing_channel_s channel_cmds = {
    .buffer_data = q_channel_data,
    .buffer_sizes = q_channel_sizes,
    .max_message_size = 64,
    .max_nb_messages = 10,
    .current_nb_messages = 0,
    .read_index = 0,
    .write_index = 0,
    .source_partition_id = 2,
    .dest_partition_id = 1
};

struct port_mapping_s {
    PARTITION_ID_TYPE partition_id;
    SAMPLING_PORT_NAME_TYPE port_name;
    PORT_DIRECTION_TYPE port_direction;
    MESSAGE_SIZE_TYPE messageSizeBytes;
    MESSAGE_RANGE_TYPE max_nb_message;
    QUEUING_DISCIPLINE_TYPE QUEUING_DISCIPLINE;
    SYSTEM_TIME_TYPE refreshPeriodMs;
    struct krnl_sampling_channel *sampling_channel;
    struct krnl_queuing_channel_s *queuing_channel;
};

static const struct port_mapping_s system_port_table[] = {
    {.partition_id = 1, .port_name = "P1_OUT_TEMP", .port_direction = SOURCE, .messageSizeBytes = 64, .refreshPeriodMs = 200, .sampling_channel = &channel_temperature},
    {.partition_id = 2, .port_name = "P2_IN_TEMP", .port_direction = DESTINATION, .messageSizeBytes = 64, .refreshPeriodMs = 200, .sampling_channel = &channel_temperature},
    {.partition_id = 1, .port_name = "P1_IN_CMDS", .port_direction = DESTINATION, .messageSizeBytes = 32, .max_nb_message = 10, .QUEUING_DISCIPLINE = PRIORITY, .queuing_channel = &channel_cmds},
    {.partition_id = 2, .port_name = "P2_OUT_CMDS", .port_direction = SOURCE, .messageSizeBytes = 32, .max_nb_message = 10, .QUEUING_DISCIPLINE = PRIORITY, .queuing_channel = &channel_cmds},
};
extern const int routing_table_size;


struct blackboardConfig {
    BLACKBOARD_NAME_TYPE blackboard_name;
    MESSAGE_SIZE_TYPE max_message_size;
};

static const struct blackboardConfig blackboard_configs[] = {
    {.blackboard_name = "BB1", .max_message_size = BLACKBOARD_MAX_MESSAGE_SIZE},
    {.blackboard_name = "BB2", .max_message_size = BLACKBOARD_MAX_MESSAGE_SIZE},
};


struct bufferConfig {
    BUFFER_NAME_TYPE buffer_name;
    MESSAGE_SIZE_TYPE max_message_size;
    MESSAGE_RANGE_TYPE max_nb_message;
    QUEUING_DISCIPLINE_TYPE queuing_discipline;
};

static const struct bufferConfig buffer_configs[] = {
    {.buffer_name = "Buffer1", .max_message_size = BUFFER_MAX_MESSAGE_SIZE, .max_nb_message = BUFFER_MAX_NB_MESSAGE, .queuing_discipline = FIFO},
    {.buffer_name = "Buffer2", .max_message_size = BUFFER_MAX_MESSAGE_SIZE, .max_nb_message = BUFFER_MAX_NB_MESSAGE, .queuing_discipline = FIFO},
};

struct semaphoreConfig {
    SEMAPHORE_NAME_TYPE semaphore_name;
    SEMAPHORE_VALUE_TYPE current_value;
    SEMAPHORE_VALUE_TYPE maximum_value;
    QUEUING_DISCIPLINE_TYPE queuing_discipline;
};
    
static const struct semaphoreConfig semaphore_configs[] = {
    {.semaphore_name = "Semaphore1", .current_value = 0, .maximum_value = 1, .queuing_discipline = FIFO},
};

struct eventConfig {
    EVENT_NAME_TYPE event_name;
    EVENT_ID_TYPE event_id;
    PARTITION_ID_TYPE partition_id;
};

static const struct eventConfig event_configs[] = {
    {.event_name = "Event1", .event_id = 1},
};

struct mutexConfig {
    MUTEX_NAME_TYPE mutex_name;
    PRIORITY_TYPE mutex_priority;
    PARTITION_ID_TYPE partition_id;
    QUEUING_DISCIPLINE_TYPE queuing_discipline;
};

static const struct mutexConfig mutex_configs[] = {
    {.mutex_name = "Mutex1", .mutex_priority = 2, .queuing_discipline = FIFO},
};

#endif 