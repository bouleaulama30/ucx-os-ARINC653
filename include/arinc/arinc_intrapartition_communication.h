
/*----------------------------------------------------------------*/
/*                                                                */
/* BUFFER constant and type definitions and management services   */
/*                                                                */
/*----------------------------------------------------------------*/

#include "arinc_apex_types.h"
#include "arinc_process.h"



#ifndef APEX_BUFFER
#define APEX_BUFFER

#define  MAX_NUMBER_OF_BUFFERS    SYSTEM_LIMIT_NUMBER_OF_BUFFERS

typedef  NAME_TYPE       BUFFER_NAME_TYPE;

typedef  APEX_INTEGER   BUFFER_ID_TYPE;
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

typedef  APEX_INTEGER   BLACKBOARD_ID_TYPE;
#define  NULL_BLACKBOARD_ID           0

typedef  enum { EMPTY = 0, OCCUPIED = 1 } EMPTY_INDICATOR_TYPE;

typedef
   struct {
      EMPTY_INDICATOR_TYPE  EMPTY_INDICATOR;
      MESSAGE_SIZE_TYPE     MAX_MESSAGE_SIZE;
      WAITING_RANGE_TYPE    WAITING_PROCESSES;
   } BLACKBOARD_STATUS_TYPE;

struct blackboard_s {
       BLACKBOARD_NAME_TYPE blackboard_name;
       BLACKBOARD_ID_TYPE blackboard_id;
       PARTITION_ID_TYPE partition_id;
       BLACKBOARD_STATUS_TYPE *blackboard_status;
       struct list_s *waiting_processes;
};

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

typedef  APEX_INTEGER   SEMAPHORE_ID_TYPE;
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

typedef  APEX_INTEGER   EVENT_ID_TYPE;
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

typedef  APEX_INTEGER   MUTEX_ID_TYPE;
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
