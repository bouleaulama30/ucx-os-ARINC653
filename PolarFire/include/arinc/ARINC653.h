/*  This is a C language specification of the APEX interface,        	    */
/*  derived from Section 3 of ARINC 653 Part 1.                             */

/*  The declarations of the services given below are taken from the         */
/*  standard, as are the enum types and the names of the others types.      */
/*  Unless specified as implementation dependent, the values specified in   */
/*  this appendix should be implemented as defined.                         */

/*  This C language specification follows the same structure (package) as   */
/*  the Ada specification and the parameters within a structure are defined */
/*  in the same order. The objective is to have the same definitions        */
/*  and representations of the interface in both the Ada and C languages.   */
/*  The two specifications are aligned in the same order, and define the    */
/*  same items.                                                             */

/* Parameter ordering within a struct as well as ID types and sizes are     */
/* implementation dependent. For example, an ID returned by the O/S may be  */
/* based on an index or may be based on a reference to a data structure.    */
/* The APEX_NATIVE_INTEGER and APEX_NATIVE_UNSIGNED types are intended      */
/* to support references to a data structure that scale with the            */
/* processor's native address size.                                         */


/* The APEX specifications will be available via "ARINC653.h" header file.  */


/*----------------------------------------------------------------*/
/*                                                                */
/* Global constant and type definitions                           */
/*                                                                */
/*----------------------------------------------------------------*/

#ifndef APEX_TYPES
#define APEX_TYPES

/*---------------------------*/
/* Domain limits             */
/*---------------------------*/

/*                       Implementation Dependent                          */
/* These values define the domain limits and are implementation dependent. */
/* These values represent typical minimum values supported by an O/S,      */
/* unless otherwise limited (e.g., message size limited by transport).     */
/* For partition scope, the limits are per partition.                      */

#define  SYSTEM_LIMIT_NUMBER_OF_PARTITIONS      32      /* module scope */

#define  SYSTEM_LIMIT_NUMBER_OF_MESSAGES        512     /* module scope */

#define  SYSTEM_LIMIT_MESSAGE_SIZE              8192    /* module scope */

#define  SYSTEM_LIMIT_NUMBER_OF_PROCESSES       128     /* partition scope */

#define  SYSTEM_LIMIT_NUMBER_OF_SAMPLING_PORTS  512     /* partition scope */

#define  SYSTEM_LIMIT_NUMBER_OF_QUEUING_PORTS   512     /* partition scope */

#define  SYSTEM_LIMIT_NUMBER_OF_BUFFERS         256     /* partition scope */

#define  SYSTEM_LIMIT_NUMBER_OF_BLACKBOARDS     256     /* partition scope */

#define  SYSTEM_LIMIT_NUMBER_OF_SEMAPHORES      256     /* partition scope */

#define  SYSTEM_LIMIT_NUMBER_OF_EVENTS          256     /* partition scope */

#define  SYSTEM_LIMIT_NUMBER_OF_MUTEXES         256     /* partition scope */

/*----------------------*/
/* Base APEX types      */
/*----------------------*/

/*                       Implementation Portable                         */
/*  The sizes of these base types are identical for all systems and the  */
/*  actual types must match the types used by the implementation of the  */
/*  underlying Operating System to result in the defined sizes.          */

typedef  unsigned char   APEX_BYTE;             /* 8-bit unsigned  */

typedef  int             APEX_INTEGER;          /* 32-bit signed   */

typedef  unsigned        APEX_UNSIGNED;         /* 32-bit unsigned */

typedef  long long     	 APEX_LONG_INTEGER;     /* 64-bit signed   */


/*  The following types are either 32-bit or 64-bit and will match */ 
/*  the processor's native address size.                           */
typedef  long            APEX_NATIVE_INTEGER; 
typedef  unsigned long   APEX_NATIVE_UNSIGNED; 


/*----------------------*/
/* General APEX types   */
/*----------------------*/

