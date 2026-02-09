
/*----------------------------------------------------------------*/
/*                                                                */
/* PARTITION constant and type definitions and management services*/
/*                                                                */
/*----------------------------------------------------------------*/

#include "arinc_apex_types.h"
#include "arinc_process.h"

#ifndef APEX_PARTITION
#define APEX_PARTITION

#define  MAX_NUMBER_OF_PARTITIONS  SYSTEM_LIMIT_NUMBER_OF_PARTITIONS

#define IDLE_PARTITION_ID 0

typedef
   enum {
        IDLE       = 0,
        COLD_START = 1,
        WARM_START = 2,
        NORMAL     = 3
   } OPERATING_MODE_TYPE;

/* PARTITION_IDs originate from the configuration data */
typedef APEX_INTEGER     PARTITION_ID_TYPE; 
typedef APEX_UNSIGNED    NUM_CORES_TYPE; 

// definition of new type
typedef NAME_TYPE PARTITION_NAME_TYPE;
typedef NAME_TYPE REGION_NAME_TYPE;
typedef NAME_TYPE ACCESS_TYPE;

enum {
      CODE = 0,
      DATA = 1,
    };

typedef 
   struct {
    REGION_NAME_TYPE   region_name;
    void*      base;
    APEX_UNSIGNED      size;
    ACCESS_TYPE        access;
   } MEMORY_REGION_TYPE;

typedef 
   struct {
      // une region code et une région data
      MEMORY_REGION_TYPE memory[2];
   } MEMORY_REQUIREMENTS_TYPE;

typedef 
    enum {
        true = 0,
        false = 1
    } SYSTEM_PARTITION_TYPE;


typedef
   enum {
        NORMAL_START         = 0,
        PARTITION_RESTART    = 1,
        HM_MODULE_RESTART    = 2,
        HM_PARTITION_RESTART = 3
   } START_CONDITION_TYPE;

typedef
   struct {
      SYSTEM_TIME_TYPE       PERIOD;
      SYSTEM_TIME_TYPE       DURATION;
      PARTITION_ID_TYPE      IDENTIFIER;
      LOCK_LEVEL_TYPE        LOCK_LEVEL;
      OPERATING_MODE_TYPE    OPERATING_MODE;
      START_CONDITION_TYPE   START_CONDITION;
      NUM_CORES_TYPE         NUM_ASSIGNED_CORES;
   } PARTITION_STATUS_TYPE;


struct pcb_s {
   struct tcb_s tcb;
   PARTITION_STATUS_TYPE *status;
   PARTITION_NAME_TYPE name;
   MEMORY_REQUIREMENTS_TYPE *memory_requirements;
   // interpartition_communication_type communication_ports;
   // Partion HM Table
   SYSTEM_ADDRESS_TYPE entry_point;
   SYSTEM_PARTITION_TYPE is_system_partition;

};


extern void GET_PARTITION_STATUS (
       /*out*/ PARTITION_STATUS_TYPE      *PARTITION_STATUS,
       /*out*/ RETURN_CODE_TYPE           *RETURN_CODE );

extern void SET_PARTITION_MODE (
       /*in */ OPERATING_MODE_TYPE        OPERATING_MODE,
       /*out*/ RETURN_CODE_TYPE           *RETURN_CODE );

extern void GET_MY_PARTITION_ID (
       /*out*/ PARTITION_ID_TYPE          *PARTITION_ID,
       /*out*/ RETURN_CODE_TYPE           *RETURN_CODE );

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
                        SYSTEM_PARTITION_TYPE is_system_partition);

extern int32_t activate_partition(PARTITION_ID_TYPE IDENTIFIER);

#endif