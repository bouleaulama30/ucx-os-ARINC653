#include "ucx.h"


int32_t ucx_process_spawn(void *task, uint16_t stack_size, struct process_s *process, struct pcb_s *current_partition, int is_error_handler)
{
	struct tcb_s *new_tcb;
	struct node_s *new_task;

	new_tcb = malloc(sizeof(struct tcb_s));
		
	if (!new_tcb)
		krnl_panic(ERR_TCB_ALLOC);

	CRITICAL_ENTER();
    

	new_tcb->task = task;
	new_tcb->rt_prio = 0;
	new_tcb->delay = 0;
	new_tcb->stack_sz = stack_size;

    if (!is_error_handler) {
        current_partition->id_next++;
        new_tcb->id = current_partition->id_next;
    }

	new_tcb->stack = current_partition->next_stack_addr;
	current_partition->next_stack_addr += stack_size;

	// alignement de la prochaine stack adresse sur 8 octets
	size_t addr = (size_t) current_partition->next_stack_addr;
    addr = (addr + 7) & ~7;
    current_partition->next_stack_addr = (uint8_t *) addr;
		
	CRITICAL_LEAVE();
	
	// printf("core %d, task %d: 0x%p, stack: 0x%p, size %d\n", _cpu_id(),
	// 	new_tcb->id, new_tcb->task, new_tcb->stack, new_tcb->stack_sz);

    process->tcb = *new_tcb;

	return new_tcb->id;
}

uint16_t process_schedule(void)
{
    struct pcb_s *partition = get_current_partition();

    struct node_s *node;
    
    struct node_s *select = NULL;
    struct process_s *pselect = NULL;
    int32_t highest_priority = -1;
    
    if (partition->process_current != NULL) {
        struct process_s *current_process = partition->process_current->data;
        if (current_process->processus_status->PROCESS_STATE == RUNNING) {
            current_process->processus_status->PROCESS_STATE = READY;
        }
    }

    // 2. PARCOURS UNIQUE : On cherche strictement le processus READY avec le plus grand chiffre
    node = partition->processes->head;
    while ((node = list_next(node))) {
        if (!node->next) break; 
        
        struct process_s *process = node->data;        

        // Si le processus est prêt à s'exécuter
        if (process->processus_status->PROCESS_STATE == READY && !process->tcb.rt_prio) {
            
            if (process->processus_status->CURRENT_PRIORITY > highest_priority) {
                highest_priority = process->processus_status->CURRENT_PRIORITY;
                select = node;
                pselect = process;
            }
        }
    };
    
    // 3. Sécurité : Vérifier si on a trouvé au moins un processus READY
    if (select == NULL || pselect == NULL) {
        partition->process_current = NULL;
        return 0;
    }

    // 4. Mise à jour du noyau
    partition->process_current = select;
    pselect->processus_status->PROCESS_STATE = RUNNING;

    return pselect->tcb.id;
}