typedef
   enum {
        NO_ERROR       = 0,   /* request valid and operation performed      */
        NO_ACTION      = 1,   /* status of system unaffected by request     */
        NOT_AVAILABLE  = 2,   /* resource required by request unavailable   */
        INVALID_PARAM  = 3,   /* invalid parameter specified in request     */
        INVALID_CONFIG = 4,   /* parameter incompatible with configuration  */
        INVALID_MODE   = 5,   /* request incompatible with current mode     */
        TIMED_OUT      = 6    /* time-out tied up with request has expired  */
   } RETURN_CODE_TYPE;

#define  MAX_NAME_LENGTH          32

typedef  char            NAME_TYPE[MAX_NAME_LENGTH];

/* the size of address pointers must match the address range supported by   */
/* the processor (e.g., 32-bits or 64-bits as supported or as configured    */
/* for a processor).                                                        */
typedef  void            (* SYSTEM_ADDRESS_TYPE);

typedef  APEX_BYTE *     MESSAGE_ADDR_TYPE;

typedef  APEX_INTEGER    MESSAGE_SIZE_TYPE;

typedef  APEX_INTEGER    MESSAGE_RANGE_TYPE;

typedef  enum { SOURCE = 0, DESTINATION = 1 } PORT_DIRECTION_TYPE;

typedef  enum { FIFO = 0, PRIORITY = 1 } QUEUING_DISCIPLINE_TYPE;

typedef  APEX_LONG_INTEGER   SYSTEM_TIME_TYPE;
                           /* 64-bit signed integer with a 1 nanosecond LSB */

#define  INFINITE_TIME_VALUE    -1

typedef  APEX_INTEGER           PROCESSOR_CORE_ID_TYPE; 
#define  CORE_AFFINITY_NO_PREFERENCE       <implementation dependent> 

#endif


/*----------------------------------------------------------------*/
/*                                                                */
/* TIME constant and type definitions and management services     */
/*                                                                */
/*----------------------------------------------------------------*/

#ifndef APEX_TIME
#define APEX_TIME

