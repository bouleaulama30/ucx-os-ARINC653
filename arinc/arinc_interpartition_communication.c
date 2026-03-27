#include "ucx.h"

void CREATE_SAMPLING_PORT (
       /*in */ SAMPLING_PORT_NAME_TYPE    SAMPLING_PORT_NAME,
       /*in */ MESSAGE_SIZE_TYPE          MAX_MESSAGE_SIZE,
       /*in */ PORT_DIRECTION_TYPE        PORT_DIRECTION,
       /*in */ SYSTEM_TIME_TYPE           REFRESH_PERIOD,
       /*out*/ SAMPLING_PORT_ID_TYPE      *SAMPLING_PORT_ID,
       /*out*/ RETURN_CODE_TYPE           *RETURN_CODE ){
       
    struct pcb_s *partition = get_current_partition();

    if(partition->sampling_port_count + 1 > MAX_NUMBER_OF_SAMPLING_PORTS){
        *RETURN_CODE = INVALID_CONFIG;
        return;
    }
    
    
    
}

void WRITE_SAMPLING_MESSAGE (
       /*in */ SAMPLING_PORT_ID_TYPE      SAMPLING_PORT_ID,
       /*in */ MESSAGE_ADDR_TYPE          MESSAGE_ADDR,     /* by reference */
       /*in */ MESSAGE_SIZE_TYPE          LENGTH,
       /*out*/ RETURN_CODE_TYPE           *RETURN_CODE );

void READ_SAMPLING_MESSAGE (
       /*in */ SAMPLING_PORT_ID_TYPE      SAMPLING_PORT_ID,
       /*in */ MESSAGE_ADDR_TYPE          MESSAGE_ADDR,
               /* The message address is passed IN, although */
               /* the respective message is passed OUT       */
       /*out*/ MESSAGE_SIZE_TYPE          *LENGTH,
       /*out*/ VALIDITY_TYPE              *VALIDITY,
       /*out*/ RETURN_CODE_TYPE           *RETURN_CODE );

void GET_SAMPLING_PORT_ID (
       /*in */ SAMPLING_PORT_NAME_TYPE    SAMPLING_PORT_NAME,
       /*out*/ SAMPLING_PORT_ID_TYPE      *SAMPLING_PORT_ID,
       /*out*/ RETURN_CODE_TYPE           *RETURN_CODE );

void GET_SAMPLING_PORT_STATUS (
       /*in */ SAMPLING_PORT_ID_TYPE      SAMPLING_PORT_ID,
       /*out*/ SAMPLING_PORT_STATUS_TYPE  *SAMPLING_PORT_STATUS,
       /*out*/ RETURN_CODE_TYPE           *RETURN_CODE );