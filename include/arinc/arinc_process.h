/*                                                                */
/*                                                                */
/* PROCESS constant and type definitions and management services  */
/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/

#include "arinc_apex_types.h"

#ifndef APEX_PROCESS
#define APEX_PROCESS

#define  MAX_NUMBER_OF_PROCESSES  SYSTEM_LIMIT_NUMBER_OF_PROCESSES

#define  MIN_PRIORITY_VALUE       1

#define  MAX_PRIORITY_VALUE       239

#define  MAX_LOCK_LEVEL           16

//on est en sigle core donc id = 0
#define DEFAULT_PROCESS_CORE_AFFINITY 0

typedef  NAME_TYPE            PROCESS_NAME_TYPE;

typedef  APEX_INTEGER       PROCESS_ID_TYPE;
#define  NULL_PROCESS_ID          0
#define  MAIN_PROCESS_ID          1

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

struct process_s
{
    struct tcb_s tcb;
    PROCESS_STATUS_TYPE *processus_status;
    PROCESS_ID_TYPE process_id;
    PROCESS_INDEX_TYPE process_index;
    PROCESSOR_CORE_ID_TYPE processor_core_affinity;
    SYSTEM_TIME_TYPE release_point_time;
    BOOLEAN_TYPE is_suspended;
    SYSTEM_TIME_TYPE time_counter;
    SYSTEM_TIME_TYPE saved_init_delay;
    struct queuing_port_s *waiting_queuing_port;
    struct blackboard_s *waiting_blackboard;
    struct buffer_s *waiting_buffer;
    MESSAGE_ADDR_TYPE waiting_message_addr;
    MESSAGE_SIZE_TYPE waiting_message_size;
    struct semaphore_s *waiting_semaphore;
    struct event_s *waiting_event;
    struct mutex_s *waiting_mutex; 
    MUTEX_ID_TYPE owned_mutex_id;    
};

static inline void update_process_deadline(struct process_s *process, SYSTEM_TIME_TYPE base_time) {
    if (process->processus_status->ATTRIBUTES.TIME_CAPACITY == INFINITE_TIME_VALUE) {
        // La norme exige que si la capacité est infinie, la deadline devient infinie
        process->processus_status->DEADLINE_TIME = INFINITE_TIME_VALUE;      
    } else {
        // Calcul normal
        process->processus_status->DEADLINE_TIME = base_time + process->processus_status->ATTRIBUTES.TIME_CAPACITY;
    }
}

struct pcb_s;
struct node_s *find_processes_by_id(struct node_s *node, void *arg);
struct node_s *is_process_id_existed(struct pcb_s *partition, PROCESS_ID_TYPE process_id);
struct node_s *find_waiting_process_node(struct node_s *node, void *arg);
void remove_process_from_waiting_queue(struct process_s *process);


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