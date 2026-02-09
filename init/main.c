/* file:          main.c
 * description:   UCX/OS kernel
 * date:          10/2022
 *                11/2025 (multicore)
 * author:        Sergio Johann Filho <sergio.johann@acad.pucrs.br>
 */

#include <ucx.h>

/* main() function, called from the C runtime */
#ifdef MULTICORE
static volatile int boot = 0;
#endif

int main(void)
{
	uint64_t t_end_main = 0;
	uint64_t t_start_main = ucx_uptime();

    struct tcb_s *task;
	int pr;
	
	_hardware_init();
	
	printf("UCX/OS v%s\n", __VER__);
#ifndef UNKNOWN_HEAP
	ucx_heap_init((size_t *)&_heap_start, (size_t)&_heap_size);
	printf("heap_init() 0x%p - 0x%p, %d bytes free\n",
	(size_t *)&_heap_start, (size_t)&_heap_end, (size_t)&_heap_size);
#else
	ucx_heap_init((size_t *)&__bss_end, ((size_t)&__stack - (size_t)&__bss_end - DEFAULT_STACK_SIZE));
	printf("heap_init() 0x%p - 0x%p, %d bytes free\n",
	(size_t *)&__bss_end, ((size_t)&__stack - DEFAULT_STACK_SIZE),
	((size_t)&__stack - (size_t)&__bss_end - DEFAULT_STACK_SIZE));
#endif

	module_scheduler_init(DEFAULT_MODULE_NAME, DEFAULT_MAJOR_FRAME_TICK, DEFAULT_WINDOWS, DEFAULT_WINDOWS_COUNT);

#ifndef MULTICORE
	kcb->tasks = list_create();
	if (!kcb->tasks)
		krnl_panic(ERR_KCB_ALLOC);
		
	//création de la liste des partitions pour le kernel
	kcb->partitions = list_create();

	if (!kcb->partitions)
		krnl_panic(ERR_KCB_ALLOC);
	
	// initialisation de la partition IDLE dans la ram
	SYSTEM_ADDRESS_TYPE idle_stack = malloc(DEFAULT_STACK_SIZE);
	partition_init(0, 0, IDLE_PARTITION_ID, 1, "IDLE Partition", "IDLE code", idle_task, 0, "RX", "IDLE data", idle_stack, DEFAULT_STACK_SIZE, "RW", idle_task, false);

	pr = app_main();

	setjmp(kcb->context);
	
	if (!kcb->tasks->length)
		krnl_panic(ERR_NO_TASKS);

	kcb->preemptive = pr ? 'y' : 'n';
	arinc_start_scheduling();
#else
	kcb[0]->tasks = list_create();
	
	if (!kcb[0]->tasks)
		krnl_panic(ERR_KCB_ALLOC);
	
	//création de la liste des partitions pour le kernel
	kcb[0]->partitions = list_create();
	
	if (!kcb[0]->partitions)
		krnl_panic(ERR_KCB_ALLOC);

	// initialisation de la partition IDLE dans la ram
	SYSTEM_ADDRESS_TYPE idle_stack = malloc(DEFAULT_STACK_SIZE);
	partition_init(0, 0, IDLE_PARTITION_ID, 1, "IDLE Partition", "IDLE code", idle_task, 0, "RX", "IDLE data", idle_stack, DEFAULT_STACK_SIZE, "RW", idle_task, false);

	pr = app_main();

	setjmp(kcb[0]->context);
	
	if (!kcb[0]->tasks->length)
		krnl_panic(ERR_NO_TASKS);

	kcb[0]->preemptive = pr ? 'y' : 'n';

	printf("core %d ready (%s).\n", _cpu_id(), pr ? "preempt" : "coop");
	
	boot = 1;
	arinc_start_scheduling();
#endif


	
	/* never reached */
	return 0;
}

#ifdef MULTICORE
/* main1() function, called from the C runtime for other cores on SMPs */

int app_main0(void) __attribute__ ((weak, alias ("dummy_app")));
int app_main1(void) __attribute__ ((weak, alias ("dummy_app")));
int app_main2(void) __attribute__ ((weak, alias ("dummy_app")));
int app_main3(void) __attribute__ ((weak, alias ("dummy_app")));
int app_main4(void) __attribute__ ((weak, alias ("dummy_app")));
int app_main5(void) __attribute__ ((weak, alias ("dummy_app")));
int app_main6(void) __attribute__ ((weak, alias ("dummy_app")));
int app_main7(void) __attribute__ ((weak, alias ("dummy_app")));

static int (*const app_vector[])(void) = {
	app_main0,
	app_main1,
	app_main2,
	app_main3,
	app_main4,
	app_main5,
	app_main6,
	app_main7
};

int dummy_app(void)
{
	printf("core %d off.\n", _cpu_id());
	
	while (1) {
		_cpu_idle();
	}
	
	// not reached
	return 0;
}

int main1(void)
{
	struct tcb_s *task;
	int pr = 0;
	
	while (!boot);
	
	_delay_ms(50 * _cpu_id());
	
	if (_cpu_id() < sizeof(app_vector) / sizeof(int *)) {
		kcb[_cpu_id()]->tasks = list_create();
		
		if (!kcb[_cpu_id()]->tasks)
			krnl_panic(ERR_KCB_ALLOC);

		pr = app_vector[_cpu_id()]();
		setjmp(kcb[_cpu_id()]->context);
		
		if (!kcb[_cpu_id()]->tasks->length)
			krnl_panic(ERR_NO_TASKS);

		kcb[_cpu_id()]->preemptive = pr ? 'y' : 'n';
		kcb[_cpu_id()]->task_current = kcb[_cpu_id()]->tasks->head->next;
		task = kcb[_cpu_id()]->task_current->data;
		
		printf("core %d ready (%s).\n", _cpu_id(), pr ? "preempt" : "coop");
		
		_dispatch_init(task->context);
	} else {
		dummy_app();
	}
	
	/* never reached */
	return 0;
}

#else

int main1(void)
{
	return 0;
}

#endif
