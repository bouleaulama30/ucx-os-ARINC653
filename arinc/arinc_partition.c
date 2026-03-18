#include <ucx.h>

static struct node_s *find_highest_priority_process(struct list_s *processes){
	struct node_s *process_node;
    struct node_s *highest_priority_process_node;
	struct process_s *process;
    struct process_s *highest_priority_process;

	process_node = processes->head->next;
    process = process_node->data;
    highest_priority_process_node = process_node;
    highest_priority_process = highest_priority_process_node->data;

	while (process_node->next) {
        if(process->processus_status->PROCESS_STATE == READY){
            if (process->processus_status->CURRENT_PRIORITY > highest_priority_process->processus_status->CURRENT_PRIORITY ){
                highest_priority_process_node = process_node;
                highest_priority_process = highest_priority_process_node->data;
            }	

        }
        process_node = process_node->next;
        process = process_node->data;
    }
	return highest_priority_process_node;
}
static struct node_s *check_and_release_periodic_waiting_processes(struct node_s *node, void *arg)
{
	struct process_s *process = node->data;
    //periodic waiting processes 
    if ((process->processus_status->PROCESS_STATE == WAITING && process->processus_status->ATTRIBUTES.PERIOD != INFINITE_TIME_VALUE)){
        uint64_t current_time = (uint64_t)ucx_uptime();
        uint64_t rp_time = (uint64_t)process->release_point_time;
        printf("check_periodic proc: %d, uptime: %u, release point: %u\n", 
                process->process_id, 
                (unsigned)current_time, 
                (unsigned)rp_time);        
        if(current_time >= rp_time){
            printf("=> REVEIL ! release point time: %u\n", (unsigned)rp_time);
            process->processus_status->PROCESS_STATE = READY;
            // process->processus_status->DEADLINE_TIME = process->release_point_time + process->processus_status->ATTRIBUTES.TIME_CAPACITY;
            // process->release_point_time += process->processus_status->ATTRIBUTES.PERIOD;
        }
    }
    // aperiodic delayed processes started during cold start
    else if(process->processus_status->PROCESS_STATE == WAITING && process->saved_init_delay && process->processus_status->ATTRIBUTES.PERIOD == INFINITE_TIME_VALUE){
        uint64_t current_time = (uint64_t)ucx_uptime();
        uint64_t rp_time = (uint64_t)process->release_point_time;
        printf("check_periodic proc: %d, uptime: %u, release point: %u\n", 
                process->process_id, 
                (unsigned)current_time, 
                (unsigned)rp_time);        
        if(current_time >= rp_time){
            printf("=> REVEIL ! release point time: %u\n", (unsigned)rp_time);
            process->processus_status->PROCESS_STATE = READY;
            process->release_point_time = 0;
            process->saved_init_delay = 0;
        }
    }
	return 0;
}

static struct node_s *check_timeouts(struct node_s *node, void *arg) {
    struct process_s *process = node->data;
    uint64_t current_time = (uint64_t)ucx_uptime();

    // Si le processus  a un chronomètre actif (différent de 0)
    if (process->time_counter != 0) {
        // printf("check_timeouts proc: %d, uptime: %u, time counter: %u\n", 
        //         process->process_id, 
        //         (unsigned)current_time, 
        //         (unsigned)process->time_counter); 
        if (current_time >= process->time_counter) {
                        
            process->is_suspended = false;
            process->time_counter = 0;

            process->processus_status->PROCESS_STATE = READY;
        }
    }
    return 0;
}

static struct node_s *is_at_least_one_ready_process(struct node_s *node, void *arg) {
    struct process_s *process = node->data;
    // Si le processus  a un chronomètre actif (différent de 0)
    if (process->processus_status->PROCESS_STATE == READY || process->processus_status->PROCESS_STATE == RUNNING) {
        return node;
    }
    return 0;
}

void activate_process_scheduling(){
#ifndef MULTICORE
    struct pcb_s *partition = kcb->partition_current->data;
#else
    struct pcb_s *partition = kcb[_cpu_id()]->partition_current->data;
#endif
    list_foreach(partition->processes, check_and_release_periodic_waiting_processes, (void *)0);
    if (list_foreach(partition->processes, is_at_least_one_ready_process, (void *)0) == NULL) {
        printf("[AUCUN PROCESSES READY]\n");
        partition->process_current = NULL;
        longjmp(partition->partition_context, 1); 
    }
    else{

        struct node_s* first_process_node = find_highest_priority_process(partition->processes);  
        struct process_s* first_process = first_process_node->data;

        partition->process_current = first_process_node;
        first_process->processus_status->PROCESS_STATE = RUNNING;
        _dispatch_init(first_process->tcb.context);
    }
}



