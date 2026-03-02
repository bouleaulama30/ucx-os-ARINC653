#include <ucx.h>

// uint32_t start_time;
// int time_initialized = 0;



void update_kcb_task_list(struct list_s *tcb){
#ifndef MULTICORE
    kcb->tasks = tcb;
    struct node_s *tcb_test_node = kcb->tasks->head->next;
    //a changer mais pour test on met le premier process en tache courante
    // kcb->task_current = tcb_test_node;

#else
    kcb[_cpu_id()]->tasks = tcb;
    struct node_s *tcb_test_node = kcb[_cpu_id()]->tasks->head->next;
    //a changer mais pour test on met le premier process en tache courante
    // kcb[_cpu_id()]->task_current = tcb_test_node;
#endif
    // printf("ID process test %d\n", tcb_test->id);
}

void activate_process_scheduling(){
#ifndef MULTICORE
    struct pcb_s *partition = kcb->partition_current->data;
     // a changer mais temporairement le premier process de la liste est mis en current par le main process
    struct node_s* first_process_node = partition->processes->head->next;  
    kcb->task_current = first_process_node;
#else
    struct pcb_s *partition = kcb[_cpu_id()]->partition_current->data;
     // a changer mais temporairement le premier process de la liste est mis en current par le main process
    struct node_s* first_process_node = partition->processes->head->next;  
    kcb[_cpu_id()]->task_current = first_process_node;
#endif
    partition->last_running_process = first_process_node;
    struct tcb_s* first_process = first_process_node->data;
    // mise a jour de la liste des processes au niveau du kernel
    struct list_s *processes = partition->processes;
    update_kcb_task_list(processes);
    _dispatch_init(first_process->context);
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
    

    setjmp(partition->partition_context);
	if(partition->status->OPERATING_MODE == COLD_START || partition->status->OPERATING_MODE == WARM_START){
        ((void (*)(struct pcb_s *))partition->entry_point)(partition);
	}
    while (1) {
#ifndef MULTICORE
        if (!kcb->tasks->length)
            krnl_panic(ERR_NO_TASKS);
        
        if (!setjmp(partition->partition_context)) {
            // stack_check();
            // list_foreach(kcb->tasks, delay_update, (void *)0);
            krnl_schedule();
            struct node_s *task_node = kcb->task_current;
            partition->last_running_process = task_node;
            struct tcb_s *task = task_node->data;
            _interrupt_tick();
            longjmp(task->context, 1);
        }
#else
        
        if (!kcb[_cpu_id()]->tasks->length)
            krnl_panic(ERR_NO_TASKS);

        if (!setjmp(partition->partition_context)) {
            // stack_check();
            // list_foreach(kcb[_cpu_id()]->tasks, delay_update, (void *)0);
            krnl_schedule();
            struct node_s *task_node = kcb[_cpu_id()]->task_current;
            partition->last_running_process = task_node;
            struct tcb_s *task = task_node->data;
            _interrupt_tick();
            longjmp(task->context, 1);
        }
#endif
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

    // Forcer la tâche courante à redevenir READY
    if (kcb->task_current != NULL) {
        struct tcb_s *t_old = (struct tcb_s *)kcb->task_current->data;
       if (t_old->state == TASK_RUNNING) {
        t_old->state = TASK_READY; 
        }
    }

    kcb->partition_current = partition_node;

    // On restaure le pointeur EXACT de la tâche là où elle s'était arrêtée
    if (partition->last_running_process != NULL) {
        kcb->task_current = partition->last_running_process;
    } else {
    // Premier démarrage de la partition : on pointe sur la tête de liste
        kcb->task_current = partition->processes->head->next;
    }

    // mise a jour de la liste des processes au niveau du kernel
    struct list_s *processes = partition->processes;
    update_kcb_task_list(processes);



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
    // Forcer la tâche courante à redevenir READY
    if (kcb[_cpu_id()]->task_current != NULL) {
        struct tcb_s *t_old = (struct tcb_s *)kcb[_cpu_id()]->task_current->data;
           if (t_old->state == TASK_RUNNING) {
        t_old->state = TASK_READY; 
        }
    }


    kcb[_cpu_id()]->partition_current = partition_node;
    // On restaure le pointeur EXACT de la tâche là où elle s'était arrêtée
    if (partition->last_running_process != NULL) {
        kcb[_cpu_id()]->task_current = partition->last_running_process;
    } else {
    // Premier démarrage de la partition : on pointe sur la tête de liste
        kcb[_cpu_id()]->task_current = partition->processes->head->next;
    }

    // mise a jour de la liste des processes au niveau du kernel
    struct list_s *processes = partition->processes;
    update_kcb_task_list(processes);
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
        *RETURN_CODE = NO_ERROR;
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
