#include "ucx.h"


int32_t ucx_process_spawn(void *task, uint16_t stack_size, struct process_s *process, struct pcb_s *current_partition)
{
	struct tcb_s *new_tcb;
	struct node_s *new_task;

    // a changer
	new_tcb = malloc(sizeof(struct tcb_s));
		
	if (!new_tcb)
		krnl_panic(ERR_TCB_ALLOC);

	CRITICAL_ENTER();
    

	new_tcb->task = task;
	new_tcb->rt_prio = 0;
	new_tcb->delay = 0;
	new_tcb->stack_sz = stack_size;
	new_tcb->id = current_partition->id_next++;

	//pas necessaire de definir l'etat car deja dans status du process
	new_tcb->state = 0;

	// new_tcb->priority = ((process->processus_status->CURRENT_PRIORITY << 8) | process->processus_status->CURRENT_PRIORITY);
	new_tcb->stack = current_partition->next_stack_addr;
	current_partition->next_stack_addr += stack_size;

	// alignement de la prochaine stack adresse sur 8 octets
	uint32_t addr = (uint32_t) current_partition->next_stack_addr;
    addr = (addr + 7) & ~7;
    current_partition->next_stack_addr = (uint8_t *) addr;
		
	CRITICAL_LEAVE();
	
	_context_init(&new_tcb->context, (size_t)new_tcb->stack,
		stack_size, (size_t)task);

	printf("core %d, task %d: 0x%p, stack: 0x%p, size %d\n", _cpu_id(),
		new_tcb->id, new_tcb->task, new_tcb->stack, new_tcb->stack_sz);

    process->tcb = *new_tcb;

	return new_tcb->id;
}

uint16_t process_schedule(void)
{
    // printf("scheduler native\n"); // Désactivé pour les perfs
#ifndef MULTICORE
    struct pcb_s *partition = kcb->partition_current->data;
#else
    struct pcb_s *partition = kcb[_cpu_id()]->partition_current->data;
#endif

    struct process_s *process = partition->process_current->data;
    struct node_s *node;
    
    struct node_s *select = NULL;
    struct process_s *pselect = NULL;
    int32_t highest_priority = -1; // On part de la priorité la plus basse possible
    
    // 1. Le processus courant (s'il existe) repasse en READY
    if (process->processus_status->PROCESS_STATE == RUNNING)
        process->processus_status->PROCESS_STATE = READY;

    // 2. PARCOURS UNIQUE : On cherche strictement le processus READY avec le plus grand chiffre
    node = partition->processes->head;
    while ((node = list_next(node))) {
        if (!node->next) break; // Sécurité pour la liste UCX-OS
        
        process = node->data;
        
        // Si le processus est prêt à s'exécuter
        if (process->processus_status->PROCESS_STATE == READY && !process->tcb.rt_prio) {
            
            // CORRECTION ARINC 653 : 
            // On utilise ">" car on cherche la plus Grosse valeur.
            // On utilise directement la variable de la norme (CURRENT_PRIORITY) sans faire de & 0xff
            if (process->processus_status->CURRENT_PRIORITY > highest_priority) {
                highest_priority = process->processus_status->CURRENT_PRIORITY;
                select = node;
                pselect = process;
            }
        }
    };
    
    // 3. Sécurité : Vérifier si on a trouvé au moins un processus READY
    if (select == NULL || pselect == NULL) {
        // En vrai ARINC 653, s'il n'y a rien, on devrait exécuter le processus "IDLE" de la partition.
        // Pour l'instant, on laisse le panic.
        krnl_panic(ERR_NO_TASKS);
    }
    
    // SUPPRESSION DE LA 2EME BOUCLE : 
    // En ARINC 653, la priorité est statique ! On ne soustrait rien aux perdants !

    // 4. Mise à jour du noyau
    partition->process_current = select;
    pselect->processus_status->PROCESS_STATE = RUNNING;

    return pselect->tcb.id;
}


// uint16_t process_schedule(void)
// {
// 	printf("scheduler native\n");
// #ifndef MULTICORE
// 	struct pcb_s *partition = kcb->partition_current->data;
// #else
// 	struct pcb_s *partition = kcb[_cpu_id()]->partition_current->data;
// #endif
// 	struct process_s *process = partition->process_current->data;
// 	struct node_s *node, *select;
// 	struct process_s *tselect;
// 	uint16_t priority;
	
// 	if (process->processus_status->PROCESS_STATE == RUNNING)
// 		process->processus_status->PROCESS_STATE = READY;

// 	select = partition->processes->head->next;
// 	node = partition->processes->head;
// 	tselect = select->data;

// 	while ((node = list_next(node))) {
// 		if (!node->next) break;
// 		process = node->data;
// 		if ((process->tcb.priority & 0xff) <= (tselect->tcb.priority & 0xff)) {
// 			if (process->processus_status->PROCESS_STATE == READY && !process->tcb.rt_prio) {
// 				select = node;
// 				tselect = select->data;
// 			}
// 		}
// 	};
	
// 	tselect = select->data;
// 	if (tselect->processus_status->PROCESS_STATE != READY || tselect->tcb.rt_prio)
// 		krnl_panic(ERR_NO_TASKS);
	
// 	priority = tselect->tcb.priority;
// 	node = partition->processes->head;
// 	while ((node = list_next(node))) {
// 		if (!node->next) break;
// 		process = node->data;
// 		if (process->processus_status->PROCESS_STATE == READY && !process->tcb.rt_prio)
// 			process->tcb.priority -= (priority & 0xff);
// 	};

// 	partition->process_current = select;
// 	tselect->tcb.priority |= (tselect->tcb.priority >> 8) & 0xff;
// 	tselect->processus_status->PROCESS_STATE = RUNNING;

// 	return tselect->tcb.id;
// }
