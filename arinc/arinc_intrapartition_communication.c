#include "ucx.h"

int find_blackboard_by_name(struct *partition, char* name){
    for (int i = 0; i < partition->blackboard_count; i++){
        struct blackboard_s *bb = &partition->blackboards[i];
        if (strcmp(bb->blackboard_name, name) == 0){
            return i;
        }
    }
    return -1;  
}

void CREATE_BLACKBOARD (
       /*in */ BLACKBOARD_NAME_TYPE     BLACKBOARD_NAME,
       /*in */ MESSAGE_SIZE_TYPE        MAX_MESSAGE_SIZE,
       /*out*/ BLACKBOARD_ID_TYPE       *BLACKBOARD_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){

    struct pcb_s *partition = get_current_partition();
    
    if (partition->blackboard_count + 1 > partition->max_blackboards || partition->max_blackboard_data_size < MAX_MESSAGE_SIZE){
        *RETURN_CODE = INVALID_CONFIG;
        return;
    }
    
    if (find_blackboard_by_name(partition, BLACKBOARD_NAME) != -1){
        *RETURN_CODE = NO_ACTION;
        return;
    }

    if (MAX_MESSAGE_SIZE <= 0){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    if (partition->status->OPERATING_MODE == NORMAL){
         *RETURN_CODE = INVALID_MODE;
         return;
    }





    

}

void DISPLAY_BLACKBOARD (
       /*in */ BLACKBOARD_ID_TYPE       BLACKBOARD_ID,
       /*in */ MESSAGE_ADDR_TYPE        MESSAGE_ADDR,       /* by reference */
       /*in */ MESSAGE_SIZE_TYPE        LENGTH,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

void READ_BLACKBOARD (
       /*in */ BLACKBOARD_ID_TYPE       BLACKBOARD_ID,
       /*in */ SYSTEM_TIME_TYPE         TIME_OUT,
       /*in */ MESSAGE_ADDR_TYPE        MESSAGE_ADDR,
               /* The message address is passed IN, although */
               /* the respective message is passed OUT       */
       /*out*/ MESSAGE_SIZE_TYPE        *LENGTH,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

void CLEAR_BLACKBOARD (
       /*in */ BLACKBOARD_ID_TYPE       BLACKBOARD_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

void GET_BLACKBOARD_ID (
       /*in */ BLACKBOARD_NAME_TYPE     BLACKBOARD_NAME,
       /*out*/ BLACKBOARD_ID_TYPE       *BLACKBOARD_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

void GET_BLACKBOARD_STATUS (
       /*in */ BLACKBOARD_ID_TYPE       BLACKBOARD_ID,
       /*out*/ BLACKBOARD_STATUS_TYPE   *BLACKBOARD_STATUS,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );