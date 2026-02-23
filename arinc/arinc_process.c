#include <ucx.h>

static struct node_s *find_process_names(struct node_s *node, void *arg){
    const char *process_name = (const char *)node->data;
    const char *name = (const char *) arg;
    
    printf("process name %s, name %s \n", process_name, name);
    if(strcmp(process_name, name) == 0){
        return node;
    }
    return 0;
}

static int is_process_name_existed(struct pcb_s *partition ,PROCESS_NAME_TYPE process_name){
    struct node_s *same_process_name_node = list_foreach(partition->process_names, find_process_names, (void *)process_name);
    if(same_process_name_node){
        return 1;
    }
    return 0;
}


void CREATE_PROCESS (
       /*in */ PROCESS_ATTRIBUTE_TYPE   *ATTRIBUTES,
       /*out*/ PROCESS_ID_TYPE          *PROCESS_ID,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE ){

#ifndef MULTICORE
    struct node_s *partition_node = kcb->partition_current;
#else
    struct node_s *partition_node = kcb[_cpu_id()]->partition_current;
#endif
    struct pcb_s *partition = partition_node->data;
    if(partition->nbr_tasks >= MAX_NUMBER_OF_PROCESSES){
        *RETURN_CODE = INVALID_CONFIG;
    } 
    
    else if (ATTRIBUTES->STACK_SIZE > partition->storage_capacity)
    {
        *RETURN_CODE = INVALID_CONFIG;
    }
    else if (is_process_name_existed(partition, ATTRIBUTES->NAME))
    {
        *RETURN_CODE = NO_ACTION;
    }
    else if (ATTRIBUTES->STACK_SIZE > partition->memory_requirements->memory[DATA].size){
        *RETURN_CODE = INVALID_PARAM;
    }
    else if (ATTRIBUTES->BASE_PRIORITY < MIN_PRIORITY_VALUE || ATTRIBUTES->BASE_PRIORITY > MAX_PRIORITY_VALUE)
    {
        *RETURN_CODE = INVALID_PARAM;
    }
    else if (ATTRIBUTES->PERIOD < -1)
    {
        *RETURN_CODE = INVALID_PARAM;
    }
    else if (ATTRIBUTES->PERIOD >= 0 && (ATTRIBUTES->PERIOD % partition->status->PERIOD != 0))
    {
        *RETURN_CODE = INVALID_CONFIG;
    }
    else if (ATTRIBUTES->TIME_CAPACITY < -1)
    {
        *RETURN_CODE = INVALID_CONFIG;
    }
    else if (ATTRIBUTES->PERIOD >= 0 && ATTRIBUTES->TIME_CAPACITY >= 0 && ATTRIBUTES->TIME_CAPACITY >= 0 && ATTRIBUTES->TIME_CAPACITY > ATTRIBUTES->PERIOD)
    {
        *RETURN_CODE = INVALID_PARAM;
    }
    else if (partition->status->OPERATING_MODE == NORMAL){
        *RETURN_CODE = INVALID_MODE;
    }
    else{
        partition->nbr_tasks++;
        partition->storage_capacity -= ATTRIBUTES->STACK_SIZE;
        list_pushback(partition->process_names, ATTRIBUTES->NAME);
        *RETURN_CODE = NO_ERROR;
    }


}