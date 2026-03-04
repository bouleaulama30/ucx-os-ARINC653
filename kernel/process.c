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
#ifndef MULTICORE
    // a changer
	new_tcb->id = kcb->id_next++;
#else
	new_tcb->id = kcb[_cpu_id()]->id_next++;
#endif
	new_tcb->state = TASK_STOPPED;
	new_tcb->priority = TASK_NORMAL_PRIO;
	new_tcb->stack = current_partition->next_stack_addr;
	current_partition->next_stack_addr += stack_size;

	// alignement de la prochaine stack adresse sur 8 octets
	uint32_t addr = (uint32_t) current_partition->next_stack_addr;
    addr = (addr + 7) & ~7;
    current_partition->next_stack_addr = (uint8_t *) addr;
		
	CRITICAL_LEAVE();

	// memset(new_tcb->stack, 0x69, stack_size);
	// memset(new_tcb->stack, 0x33, 4);
	// memset((new_tcb->stack) + stack_size - 4, 0x33, 4);
	
	_context_init(&new_tcb->context, (size_t)new_tcb->stack,
		stack_size, (size_t)task);

	printf("core %d, task %d: 0x%p, stack: 0x%p, size %d\n", _cpu_id(),
		new_tcb->id, new_tcb->task, new_tcb->stack, new_tcb->stack_sz);

	new_tcb->state = TASK_READY;
    process->tcb = *new_tcb;

	return new_tcb->id;
}



uint16_t process_schedule(void)
{
	printf("scheduler native\n");
#ifndef MULTICORE
	struct pcb_s *partition = kcb->partition_current->data;
#else
	struct pcb_s *partition = kcb[_cpu_id()]->partition_current->data;
#endif
	struct process_s *process = partition->process_current->data;
	struct node_s *node, *select;
	struct tcb_s *tselect;
	uint16_t priority;
	
	if (process->tcb.state == TASK_RUNNING)
		process->tcb.state = TASK_READY;

	select = partition->processes->head->next;
	node = partition->processes->head;
	tselect = select->data;

	while ((node = list_next(node))) {
		if (!node->next) break;
		process = node->data;
		if ((process->tcb.priority & 0xff) <= (tselect->priority & 0xff)) {
			if (process->tcb.context == TASK_READY && !process->tcb.rt_prio) {
				select = node;
				tselect = select->data;
			}
		}
	};
	
	tselect = select->data;
	if (tselect->state != TASK_READY || tselect->rt_prio)
		krnl_panic(ERR_NO_TASKS);
	
	priority = tselect->priority;
	node = partition->processes->head;
	while ((node = list_next(node))) {
		if (!node->next) break;
		process = node->data;
		if (process->tcb.state == TASK_READY && !process->tcb.rt_prio)
			process->tcb.priority -= (priority & 0xff);
	};

#ifndef MULTICORE
	partition->process_current = select;
#endif
	tselect->priority |= (tselect->priority >> 8) & 0xff;
	tselect->state = TASK_RUNNING;

	return tselect->id;
}
