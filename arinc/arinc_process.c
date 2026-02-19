#include <ucx.h>


void CREATE_PROCESS (
       /*in */ PROCESS_ATTRIBUTE_TYPE   *ATTRIBUTES,
       /*out*/ PROCESS_ID_TYPE          *PROCESS_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){

    printf("CREATE PARTITION\n");
#ifndef MULTICORE
    struct node_s *partition_node = kcb->partition_current;
#else
    struct node_s *partition_node = kcb[_cpu_id()]->partition_current;
#endif
    struct pcb_s *partition = partition_node->data;
    if(partition->nbr_tasks >= MAX_NUMBER_OF_PROCESSES){
        *RETURN_CODE = INVALID_CONFIG;
    } 
    // a faire remain allocated stack
    else if (ATTRIBUTES->STACK_SIZE)
    {
        *RETURN_CODE = INVALID_CONFIG;
    }
    // a faire
    else if (ATTRIBUTES->NAME)
    {
        *RETURN_CODE = NO_ACTION;
    }
    else if (ATTRIBUTES->STACK_SIZE >= partition->memory_requirements[DATA].memory->size){
        *RETURN_CODE = INVALID_PARAM;
    }
    else if (ATTRIBUTES->BASE_PRIORITY < MIN_PRIORITY_VALUE || ATTRIBUTES->BASE_PRIORITY > MAX_PRIORITY_VALUE)
    {
        *RETURN_CODE = INVALID_PARAM;
    }
    // a faire
    else if (ATTRIBUTES->PERIOD)
    {
        *RETURN_CODE = INVALID_PARAM;
    }
    // a faire
    else if (ATTRIBUTES->PERIOD)
    {
        *RETURN_CODE = INVALID_CONFIG;
    }
    // a faire
    else if (ATTRIBUTES->TIME_CAPACITY)
    {
        *RETURN_CODE = INVALID_CONFIG;
    }
    // a faire
    else if (ATTRIBUTES->TIME_CAPACITY)
    {
        *RETURN_CODE = INVALID_PARAM;
    }
    else if (partition->status->OPERATING_MODE == NORMAL){
        *RETURN_CODE = INVALID_MODE;
    }
    else{
        partition->nbr_tasks++;
    }


}