#include <ucx.h>

// uint32_t start_time;
// int time_initialized = 0;

void process_test0(void)
{   
	int32_t cnt = 100000;
    RETURN_CODE_TYPE return_code;
	APEX_INTEGER id;
	GET_MY_PARTITION_ID(&id, &return_code);

	// while (1) {
	// 	if(cnt == 100002){
	// 		// SET_PARTITION_MODE(IDLE, &return_code);
	// 	}
		printf("[process %d %ld, partition %d, address cnt: 0x%p]\n\n", ucx_task_id(), cnt++, id, &cnt);
		// print_time();
	// 	ucx_task_yield();
	// }
}

void process_test1(void)
{   
	int32_t cnt = 200000;
    RETURN_CODE_TYPE return_code;
	APEX_INTEGER id;
	GET_MY_PARTITION_ID(&id, &return_code);

	// while (1) {
	// 	if(cnt == 200002){
	// 		// SET_PARTITION_MODE(IDLE, &return_code);
	// 	}
		printf("[prrocess %d %ld, partition %d, address cnt: 0x%p]\n\n", ucx_task_id(), cnt++, id, &cnt);
		// print_time();
	// 	ucx_task_yield();
	// }
}
void process_test2(void)
{   
	int32_t cnt = 300000;
    RETURN_CODE_TYPE return_code;
	APEX_INTEGER id;
	GET_MY_PARTITION_ID(&id, &return_code);

	while (1) {
		if(cnt == 200002){
			// SET_PARTITION_MODE(IDLE, &return_code);
		}
		printf("[prrocess %d %ld, partition %d, address cnt: 0x%p]\n\n", ucx_task_id(), cnt++, id, &cnt);
		// print_time();
		ucx_task_yield();
	}
}

void process_test3(void)
{   
	int32_t cnt = 400000;
    RETURN_CODE_TYPE return_code;
	APEX_INTEGER id;
	GET_MY_PARTITION_ID(&id, &return_code);

	while (1) {
		if(cnt == 200002){
			// SET_PARTITION_MODE(IDLE, &return_code);
		}
		printf("[prrocess %d %ld, partition %d, address cnt: 0x%p]\n\n", ucx_task_id(), cnt++, id, &cnt);
		// print_time();
		ucx_task_yield();
	}
}

void print_time_idle()
{
	uint32_t secs, msecs, time;
	time = ucx_uptime();

    // if(!time_initialized){
    //     start_time = time;
    //     time_initialized = 1;
    // }
    // time -= start_time;
	secs = time / 1000;
	msecs = time - secs * 1000;
	
	printf("%ld.%03lds\n", secs, msecs);
}

void idle_task(void)
{   

	while (1) {
		printf("[Partition IDLE]\n\n");
        // print_time_idle();

		ucx_task_yield();
	}
}

void update_kcb_task_list(struct list_s *tcb){
#ifndef MULTICORE
    printf("[debug] update_kcb_task_list: tcb=%p\n", tcb);
    if (!tcb || !tcb->head || !tcb->head->next) {
        printf("[debug] update_kcb_task_list: empty task list\n");
        return;
    }
    kcb->tasks = tcb;
    struct node_s *tcb_test_node = kcb->tasks->head->next;
    //a changer mais pour test on met le premier process en tache courante
    kcb->task_current = tcb_test_node;
    printf("[debug] update_kcb_task_list: current node=%p data=%p\n",
        (void *)kcb->task_current, kcb->task_current->data);
    {
        int i = 0;
        struct node_s *node = kcb->tasks->head->next;
        while (node && node->next && i < 3) {
            struct process_s *proc = (struct process_s *)node->data;
            struct tcb_s *task = &proc->tcb;
            printf("[debug] task[%d]: node=%p data=%p id=%d state=%d\n",
                i, (void *)node, (void *)proc, task->id, task->state);
            node = node->next;
            i++;
        }
    }
#else
    printf("[debug] update_kcb_task_list: tcb=%p (cpu %d)\n", tcb, _cpu_id());
    if (!tcb || !tcb->head || !tcb->head->next) {
        printf("[debug] update_kcb_task_list: empty task list (cpu %d)\n", _cpu_id());
        return;
    }
    kcb[_cpu_id()]->tasks = tcb;
    struct node_s *tcb_test_node = kcb[_cpu_id()]->tasks->head->next;
    kcb[_cpu_id()]->task_current = tcb_test_node;
    printf("[debug] update_kcb_task_list: current node=%p data=%p (cpu %d)\n",
        (void *)kcb[_cpu_id()]->task_current, kcb[_cpu_id()]->task_current->data, _cpu_id());
    {
        int i = 0;
        struct node_s *node = kcb[_cpu_id()]->tasks->head->next;
        while (node && node->next && i < 3) {
            struct process_s *proc = (struct process_s *)node->data;
            struct tcb_s *task = &proc->tcb;
            printf("[debug] task[%d]: node=%p data=%p id=%d state=%d (cpu %d)\n",
                i, (void *)node, (void *)proc, task->id, task->state, _cpu_id());
            node = node->next;
            i++;
        }
    }
#endif
    // printf("ID process test %d\n", tcb_test->id);
}

