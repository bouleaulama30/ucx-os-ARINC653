#ifndef ARINC_PARTITIONS_CONF_H
#define ARINC_PARTITIONS_CONF_H

#include "arinc/arinc_partition.h"


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
    SYSTEM_PARTITION_TYPE is_system_partition;
};

// Default hardcoded partition configuration et voir le ldscript pour la conf mémoire
static const struct PartitionConfig DEFAULT_PARTITION_CONFIG = {
    .period = 1000000,                    // 1 second in nanoseconds
    .duration = 500000,                   // 500ms
    .identifier = 1,
    .num_assigned_cores = 1,
    .name = "DefaultPartition",
    .region_name_code_mem = "p1_code",
    .access_code_mem = "RX",
    .region_name_data_mem = "p1_data",
    .access_data_mem = "RW",
    .is_system_partition = (SYSTEM_PARTITION_TYPE)false,
};

static const struct PartitionConfig P2_CONFIG = {
    .period = 1000000,                    // 1 second in nanoseconds
    .duration = 500000,                   // 500ms
    .identifier = 2,
    .num_assigned_cores = 1,
    .name = "P2",
    .region_name_code_mem = "p2_code",
    .access_code_mem = "RX",
    .region_name_data_mem = "p2_data",
    .access_data_mem = "RW",
    .is_system_partition = (SYSTEM_PARTITION_TYPE)false,
};

#endif // ARINC_PARTITIONS_CONF_H