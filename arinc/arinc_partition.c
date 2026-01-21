#include <ucx.h>

int32_t partition_init(SYSTEM_TIME_TYPE PERIOD, 
                        SYSTEM_TIME_TYPE DURATION,
                        PARTITION_ID_TYPE IDENTIFIER,
                        NUM_CORES_TYPE    NUM_ASSIGNED_CORES,
                        PARTITION_NAME_TYPE name,
                        REGION_NAME_TYPE   region_name_code_meme,
                        // APEX_UNSIGNED      base_code_mem,
                        APEX_UNSIGNED      size_code_mem,
                        ACCESS_TYPE        access_code_mem,
                        REGION_NAME_TYPE   region_name_data_meme,
                        // APEX_UNSIGNED      base_data_mem,
                        APEX_UNSIGNED      size_data_mem,
                        ACCESS_TYPE        access_data_mem,
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

    strcpy(code_region->region_name, region_name_code_meme);
    code_region->base = malloc(size_code_mem); // TODO -Q définir base code mem
    code_region->size = size_code_mem;
    strcpy(code_region->access, access_code_mem);

    // strcpy(data_region->region_name, region_name_data_meme);
    // data_region->base = base_data_mem;
    // data_region->size = size_data_mem;
    // strcpy(data_region->access, access_data_mem);

    memory_requirements->memory[0] = *code_region;
    memory_requirements->memory[1] = *data_region;


	if (!new_pcb)
		krnl_panic(ERR_PCB_ALLOC);

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

    // initialisation de la mémoire pour le code, TO-DO faire pareil pour la data
    memset(code_region->base, 0x69, code_region->size);
	memset(code_region->base, 0x33, 4);
	memset((code_region->base) + code_region->size - 4, 0x33, 4);

	_context_init(&new_pcb->context, (size_t)code_region->base,
		code_region->size, (size_t)new_pcb->entry_point);

	printf("core %d, task %d: 0x%p, stack: 0x%p, size %d\n", _cpu_id(),
		new_pcb->status->IDENTIFIER, new_pcb->entry_point ,new_pcb->memory_requirements->memory[0].base, new_pcb->memory_requirements->memory[0].size);
    return new_pcb->status->IDENTIFIER;
    

}


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