static void partition_trampoline(void)
{
    struct pcb_s *partition;

#ifndef MULTICORE
    partition = kcb->task_current->data;
#else
    partition = kcb[_cpu_id()]->task_current->data;
#endif

    _mprv_activate();

    if(partition->status->IDENTIFIER == IDLE_PARTITION_ID){
        ((void (*)(void))partition->entry_point)();
    }

    if(partition->status->IDENTIFIER == 1){
        RETURN_CODE_TYPE return_code0;
        RETURN_CODE_TYPE return_code1;
        PROCESS_ID_TYPE process_id_0;
        PROCESS_ID_TYPE process_id_1;

        CREATE_PROCESS(&DEFAULT_PROCESS_CONFIG, &process_id_0, &return_code0);
        CREATE_PROCESS(&PROCESS_1_CONFIG, &process_id_1, &return_code1);

        printf("CREATE PROCESS %d and Error code is %d\n", process_id_0, return_code0);
        printf("CREATE PROCESS %d and Error code is %d\n", process_id_1, return_code1);        
    }

    if(partition->status->IDENTIFIER == 2){
        RETURN_CODE_TYPE return_code0;
        RETURN_CODE_TYPE return_code1;
        PROCESS_ID_TYPE process_id_0;
        PROCESS_ID_TYPE process_id_1;

        CREATE_PROCESS(&PROCESS_2_CONFIG, &process_id_0, &return_code0);
        CREATE_PROCESS(&PROCESS_3_CONFIG, &process_id_1, &return_code1);

        printf("CREATE PROCESS %d and Error code is %d\n", process_id_0, return_code0);
        printf("CREATE PROCESS %d and Error code is %d\n", process_id_1, return_code1);
        struct tcb_s* process = partition->processes->head->next->data;   
    }

    struct list_s *processes = partition->processes;
    update_kcb_task_list(processes);
   
    // ((void (*)(void))partition->entry_point)();

    while (1) {
     krnl_schedule();

#ifndef MULTICORE
    
        struct tcb_s *next_task = kcb->task_current->data;
    
#else
    
        struct tcb_s *next_task = kcb[_cpu_id()]->task_current->data;
    
#endif
        if(setjmp(next_task->context)){
            longjmp(next_task->context, 1);
        }
        ucx_task_yield();
    }
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
    // status->OPERATING_MODE = (IDENTIFIER == IDLE_PARTITION_ID) ? NORMAL : (IDENTIFIER == 1 ? NORMAL : NORMAL); 
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
    new_pcb->tcb.task = partition_trampoline;
    new_pcb->tcb.stack = data_region->base;         
    new_pcb->tcb.stack_sz = data_region->size;
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
    new_pcb->storage_capacity = size_data_mem;
    new_pcb->processes = list_create();


    CRITICAL_LEAVE();

    // // initialisation de la mémoire pour le code, TO-DO faire pareil pour la data
    // memset(code_region->base, 0x69, code_region->size);
	// memset(code_region->base, 0x33, 4);
	// memset((code_region->base) + code_region->size - 4, 0x33, 4);

    _context_init(&new_pcb->tcb.context, (size_t)new_pcb->tcb.stack,
        new_pcb->tcb.stack_sz, (size_t)new_pcb->tcb.task);

	printf("core %d, partition %d: 0x%p, memory code: 0x%p, memory code size: %d, memory data: 0x%p, memory data size %d\n", _cpu_id(),
		new_pcb->status->IDENTIFIER, new_pcb->entry_point ,new_pcb->memory_requirements->memory[CODE].base, new_pcb->memory_requirements->memory[CODE].size, new_pcb->memory_requirements->memory[DATA].base, new_pcb->memory_requirements->memory[DATA].size);
    return new_pcb->status->IDENTIFIER;
}

static struct node_s *find_partition(struct node_s *node, void *arg){
    struct pcb_s *partition = node->data;
    PARTITION_ID_TYPE id = (PARTITION_ID_TYPE) arg;
    
    if(partition->status->IDENTIFIER == id){
        return node;
    }
    
    return 0;
}


int32_t activate_partition(PARTITION_ID_TYPE IDENTIFIER){

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

    kcb->task_current = partition_node;
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
    kcb[_cpu_id()]->task_current = partition_node;    
    kcb[_cpu_id()]->partition_current = partition_node;    
#endif
    
    return IDENTIFIER;
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
       /*out*/ RETURN_CODE_TYPE           *RETURN_CODE ){
#ifndef MULTICORE
    struct pcb_s* my_partition = kcb->task_current->data;
#else
    struct pcb_s* my_partition = kcb[_cpu_id()]->task_current->data;
#endif
    // error
    if(OPERATING_MODE != IDLE && OPERATING_MODE != COLD_START && OPERATING_MODE != WARM_START && OPERATING_MODE != NORMAL){
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    if(OPERATING_MODE == NORMAL && my_partition->status->OPERATING_MODE == NORMAL){
        *RETURN_CODE = NO_ACTION;
        return;
    }

    if(OPERATING_MODE == WARM_START && my_partition->status->OPERATING_MODE == COLD_START){
        *RETURN_CODE = INVALID_MODE;
        return;
    }

    // normal
    my_partition->status->OPERATING_MODE = OPERATING_MODE;
    
    if (OPERATING_MODE == IDLE)
    {   
        printf("OPERATING MODE is IDLE\n");
        signal_idle_current_partition();
        ucx_task_yield();
    }

    if (OPERATING_MODE == WARM_START || OPERATING_MODE == COLD_START)
    {
        // inhibit process scheduling and switch back to initialization mode
        printf("OPERATING MODE is WARM START or COLD START\n");
    }
    
    if (OPERATING_MODE == NORMAL)
    {
        // cf norme
        printf("OPERATING MODE is NORMAL\n");
    }
    
    *RETURN_CODE = NO_ERROR;

}
       
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
