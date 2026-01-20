#include <ucx.h>



void GET_PARTITION_STATUS (
    /*out*/ PARTITION_STATUS_TYPE      *PARTITION_STATUS,
    /*out*/ RETURN_CODE_TYPE           *RETURN_CODE );
    
void SET_PARTITION_MODE (
       /*in */ OPERATING_MODE_TYPE        OPERATING_MODE,
       /*out*/ RETURN_CODE_TYPE           *RETURN_CODE );
       
void GET_MY_PARTITION_ID(
               /*out*/ PARTITION_ID_TYPE          *PARTITION_ID,
              /*out*/ RETURN_CODE_TYPE           *RETURN_CODE )
       {
           *PARTITION_ID = 2;
           *RETURN_CODE = NO_ERROR;
       
       }
