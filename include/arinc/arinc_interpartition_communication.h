/*--------------------------------------------------------------------*/
/*                                                                    */
/* SAMPLING PORT constant and type definitions and management services*/
/*                                                                    */
/*--------------------------------------------------------------------*/

#include "arinc_apex_types.h"
#include "arinc_process.h"


#ifndef APEX_SAMPLING
#define APEX_SAMPLING

#define  MAX_NUMBER_OF_SAMPLING_PORTS   SYSTEM_LIMIT_NUMBER_OF_SAMPLING_PORTS

typedef  NAME_TYPE       SAMPLING_PORT_NAME_TYPE;

typedef  APEX_INTEGER   SAMPLING_PORT_ID_TYPE;
#define  NULL_SAMPLING_PORT_ID          0

typedef  enum { INVALID = 0, VALID = 1 } VALIDITY_TYPE;

typedef
   struct {
      SYSTEM_TIME_TYPE         REFRESH_PERIOD;
      MESSAGE_SIZE_TYPE        MAX_MESSAGE_SIZE;
      PORT_DIRECTION_TYPE      PORT_DIRECTION;
      VALIDITY_TYPE            LAST_MSG_VALIDITY;
   } SAMPLING_PORT_STATUS_TYPE;

struct sampling_port_s {
       SAMPLING_PORT_NAME_TYPE sampling_port_name;
       PARTITION_ID_TYPE partition_id;
       SAMPLING_PORT_STATUS_TYPE sampling_port_status;
       struct krnl_sampling_channel *channel;
};

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

typedef  APEX_INTEGER   QUEUING_PORT_ID_TYPE;
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