extern void TIMED_WAIT (
       /*in */ SYSTEM_TIME_TYPE         DELAY_TIME,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void PERIODIC_WAIT (
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void GET_TIME (
       /*out*/ SYSTEM_TIME_TYPE         *SYSTEM_TIME,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void REPLENISH (
       /*in */ SYSTEM_TIME_TYPE         BUDGET_TIME,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

#endif


/*----------------------------------------------------------------*/
/*                                                                */
/* PROCESS constant and type definitions and management services  */
/*                                                                */
/*----------------------------------------------------------------*/
       
#ifndef APEX_PROCESS
#define APEX_PROCESS

#define  MAX_NUMBER_OF_PROCESSES  SYSTEM_LIMIT_NUMBER_OF_PROCESSES

#define  MIN_PRIORITY_VALUE       1

#define  MAX_PRIORITY_VALUE       239

#define  MAX_LOCK_LEVEL           16

typedef  NAME_TYPE            PROCESS_NAME_TYPE;

typedef  <implementation dependent>        PROCESS_ID_TYPE;
#define  NULL_PROCESS_ID          0
#define  MAIN_PROCESS_ID          <implementation dependent>

typedef  APEX_INTEGER		PROCESS_INDEX_TYPE;

typedef  APEX_INTEGER         LOCK_LEVEL_TYPE;

typedef  APEX_UNSIGNED        STACK_SIZE_TYPE;

typedef  APEX_INTEGER         WAITING_RANGE_TYPE;

typedef  APEX_INTEGER         PRIORITY_TYPE;


typedef
   enum {
        DORMANT  = 0,
        READY    = 1,
        RUNNING  = 2,
        WAITING  = 3,
        FAULTED  = 4
   } PROCESS_STATE_TYPE;

typedef  enum { SOFT = 0, HARD = 1 } DEADLINE_TYPE;


typedef
   struct {
      SYSTEM_TIME_TYPE      PERIOD;
      SYSTEM_TIME_TYPE      TIME_CAPACITY;
      SYSTEM_ADDRESS_TYPE   ENTRY_POINT;
      STACK_SIZE_TYPE       STACK_SIZE;
      PRIORITY_TYPE         BASE_PRIORITY;
      DEADLINE_TYPE         DEADLINE;
      PROCESS_NAME_TYPE     NAME;
   } PROCESS_ATTRIBUTE_TYPE;

typedef
   struct {
      SYSTEM_TIME_TYPE        DEADLINE_TIME;
      PRIORITY_TYPE           CURRENT_PRIORITY;
      PROCESS_STATE_TYPE      PROCESS_STATE;
      PROCESS_ATTRIBUTE_TYPE  ATTRIBUTES;
   } PROCESS_STATUS_TYPE;


extern void CREATE_PROCESS (
       /*in */ PROCESS_ATTRIBUTE_TYPE   *ATTRIBUTES,
       /*out*/ PROCESS_ID_TYPE          *PROCESS_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void SET_PRIORITY (
       /*in */ PROCESS_ID_TYPE          PROCESS_ID,
       /*in */ PRIORITY_TYPE            PRIORITY,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void SUSPEND_SELF (
       /*in */ SYSTEM_TIME_TYPE         TIME_OUT,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void SUSPEND (
       /*in */ PROCESS_ID_TYPE          PROCESS_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void RESUME (
       /*in */ PROCESS_ID_TYPE          PROCESS_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void STOP_SELF (void);

extern void STOP (
       /*in */ PROCESS_ID_TYPE          PROCESS_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void START (
       /*in */ PROCESS_ID_TYPE          PROCESS_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void DELAYED_START (
       /*in */ PROCESS_ID_TYPE          PROCESS_ID,
       /*in */ SYSTEM_TIME_TYPE         DELAY_TIME,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void LOCK_PREEMPTION (
       /*out*/ LOCK_LEVEL_TYPE          *LOCK_LEVEL,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void UNLOCK_PREEMPTION (
       /*out*/ LOCK_LEVEL_TYPE          *LOCK_LEVEL,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void GET_MY_ID (
       /*out*/ PROCESS_ID_TYPE          *PROCESS_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void GET_PROCESS_ID (
       /*in */ PROCESS_NAME_TYPE        PROCESS_NAME,
       /*out*/ PROCESS_ID_TYPE          *PROCESS_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void GET_PROCESS_STATUS (
       /*in */ PROCESS_ID_TYPE          PROCESS_ID,
       /*out*/ PROCESS_STATUS_TYPE      *PROCESS_STATUS,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void INITIALIZE_PROCESS_CORE_AFFINITY (
       /*in */ PROCESS_ID_TYPE          PROCESS_ID,
       /*in */ PROCESSOR_CORE_ID_TYPE   PROCESSOR_CORE_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void GET_MY_PROCESSOR_CORE_ID (
       /*out*/ PROCESSOR_CORE_ID_TYPE   *PROCESSOR_CORE_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void GET_MY_INDEX (
       /*out*/ PROCESS_INDEX_TYPE       *PROCESS_INDEX,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

#endif

/*----------------------------------------------------------------*/
/*                                                                */
/* PARTITION constant and type definitions and management services*/
/*                                                                */
/*----------------------------------------------------------------*/

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

/*--------------------------------------------------------------------*/
/*                                                                    */
/* SAMPLING PORT constant and type definitions and management services*/
/*                                                                    */
/*--------------------------------------------------------------------*/

#ifndef APEX_SAMPLING
#define APEX_SAMPLING

#define  MAX_NUMBER_OF_SAMPLING_PORTS   SYSTEM_LIMIT_NUMBER_OF_SAMPLING_PORTS

typedef  NAME_TYPE       SAMPLING_PORT_NAME_TYPE;

typedef  <implementation dependent>   SAMPLING_PORT_ID_TYPE;
#define  NULL_SAMPLING_PORT_ID          0

typedef  enum { INVALID = 0, VALID = 1 } VALIDITY_TYPE;

typedef
   struct {
      SYSTEM_TIME_TYPE         REFRESH_PERIOD;
      MESSAGE_SIZE_TYPE        MAX_MESSAGE_SIZE;
      PORT_DIRECTION_TYPE      PORT_DIRECTION;
      VALIDITY_TYPE            LAST_MSG_VALIDITY;
   } SAMPLING_PORT_STATUS_TYPE;


extern void CREATE_SAMPLING_PORT (
       /*in */ SAMPLING_PORT_NAME_TYPE    SAMPLING_PORT_NAME,
       /*in */ MESSAGE_SIZE_TYPE          MAX_MESSAGE_SIZE,
       /*in */ PORT_DIRECTION_TYPE        PORT_DIRECTION,
       /*in */ SYSTEM_TIME_TYPE           REFRESH_PERIOD,
       /*out*/ SAMPLING_PORT_ID_TYPE      *SAMPLING_PORT_ID,
       /*out*/ RETURN_CODE_TYPE           *RETURN_CODE );

extern void WRITE_SAMPLING_MESSAGE (
       /*in */ SAMPLING_PORT_ID_TYPE      SAMPLING_PORT_ID,
       /*in */ MESSAGE_ADDR_TYPE          MESSAGE_ADDR,     /* by reference */
       /*in */ MESSAGE_SIZE_TYPE          LENGTH,
       /*out*/ RETURN_CODE_TYPE           *RETURN_CODE );

extern void READ_SAMPLING_MESSAGE (
       /*in */ SAMPLING_PORT_ID_TYPE      SAMPLING_PORT_ID,
       /*in */ MESSAGE_ADDR_TYPE          MESSAGE_ADDR,
               /* The message address is passed IN, although */
               /* the respective message is passed OUT       */
       /*out*/ MESSAGE_SIZE_TYPE          *LENGTH,
       /*out*/ VALIDITY_TYPE              *VALIDITY,
       /*out*/ RETURN_CODE_TYPE           *RETURN_CODE );

extern void GET_SAMPLING_PORT_ID (
       /*in */ SAMPLING_PORT_NAME_TYPE    SAMPLING_PORT_NAME,
       /*out*/ SAMPLING_PORT_ID_TYPE      *SAMPLING_PORT_ID,
       /*out*/ RETURN_CODE_TYPE           *RETURN_CODE );

extern void GET_SAMPLING_PORT_STATUS (
       /*in */ SAMPLING_PORT_ID_TYPE      SAMPLING_PORT_ID,
       /*out*/ SAMPLING_PORT_STATUS_TYPE  *SAMPLING_PORT_STATUS,
       /*out*/ RETURN_CODE_TYPE           *RETURN_CODE );

#endif

/*--------------------------------------------------------------------*/
/*                                                                    */
/* QUEUING PORT constant and type definitions and management services */
/*                                                                    */
/*--------------------------------------------------------------------*/

#ifndef APEX_QUEUING
#define APEX_QUEUING

#define  MAX_NUMBER_OF_QUEUING_PORTS    SYSTEM_LIMIT_NUMBER_OF_QUEUING_PORTS

typedef  NAME_TYPE       QUEUING_PORT_NAME_TYPE;

typedef  <implementation dependent>   QUEUING_PORT_ID_TYPE;
#define  NULL_QUEUING_PORT_ID          0

typedef
   struct {
      MESSAGE_RANGE_TYPE      NB_MESSAGE;
      MESSAGE_RANGE_TYPE      MAX_NB_MESSAGE;
      MESSAGE_SIZE_TYPE       MAX_MESSAGE_SIZE;
      PORT_DIRECTION_TYPE     PORT_DIRECTION;
      WAITING_RANGE_TYPE      WAITING_PROCESSES;
   } QUEUING_PORT_STATUS_TYPE;


extern void CREATE_QUEUING_PORT (
       /*in */ QUEUING_PORT_NAME_TYPE   QUEUING_PORT_NAME,
       /*in */ MESSAGE_SIZE_TYPE        MAX_MESSAGE_SIZE,
       /*in */ MESSAGE_RANGE_TYPE       MAX_NB_MESSAGE,
       /*in */ PORT_DIRECTION_TYPE      PORT_DIRECTION,
       /*in */ QUEUING_DISCIPLINE_TYPE  QUEUING_DISCIPLINE,
       /*out*/ QUEUING_PORT_ID_TYPE     *QUEUING_PORT_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void SEND_QUEUING_MESSAGE (
       /*in */ QUEUING_PORT_ID_TYPE     QUEUING_PORT_ID,
       /*in */ MESSAGE_ADDR_TYPE        MESSAGE_ADDR,       /* by reference */
       /*in */ MESSAGE_SIZE_TYPE        LENGTH,
       /*in */ SYSTEM_TIME_TYPE         TIME_OUT,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE);

extern void RECEIVE_QUEUING_MESSAGE (
       /*in */ QUEUING_PORT_ID_TYPE     QUEUING_PORT_ID,
       /*in */ SYSTEM_TIME_TYPE         TIME_OUT,
       /*in */ MESSAGE_ADDR_TYPE        MESSAGE_ADDR,
               /* The message address is passed IN, although */
               /* the respective message is passed OUT       */
       /*out*/ MESSAGE_SIZE_TYPE        *LENGTH,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void GET_QUEUING_PORT_ID (
       /*in */ QUEUING_PORT_NAME_TYPE   QUEUING_PORT_NAME,
       /*out*/ QUEUING_PORT_ID_TYPE     *QUEUING_PORT_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void GET_QUEUING_PORT_STATUS (
       /*in */ QUEUING_PORT_ID_TYPE     QUEUING_PORT_ID,
       /*out*/ QUEUING_PORT_STATUS_TYPE *QUEUING_PORT_STATUS,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void CLEAR_QUEUING_PORT (
       /*in */ QUEUING_PORT_ID_TYPE     QUEUING_PORT_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

#endif

/*----------------------------------------------------------------*/
/*                                                                */
/* BUFFER constant and type definitions and management services   */
/*                                                                */
/*----------------------------------------------------------------*/

#ifndef APEX_BUFFER
#define APEX_BUFFER

#define  MAX_NUMBER_OF_BUFFERS    SYSTEM_LIMIT_NUMBER_OF_BUFFERS

typedef  NAME_TYPE       BUFFER_NAME_TYPE;

typedef  <implementation dependent>   BUFFER_ID_TYPE;
#define  NULL_BUFFER_ID               0

typedef
   struct {
      MESSAGE_RANGE_TYPE  NB_MESSAGE;
      MESSAGE_RANGE_TYPE  MAX_NB_MESSAGE;
      MESSAGE_SIZE_TYPE   MAX_MESSAGE_SIZE;
      WAITING_RANGE_TYPE  WAITING_PROCESSES;
   } BUFFER_STATUS_TYPE;


extern void CREATE_BUFFER (
       /*in */ BUFFER_NAME_TYPE         BUFFER_NAME,
       /*in */ MESSAGE_SIZE_TYPE        MAX_MESSAGE_SIZE,
       /*in */ MESSAGE_RANGE_TYPE       MAX_NB_MESSAGE,
       /*in */ QUEUING_DISCIPLINE_TYPE  QUEUING_DISCIPLINE,
       /*out*/ BUFFER_ID_TYPE           *BUFFER_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void SEND_BUFFER (
       /*in */ BUFFER_ID_TYPE           BUFFER_ID,
       /*in */ MESSAGE_ADDR_TYPE        MESSAGE_ADDR,       /* by reference */
       /*in */ MESSAGE_SIZE_TYPE        LENGTH,
       /*in */ SYSTEM_TIME_TYPE         TIME_OUT,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void RECEIVE_BUFFER (
       /*in */ BUFFER_ID_TYPE           BUFFER_ID,
       /*in */ SYSTEM_TIME_TYPE         TIME_OUT,
       /*in */ MESSAGE_ADDR_TYPE        MESSAGE_ADDR,
               /* The message address is passed IN, although */
               /* the respective message is passed OUT       */
       /*out*/ MESSAGE_SIZE_TYPE        *LENGTH,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void GET_BUFFER_ID (
       /*in */ BUFFER_NAME_TYPE         BUFFER_NAME,
       /*out*/ BUFFER_ID_TYPE           *BUFFER_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void GET_BUFFER_STATUS (
       /*in */ BUFFER_ID_TYPE           BUFFER_ID,
       /*out*/ BUFFER_STATUS_TYPE       *BUFFER_STATUS,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

#endif

/*-------------------------------------------------------------------*/
/*                                                                   */
/* BLACKBOARD constant and type definitions and management services  */
/*                                                                   */
/*-------------------------------------------------------------------*/

#ifndef APEX_BLACKBOARD
#define APEX_BLACKBOARD

#define  MAX_NUMBER_OF_BLACKBOARDS      SYSTEM_LIMIT_NUMBER_OF_BLACKBOARDS

typedef  NAME_TYPE       BLACKBOARD_NAME_TYPE;

typedef  <implementation dependent>   BLACKBOARD_ID_TYPE;
#define  NULL_BLACKBOARD_ID           0

typedef  enum { EMPTY = 0, OCCUPIED = 1 } EMPTY_INDICATOR_TYPE;

typedef
   struct {
      EMPTY_INDICATOR_TYPE  EMPTY_INDICATOR;
      MESSAGE_SIZE_TYPE     MAX_MESSAGE_SIZE;
      WAITING_RANGE_TYPE    WAITING_PROCESSES;
   } BLACKBOARD_STATUS_TYPE;


extern void CREATE_BLACKBOARD (
       /*in */ BLACKBOARD_NAME_TYPE     BLACKBOARD_NAME,
       /*in */ MESSAGE_SIZE_TYPE        MAX_MESSAGE_SIZE,
       /*out*/ BLACKBOARD_ID_TYPE       *BLACKBOARD_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void DISPLAY_BLACKBOARD (
       /*in */ BLACKBOARD_ID_TYPE       BLACKBOARD_ID,
       /*in */ MESSAGE_ADDR_TYPE        MESSAGE_ADDR,       /* by reference */
       /*in */ MESSAGE_SIZE_TYPE        LENGTH,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void READ_BLACKBOARD (
       /*in */ BLACKBOARD_ID_TYPE       BLACKBOARD_ID,
       /*in */ SYSTEM_TIME_TYPE         TIME_OUT,
       /*in */ MESSAGE_ADDR_TYPE        MESSAGE_ADDR,
               /* The message address is passed IN, although */
               /* the respective message is passed OUT       */
       /*out*/ MESSAGE_SIZE_TYPE        *LENGTH,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void CLEAR_BLACKBOARD (
       /*in */ BLACKBOARD_ID_TYPE       BLACKBOARD_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void GET_BLACKBOARD_ID (
       /*in */ BLACKBOARD_NAME_TYPE     BLACKBOARD_NAME,
       /*out*/ BLACKBOARD_ID_TYPE       *BLACKBOARD_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void GET_BLACKBOARD_STATUS (
       /*in */ BLACKBOARD_ID_TYPE       BLACKBOARD_ID,
       /*out*/ BLACKBOARD_STATUS_TYPE   *BLACKBOARD_STATUS,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

#endif

/*----------------------------------------------------------------*/
/*                                                                */
/* SEMAPHORE constant and type definitions and management services*/
/*                                                                */
/*----------------------------------------------------------------*/

#ifndef APEX_SEMAPHORE
#define APEX_SEMAPHORE

#define  MAX_NUMBER_OF_SEMAPHORES  SYSTEM_LIMIT_NUMBER_OF_SEMAPHORES

#define  MAX_SEMAPHORE_VALUE       32767

typedef  NAME_TYPE       SEMAPHORE_NAME_TYPE;

typedef  <implementation dependent>   SEMAPHORE_ID_TYPE;
#define  NULL_SEMAPHORE_ID            0

typedef  APEX_INTEGER    SEMAPHORE_VALUE_TYPE;

typedef
   struct {
      SEMAPHORE_VALUE_TYPE  CURRENT_VALUE;
      SEMAPHORE_VALUE_TYPE  MAXIMUM_VALUE;
      WAITING_RANGE_TYPE    WAITING_PROCESSES;
   } SEMAPHORE_STATUS_TYPE;


extern void CREATE_SEMAPHORE (
       /*in */ SEMAPHORE_NAME_TYPE      SEMAPHORE_NAME,
       /*in */ SEMAPHORE_VALUE_TYPE     CURRENT_VALUE,
       /*in */ SEMAPHORE_VALUE_TYPE     MAXIMUM_VALUE,
       /*in */ QUEUING_DISCIPLINE_TYPE  QUEUING_DISCIPLINE,
       /*out*/ SEMAPHORE_ID_TYPE        *SEMAPHORE_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void WAIT_SEMAPHORE (
       /*in */ SEMAPHORE_ID_TYPE        SEMAPHORE_ID,
       /*in */ SYSTEM_TIME_TYPE         TIME_OUT,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void SIGNAL_SEMAPHORE (
       /*in */ SEMAPHORE_ID_TYPE        SEMAPHORE_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void GET_SEMAPHORE_ID (
       /*in */ SEMAPHORE_NAME_TYPE      SEMAPHORE_NAME,
       /*out*/ SEMAPHORE_ID_TYPE        *SEMAPHORE_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void GET_SEMAPHORE_STATUS (
       /*in */ SEMAPHORE_ID_TYPE        SEMAPHORE_ID,
       /*out*/ SEMAPHORE_STATUS_TYPE    *SEMAPHORE_STATUS,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

#endif


/*----------------------------------------------------------------*/
/*                                                                */
/* EVENT constant and type definitions and management services    */
/*                                                                */
/*----------------------------------------------------------------*/

#ifndef APEX_EVENT
#define APEX_EVENT

#define  MAX_NUMBER_OF_EVENTS     SYSTEM_LIMIT_NUMBER_OF_EVENTS

typedef  NAME_TYPE       EVENT_NAME_TYPE;

typedef  <implementation dependent>   EVENT_ID_TYPE;
#define  NULL_EVENT_ID                0

typedef  enum { DOWN = 0, UP = 1 } EVENT_STATE_TYPE;

typedef
   struct {
      EVENT_STATE_TYPE    EVENT_STATE;
      WAITING_RANGE_TYPE  WAITING_PROCESSES;
   } EVENT_STATUS_TYPE;


extern void CREATE_EVENT (
       /*in */ EVENT_NAME_TYPE          EVENT_NAME,
       /*out*/ EVENT_ID_TYPE            *EVENT_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void SET_EVENT (
       /*in */ EVENT_ID_TYPE            EVENT_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void RESET_EVENT (
       /*in */ EVENT_ID_TYPE            EVENT_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void WAIT_EVENT (
       /*in */ EVENT_ID_TYPE            EVENT_ID,
       /*in */ SYSTEM_TIME_TYPE         TIME_OUT,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void GET_EVENT_ID (
       /*in */ EVENT_NAME_TYPE          EVENT_NAME,
       /*out*/ EVENT_ID_TYPE            *EVENT_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void GET_EVENT_STATUS (
       /*in */ EVENT_ID_TYPE            EVENT_ID,
       /*out*/ EVENT_STATUS_TYPE        *EVENT_STATUS,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void APERIODIC_WAIT_EVENT (
       /*in */ EVENT_ID_TYPE            EVENT_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

#endif

/*----------------------------------------------------------------*/
/*                                                                */
/* MUTEX constant and type definitions and management services    */
/*                                                                */
/*----------------------------------------------------------------*/

#ifndef APEX_MUTEX
#define APEX_MUTEX

#define  MAX_NUMBER_OF_MUTEXES  SYSTEM_LIMIT_NUMBER_OF_MUTEXES

typedef  NAME_TYPE       MUTEX_NAME_TYPE;

typedef  <implementation dependent>   MUTEX_ID_TYPE;
#define  NULL_MUTEX_ID           0
#define  NO_MUTEX_OWNED          <implementation dependent>
#define  PREEMPTION_LOCK_MUTEX   <implementation dependent>

typedef  APEX_INTEGER    LOCK_COUNT_TYPE;

typedef  enum { AVAILABLE = 0, OWNED = 1 } MUTEX_STATE_TYPE;


typedef
   struct {
      PROCESS_ID_TYPE       MUTEX_OWNER;
      MUTEX_STATE_TYPE      MUTEX_STATE;
      PRIORITY_TYPE         MUTEX_PRIORITY;
      LOCK_COUNT_TYPE       LOCK_COUNT;
      WAITING_RANGE_TYPE    WAITING_PROCESSES;
   } MUTEX_STATUS_TYPE;


extern void CREATE_MUTEX (
       /*in */ MUTEX_NAME_TYPE          MUTEX_NAME,
       /*in */ PRIORITY_TYPE            MUTEX_PRIORITY,
       /*in */ QUEUING_DISCIPLINE_TYPE  QUEUING_DISCIPLINE,
       /*out*/ MUTEX_ID_TYPE            *MUTEX_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void ACQUIRE_MUTEX (
       /*in */ MUTEX_ID_TYPE            MUTEX_ID,
       /*in */ SYSTEM_TIME_TYPE         TIME_OUT,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void RELEASE_MUTEX (
       /*in */ MUTEX_ID_TYPE            MUTEX_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void RESET_MUTEX (
       /*in */ MUTEX_ID_TYPE            MUTEX_ID,
       /*in */ PROCESS_ID_TYPE          PROCESS_ID, 
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void GET_MUTEX_ID (
       /*in */ MUTEX_NAME_TYPE          MUTEX_NAME,
       /*out*/ MUTEX_ID_TYPE            *MUTEX_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void GET_MUTEX_STATUS (
       /*in */ MUTEX_ID_TYPE            MUTEX_ID,
       /*out*/ MUTEX_STATUS_TYPE        *MUTEX_STATUS,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void GET_PROCESS_MUTEX_STATE (
       /*in */ PROCESS_ID_TYPE          PROCESS_ID,
       /*out*/ MUTEX_ID_TYPE            *MUTEX_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

#endif

/*----------------------------------------------------------------*/
/*                                                                */
/* ERROR constant and type definitions and management services    */
/*                                                                */
/*----------------------------------------------------------------*/

#ifndef APEX_ERROR
#define APEX_ERROR

#define  MAX_ERROR_MESSAGE_SIZE         128

typedef  APEX_INTEGER   ERROR_MESSAGE_SIZE_TYPE;

typedef  APEX_BYTE      ERROR_MESSAGE_TYPE[MAX_ERROR_MESSAGE_SIZE];

typedef
   enum {
        DEADLINE_MISSED   = 0,
        APPLICATION_ERROR = 1,
        NUMERIC_ERROR     = 2,
        ILLEGAL_REQUEST   = 3,
        STACK_OVERFLOW    = 4,
        MEMORY_VIOLATION  = 5,
        HARDWARE_FAULT    = 6,
        POWER_FAIL        = 7
   } ERROR_CODE_TYPE;

typedef
   struct {
      SYSTEM_ADDRESS_TYPE     FAILED_ADDRESS;
      PROCESS_ID_TYPE         FAILED_PROCESS_ID;
      ERROR_CODE_TYPE         ERROR_CODE;
      ERROR_MESSAGE_SIZE_TYPE LENGTH;
      ERROR_MESSAGE_TYPE      MESSAGE;
   } ERROR_STATUS_TYPE;

typedef
   enum {
        PROCESSES_PAUSE     = 0,
        PROCESSES_SCHEDULED = 1
   } ERROR_HANDLER_CONCURRENCY_CONTROL_TYPE;


extern void REPORT_APPLICATION_MESSAGE (
       /*in */   MESSAGE_ADDR_TYPE        MESSAGE_ADDR,
       /*in */   MESSAGE_SIZE_TYPE        LENGTH,
       /*out*/   RETURN_CODE_TYPE         *RETURN_CODE );

extern void CREATE_ERROR_HANDLER (
       /*in */   SYSTEM_ADDRESS_TYPE      ENTRY_POINT,
       /*in */   STACK_SIZE_TYPE          STACK_SIZE,
       /*out*/   RETURN_CODE_TYPE         *RETURN_CODE );

extern void GET_ERROR_STATUS (
       /*out*/   ERROR_STATUS_TYPE        *ERROR_STATUS,
       /*out*/   RETURN_CODE_TYPE         *RETURN_CODE );

extern void RAISE_APPLICATION_ERROR (
       /*in */   ERROR_CODE_TYPE          ERROR_CODE,
       /*in */   MESSAGE_ADDR_TYPE        MESSAGE_ADDR,
       /*in */   ERROR_MESSAGE_SIZE_TYPE  LENGTH,
       /*out*/   RETURN_CODE_TYPE         *RETURN_CODE );

extern void CONFIGURE_ERROR_HANDLER (
       /*in */   ERROR_HANDLER_CONCURRENCY_CONTROL_TYPE  CONCURRENCY_CONTROL,
       /*in */   PROCESSOR_CORE_ID_TYPE                  PROCESSOR_CORE_ID,
       /*out*/   RETURN_CODE_TYPE                        *RETURN_CODE );

#endif
/*============================================================================*/





