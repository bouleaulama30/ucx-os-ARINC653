#include <ucx.h>


int32_t partition_init(SYSTEM_TIME_TYPE PERIOD, 
                        SYSTEM_TIME_TYPE DURATION,
                        PARTITION_ID_TYPE IDENTIFIER,
                        NUM_CORES_TYPE    NUM_ASSIGNED_CORES,
                        const PARTITION_NAME_TYPE name,
                        const REGION_NAME_TYPE   region_name_code_mem,
                        SYSTEM_ADDRESS_TYPE base_code_mem,
                        APEX_UNSIGNED      size_code_mem,
                        const ACCESS_TYPE        access_code_mem,
                        const REGION_NAME_TYPE   region_name_data_mem,
                        SYSTEM_ADDRESS_TYPE base_data_mem,
                        APEX_UNSIGNED      size_data_mem,
                        const ACCESS_TYPE        access_data_mem,
                        SYSTEM_ADDRESS_TYPE entry_point,
                        SYSTEM_PARTITION_TYPE is_system_partition)
{
    // déclaration des structures
	struct pcb_s *new_pcb;
	struct node_s *new_partition;
    PARTITION_STATUS_TYPE* status;
    MEMORY_REGION_TYPE* code_region;
    MEMORY_REGION_TYPE* data_region;
    MEMORY_REQUIREMENTS_TYPE* memory_requirements; 

    // allocations des structures
	new_pcb = malloc(sizeof(struct pcb_s));
    status = malloc(sizeof(PARTITION_STATUS_TYPE));
    code_region = malloc(sizeof(MEMORY_REGION_TYPE));
    data_region = malloc(sizeof(MEMORY_REGION_TYPE));
    memory_requirements = malloc(sizeof(MEMORY_REQUIREMENTS_TYPE));

    // initialisation des structures
    status->PERIOD = PERIOD;
    status->DURATION = DURATION;
    status->IDENTIFIER = IDENTIFIER;
    status->NUM_ASSIGNED_CORES = NUM_ASSIGNED_CORES;
    status->LOCK_LEVEL = 0; // TODO -Q définir lock level
    status->OPERATING_MODE = IDLE; // TODO -Q définir operating mode
    status->START_CONDITION = NORMAL_START; // TODO -Q définir start condition

    strcpy(code_region->region_name, region_name_code_mem);
    code_region->base = base_code_mem; // TODO -Q définir base code mem
    code_region->size = size_code_mem;
    strcpy(code_region->access, access_code_mem);

    strcpy(data_region->region_name, region_name_data_mem);
    data_region->base = base_data_mem;
    data_region->size = size_data_mem;
    strcpy(data_region->access, access_data_mem);

    memory_requirements->memory[CODE] = *code_region;
    memory_requirements->memory[DATA] = *data_region;


	if (!new_pcb)
		krnl_panic(ERR_PCB_ALLOC);

    /* --- INITIALISATION DE LA PARTIE TCB (POUR LE KERNEL) --- */
    new_pcb->tcb.id = (uint16_t)IDENTIFIER;         
    new_pcb->tcb.task = (void (*)(void))entry_point; 
    new_pcb->tcb.stack = data_region->base;         
    new_pcb->tcb.stack_sz = data_region->size;
    new_pcb->tcb.state = TASK_READY;                
    new_pcb->tcb.priority = TASK_NORMAL_PRIO;       
    new_pcb->tcb.delay = 0;

    CRITICAL_ENTER();

#ifndef MULTICORE
    new_partition = list_pushback(kcb->tasks, new_pcb);
#else
    new_partition = list_pushback(kcb[_cpu_id()]->tasks, new_pcb);
#endif


    if (!new_partition)
		krnl_panic(ERR_PCB_ALLOC);

	new_partition->data = new_pcb;
    new_pcb->status = status;
    strcpy(new_pcb->name, name);
    new_pcb->memory_requirements = memory_requirements;
    new_pcb->entry_point = entry_point;
    new_pcb->is_system_partition = is_system_partition;


    CRITICAL_LEAVE();

    // // initialisation de la mémoire pour le code, TO-DO faire pareil pour la data
    // memset(code_region->base, 0x69, code_region->size);
	// memset(code_region->base, 0x33, 4);
	// memset((code_region->base) + code_region->size - 4, 0x33, 4);

	_context_init(&new_pcb->tcb.context, (size_t)new_pcb->tcb.stack,
		new_pcb->tcb.stack_sz, (size_t)new_pcb->entry_point);

	printf("core %d, partition %d: 0x%p, memory code: 0x%p, memory code size: %d, memory data: 0x%p, memory data size %d\n", _cpu_id(),
		new_pcb->status->IDENTIFIER, new_pcb->entry_point ,new_pcb->memory_requirements->memory[CODE].base, new_pcb->memory_requirements->memory[CODE].size, new_pcb->memory_requirements->memory[DATA].base, new_pcb->memory_requirements->memory[DATA].size);
    return new_pcb->status->IDENTIFIER;
}


void GET_PARTITION_STATUS (
    /*out*/ PARTITION_STATUS_TYPE      *PARTITION_STATUS,
    /*out*/ RETURN_CODE_TYPE           *RETURN_CODE ){
#ifndef MULTICORE
    struct pcb_s* my_partition = kcb->task_current->data;
#else
    struct pcb_s* my_partition = kcb[_cpu_id()]->task_current->data;
#endif
    
    if (!my_partition->status) {
        *RETURN_CODE = NOT_AVAILABLE;
        return;
    }

    *PARTITION_STATUS = *(my_partition->status);
    *RETURN_CODE = NOT_AVAILABLE;       
}


    
void SET_PARTITION_MODE (
       /*in */ OPERATING_MODE_TYPE        OPERATING_MODE,
       /*out*/ RETURN_CODE_TYPE           *RETURN_CODE );
       
void GET_MY_PARTITION_ID(
               /*out*/ PARTITION_ID_TYPE          *PARTITION_ID,
              /*out*/ RETURN_CODE_TYPE           *RETURN_CODE )
{
#ifndef MULTICORE
    struct pcb_s* my_partition = kcb->task_current->data;
#else
    struct pcb_s* my_partition = kcb[_cpu_id()]->task_current->data;
#endif
    *PARTITION_ID = my_partition->status->IDENTIFIER;
    if(*PARTITION_ID)
        *RETURN_CODE = NO_ERROR;
    else
        *RETURN_CODE = NOT_AVAILABLE;       
}
