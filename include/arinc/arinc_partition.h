
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
typedef NAME_TYPE partition_name_type;
typedef NAME_TYPE region_name_type;

typedef 
   struct {
    //TODO -Q à revoir
    region_name_type   region_name;
    // APEX_UNSIGNED      base;
    // APEX_UNSIGNED      size;


   } memory_requirements_type;

typedef 
    enum {
        false = 0,
        true = 1
    } system_partition_type;

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
    PARTITION_STATUS_TYPE *status;
    partition_name_type name;
    memory_requirements_type memory_requirements;
    // interpartition_communication_type communication_ports;
    // Partion HM Table
    void* entry_point;
    system_partition_type is_system_partition;
    OPERATING_MODE_TYPE operating_mode;
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

#endif