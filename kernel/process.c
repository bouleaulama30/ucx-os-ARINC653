#include "ucx.h"


int32_t ucx_process_spawn(void *task, uint16_t stack_size, struct process_s *process)
{
	struct tcb_s *new_tcb;
	struct node_s *new_task;

    // a changer
	new_tcb = malloc(sizeof(struct tcb_s));
		
	if (!new_tcb)
		krnl_panic(ERR_TCB_ALLOC);

	CRITICAL_ENTER();
    
// #ifndef MULTICORE
// 	new_task = list_pushback(kcb->tasks, new_tcb);
// #else
// 	new_task = list_pushback(kcb[_cpu_id()]->tasks, new_tcb);
// #endif
	
	// if (!new_task)
	// 	krnl_panic(ERR_TCB_ALLOC);
	
	// new_task->data = new_tcb;
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
	new_tcb->stack = malloc(stack_size);
		
	if (!new_tcb->stack)
		krnl_panic(ERR_STACK_ALLOC);

    process->tcb = *new_tcb;

	CRITICAL_LEAVE();

	memset(new_tcb->stack, 0x69, stack_size);
	memset(new_tcb->stack, 0x33, 4);
	memset((new_tcb->stack) + stack_size - 4, 0x33, 4);
	
	_context_init(&new_tcb->context, (size_t)new_tcb->stack,
		stack_size, (size_t)task);

	printf("core %d, task %d: 0x%p, stack: 0x%p, size %d\n", _cpu_id(),
		new_tcb->id, new_tcb->task, new_tcb->stack, new_tcb->stack_sz);

	new_tcb->state = TASK_READY;

	return new_tcb->id;
}