static void partition_OS(void)
{
    struct pcb_s *partition;
#ifndef MULTICORE
    partition = kcb->partition_current->data;
#else
    partition = kcb[_cpu_id()]->partition_current->data;
#endif
    
    _mprv_activate();
    
start_over:
    setjmp(partition->partition_context);
	if(partition->status->OPERATING_MODE == COLD_START || partition->status->OPERATING_MODE == WARM_START){
        ((void (*)(struct pcb_s *))partition->entry_point)(partition);
	}
    while (1) {
        if(partition->status->OPERATING_MODE == COLD_START || partition->status->OPERATING_MODE == WARM_START){
            goto start_over;
        }
        list_foreach(partition->processes, check_and_release_periodic_waiting_processes, (void *)0);
        list_foreach(partition->processes, check_timeouts, (void *)0);
    
        if (!setjmp(partition->partition_context)) {
            
            // 3. Appel de l'ordonnanceur
            process_schedule();

            // 4. A-t-on un processus à exécuter ?
            if (partition->process_current != NULL && partition->status->OPERATING_MODE == NORMAL) {
                // OUI : On a trouvé un processus READY, on lui donne le CPU !
                struct process_s *process = partition->process_current->data;
                _interrupt_tick_process();
                longjmp(process->tcb.context, 1); // <-- Saute vers le process
            } 
            else {
                // NON : SLACK TIME ! Tous les processus dorment.
                // On NE FAIT PAS de longjmp. On s'endort ici, dans le Kernel.
                
                // printf("[SLACK TIME] wfi...\n"); // Décommentez pour débugger
                
                // On s'assure que les interruptions matérielles sont actives 
                // pour que le Timer Tick puisse nous réveiller
                asm volatile ("csrs mstatus, 8"); 
                
                // Met le CPU RISC-V en basse consommation
                _cpu_idle(); 

                // --- BIIIP (Timer Tick Interrupt) ---
                // Quand l'interruption se termine, le code reprend ici.
                // La boucle while(1) va recommencer, mettre à jour l'Uptime,
                // vérifier les timeouts, et si quelqu'un s'est réveillé, 
                // le prochain process_schedule() le trouvera !
            }
        }
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
// --- 1. GESTION DU CREUX ENTRE LES PARTITIONS (IDLE) ---
    if (IDENTIFIER == IDLE_PARTITION_ID) {
        // On sécurise le système : le PMP bloque tous les accès aux zones mémoires usager
        _pmp_partition_activate((uint32_t)_kernel_end, (uint32_t)0, (uint32_t)0);

// L'IDLE n'est plus une vraie partition, on coupe juste le pointeur
#ifndef MULTICORE
        kcb->partition_current = NULL;
#else
        kcb[_cpu_id()]->partition_current = NULL;
#endif
        return IDLE_PARTITION_ID;
    }

    // --- 2. GESTION DES PARTITIONS USAGER NORMALES ---
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


void GET_PARTITION_STATUS (
    /*out*/ PARTITION_STATUS_TYPE      *PARTITION_STATUS,
    /*out*/ RETURN_CODE_TYPE           *RETURN_CODE ){
#ifndef MULTICORE
    struct pcb_s* my_partition = kcb->partition_current->data;
#else
    struct pcb_s* my_partition = kcb[_cpu_id()]->partition_current->data;
#endif
    
    if (!my_partition->status) {
        *RETURN_CODE = NOT_AVAILABLE;
        return;
    }

    *PARTITION_STATUS = *(my_partition->status);
    *RETURN_CODE = NOT_AVAILABLE;       
}



static struct node_s *start_process(struct node_s *node, void *arg)
{
	struct process_s *process = node->data;
	SYSTEM_TIME_TYPE first_release_point = (SYSTEM_TIME_TYPE) arg;

    // set to READY all previously started (not delayed) aperiodic processes (unless the process was suspended)
	if (process->processus_status->PROCESS_STATE == WAITING && !process->saved_init_delay && process->processus_status->ATTRIBUTES.PERIOD == INFINITE_TIME_VALUE && !process->is_suspended){
        process->processus_status->PROCESS_STATE = READY;
        // calculate the DEADLINE_TIME of all non-dormant processes in the partition;
        process->processus_status->DEADLINE_TIME = ucx_uptime() + process->processus_status->ATTRIBUTES.TIME_CAPACITY;
        return 0;
    }

    // set release point of all previously delay started aperiodic processes to the system clock time plus their delay times;
	else if (process->processus_status->PROCESS_STATE == WAITING && process->saved_init_delay && process->processus_status->ATTRIBUTES.PERIOD == INFINITE_TIME_VALUE){
        process->release_point_time = ucx_uptime() + process->saved_init_delay;
    }

    // set first release points of all previously started (not delayed) periodic processes to the partition’s next periodic processing start
    else if (process->processus_status->PROCESS_STATE == WAITING && !process->saved_init_delay &&process->processus_status->ATTRIBUTES.PERIOD != INFINITE_TIME_VALUE){
        process->release_point_time = first_release_point;
    }

    // set first release points of all previously delay started periodic processes to the partition’s next periodic processing start plus their delay times;
    else if (process->processus_status->PROCESS_STATE == WAITING && process->saved_init_delay && process->processus_status->ATTRIBUTES.PERIOD != INFINITE_TIME_VALUE){
        process->release_point_time = first_release_point + process->saved_init_delay;
    }

    // calculate the DEADLINE_TIME of all non-dormant processes in the partition;
    if(process->processus_status->PROCESS_STATE != DORMANT){    
        process->processus_status->DEADLINE_TIME =  process->release_point_time + process->processus_status->ATTRIBUTES.TIME_CAPACITY;
        return 0;
    }
}


void SET_PARTITION_MODE (
       /*in */ OPERATING_MODE_TYPE        OPERATING_MODE,
       /*out*/ RETURN_CODE_TYPE           *RETURN_CODE ){
#ifndef MULTICORE
    struct pcb_s* my_partition = kcb->partition_current->data;
#else
    struct pcb_s* my_partition = kcb[_cpu_id()]->partition_current->data;
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
        my_partition->process_current = NULL;
        signal_idle_current_partition();
        longjmp(my_partition->partition_context, 1);
        // ucx_task_yield();
    }

    if (OPERATING_MODE == WARM_START || OPERATING_MODE == COLD_START)
    {
        // inhibit process scheduling and switch back to initialization mode
        printf("OPERATING MODE is WARM START or COLD START\n");
        my_partition->process_current = NULL;
        my_partition->nbr_processes = 0;
        my_partition->id_next = 0;
        my_partition->next_stack_addr = my_partition->memory_requirements->memory[DATA].base + PARTIION_OS_AND_MAIN_PROCESS_STACK_SIZE;

        // on vide la liste des processes de la partition
        while (my_partition->processes->length != 0)
        {
            struct process_s *process  = list_pop(my_partition->processes);
            free(process->processus_status);
            free(process);
        }
        *RETURN_CODE = NO_ERROR;
        longjmp(my_partition->partition_context, 1);
    }
    
    if (OPERATING_MODE == NORMAL)
    {
        // cf norme
        printf("OPERATING MODE is NORMAL\n");
        //set all processes to ready state
        *RETURN_CODE = NO_ERROR;

        SYSTEM_TIME_TYPE first_release_point = find_first_release_point(my_partition);
        list_foreach(my_partition->processes, start_process, (void *)first_release_point);
        activate_process_scheduling();
    }
    
    *RETURN_CODE = NO_ERROR;

}
       
void GET_MY_PARTITION_ID(
               /*out*/ PARTITION_ID_TYPE          *PARTITION_ID,
              /*out*/ RETURN_CODE_TYPE           *RETURN_CODE )
{
#ifndef MULTICORE
    struct pcb_s* my_partition = kcb->partition_current->data;
#else
    struct pcb_s* my_partition = kcb[_cpu_id()]->partition_current->data;
#endif
    *PARTITION_ID = my_partition->status->IDENTIFIER;
    if(*PARTITION_ID)
        *RETURN_CODE = NO_ERROR;
    else
        *RETURN_CODE = NOT_AVAILABLE;       
}
