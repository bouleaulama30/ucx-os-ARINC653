#include "ucx.h"

static struct node_s *find_partition(struct node_s *node, void *arg){
    struct pcb_s *partition = node->data;
    PARTITION_ID_TYPE id = (PARTITION_ID_TYPE) arg;
    
    if(partition->status->IDENTIFIER == id){
        return node;
    }
    
    return 0;
}   


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
                        BOOLEAN_TYPE is_system_partition)
{
    // déclaration des structures
	struct pcb_s *new_pcb;
	struct node_s *new_partition;
    struct node_s *new_partition_in_partitions;

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
    status->LOCK_LEVEL = 0; 
    status->OPERATING_MODE = COLD_START;
    status->START_CONDITION = NORMAL_START;

    strcpy(code_region->region_name, region_name_code_mem);
    code_region->base = base_code_mem; 
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
    new_pcb->tcb.task = partition_OS;
    new_pcb->tcb.stack = data_region->base;         
    new_pcb->tcb.stack_sz = PARTIION_OS_AND_MAIN_PROCESS_STACK_SIZE;
    new_pcb->tcb.state = TASK_READY;                
    new_pcb->tcb.priority = TASK_NORMAL_PRIO;
    new_pcb->tcb.rt_prio = 0;
    new_pcb->tcb.delay = 0;

    CRITICAL_ENTER();

#ifndef MULTICORE
    new_partition = list_pushback(kcb->tasks, new_pcb);
    new_partition_in_partitions = list_pushback(kcb->partitions, new_pcb);
    
#else
    new_partition = list_pushback(kcb[_cpu_id()]->tasks, new_pcb);
    new_partition_in_partitions = list_pushback(kcb[_cpu_id()]->partitions, new_pcb);
#endif


    if (!new_partition || !new_partition_in_partitions)
		krnl_panic(ERR_PCB_ALLOC);


    new_pcb->status = status;
    strcpy(new_pcb->name, name);
    new_pcb->memory_requirements = memory_requirements;
    new_pcb->entry_point = entry_point;
    new_pcb->is_system_partition = is_system_partition;
    new_pcb->nbr_processes = 0;
    new_pcb->id_next = 0;
    new_pcb->next_stack_addr = memory_requirements->memory[DATA].base + PARTIION_OS_AND_MAIN_PROCESS_STACK_SIZE;
    new_pcb->processes = list_create();


    CRITICAL_LEAVE();

    _context_init(&new_pcb->tcb.context, (size_t)new_pcb->tcb.stack,
        new_pcb->tcb.stack_sz, (size_t)new_pcb->tcb.task);

	printf("core %d, partition %d: 0x%p, memory code: 0x%p, memory code size: %d, memory data: 0x%p, memory data size %d\n", _cpu_id(),
		new_pcb->status->IDENTIFIER, new_pcb->entry_point ,new_pcb->memory_requirements->memory[CODE].base, new_pcb->memory_requirements->memory[CODE].size, new_pcb->memory_requirements->memory[DATA].base, new_pcb->memory_requirements->memory[DATA].size);
    return new_pcb->status->IDENTIFIER;
}



int32_t activate_partition(PARTITION_ID_TYPE IDENTIFIER){
    if (IDENTIFIER == IDLE_PARTITION_ID) {
        _pmp_partition_activate((uint32_t)_kernel_end, (uint32_t)0, (uint32_t)0);
#ifndef MULTICORE
        kcb->partition_current = NULL;
#else
        kcb[_cpu_id()]->partition_current = NULL;
#endif
        return IDLE_PARTITION_ID;
    }

#ifndef MULTICORE
    struct node_s *partition_node = list_foreach(kcb->partitions, find_partition, (void *)IDENTIFIER);
    if(!partition_node){
        krnl_panic(ERR_FAIL);
    }

    struct pcb_s *partition = partition_node->data;
    uint32_t partition_start_addr = (uint32_t) partition->memory_requirements->memory[CODE].base;
    uint32_t partition_end_addr = (uint32_t) partition->memory_requirements->memory[DATA].base + partition->memory_requirements->memory[DATA].size;
    printf("start partition addr: %x, end partition addr: %x\n", partition_start_addr, partition_end_addr);

    _pmp_partition_activate((uint32_t) _kernel_end, partition_start_addr, partition_end_addr);

    if(partition->status->OPERATING_MODE == IDLE){
        int32_t id = activate_partition(IDLE_PARTITION_ID);
        return id;
    }
    kcb->partition_current = partition_node;

#else
    struct node_s *partition_node = list_foreach(kcb[_cpu_id()]->partitions, find_partition, (void *)IDENTIFIER);

    if(!partition_node){
        krnl_panic(ERR_FAIL);
    }

    struct pcb_s *partition = partition_node->data;
    uint32_t partition_start_addr = (uint32_t) partition->memory_requirements->memory[CODE].base;
    uint32_t partition_end_addr = (uint32_t) partition->memory_requirements->memory[DATA].base + partition->memory_requirements->memory[DATA].size;
    printf("start partition addr: %x, end partition addr: %x\n", partition_start_addr, partition_end_addr);
    _pmp_partition_activate((uint32_t) _kernel_end, partition_start_addr, partition_end_addr);
    if(partition->status->OPERATING_MODE == IDLE){
        int32_t id = activate_partition(IDLE_PARTITION_ID);
        return id;
    }

    kcb[_cpu_id()]->partition_current = partition_node;
#endif
    
    return IDENTIFIER;
}
