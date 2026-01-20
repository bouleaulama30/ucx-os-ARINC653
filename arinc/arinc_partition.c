#include <ucx.h>

void GET_MY_PARTITION_ID(
        /*out*/ PARTITION_ID_TYPE          *PARTITION_ID,
       /*out*/ RETURN_CODE_TYPE           *RETURN_CODE )
{
    *PARTITION_ID = 2;
    *RETURN_CODE = NO_ERROR;

}