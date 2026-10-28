/* file:          hal.c
 * description:   hardware abstraction layer for HF-RISC
 * date:          04/2021
 * author:        Sergio Johann Filho <sergio.johann@acad.pucrs.br>
 */

#include <hal.h>
#include <console.h>
#include <lib/libc.h>
#include <lib/list.h>
#include <kernel/kernel.h>
#include <kernel/hm.h>
#include <arinc/arinc_partition.h>
#include <arinc/arinc_HM.h>
#include <device.h>
#include <uart.h>
#include <usart.h>

/*
libc basic I/O support
*/

const struct device_s *usart0 = &usart0_dev;

#ifndef DEBUG_PORT
static int __putchar(int value)		// polled putchar()
{
	usart0->api->dev_write(usart0, &value, 1);
	
	return value;
}

static int __kbhit(void)
{
	if (usart0->api->dev_read(usart0, 0, 0))
		return 1;
	else
		return 0;
}

static int __getchar(void)			// polled getch()
{
	char buf[2];
	
	usart0->api->dev_read(usart0, buf, 1);
	
	return buf[0];
}
#else
static int __putchar(int value)		// polled putchar()
{
	DEBUG_ADDR = value;
	
	return value;
}

static int __kbhit(void)
{
	return 0;
}

static int __getchar(void)			// polled getch()
{
	return DEBUG_ADDR;
}
#endif


/* delay routines */

void _delay_ms(uint32_t msec)
{
	volatile uint32_t cur, last, delta, msecs;
	uint32_t cycles_per_msec;

	last = TIMER0;
	delta = msecs = 0;
	cycles_per_msec = F_CPU / 1000;
	while (msec > msecs) {
		cur = TIMER0;
		if (cur < last)
			delta += (cur + (F_CPU - last));
		else
			delta += (cur - last);
		last = cur;
		if (delta >= cycles_per_msec) {
			msecs += delta / cycles_per_msec;
			delta %= cycles_per_msec;
		}
	}
}

void _delay_us(uint32_t usec)
{
	volatile uint32_t cur, last, delta, usecs;
	uint32_t cycles_per_usec;

	last = TIMER0;
	delta = usecs = 0;
	cycles_per_usec = F_CPU / 1000000;
	while (usec > usecs) {
		cur = TIMER0;
		if (cur < last)
			delta += (cur + (F_CPU - last));
		else
			delta += (cur - last);
		last = cur;
		if (delta >= cycles_per_usec) {
			usecs += delta / cycles_per_usec;
			delta %= cycles_per_usec;
		}
	}
}

uint64_t _read_us(void)
{
	static uint64_t timeref = 0;
	static uint32_t tval2 = 0, tref = 0;

	if (tref == 0) TIMER0;
	if (TIMER0 < tref) tval2++;
	tref = TIMER0;
	timeref = ((uint64_t)tval2 << 32) + (uint64_t)TIMER0;

	return (timeref / (F_CPU / 1000000));
}

void _cpu_idle(void)
{
}

void _panic(void)
{
	while (1);
}

uint16_t _cpu_id(void)
{
	return 0;
}

/* kernel auxiliary routines */

static void _arinc_timer_tick(void)
{
#ifndef MULTICORE
	if (kcb->partition_current != NULL) {
		struct pcb_s *current_partition = kcb->partition_current->data;
		if (setjmp(current_partition->tcb.context) == 0) {
			longjmp(kcb->context, 1);
		}
	} else {
		longjmp(kcb->context, 1);
	}
#else
	int core_id = _cpu_id();
	if (kcb[core_id]->partition_current != NULL) {
		struct pcb_s *current_partition = kcb[core_id]->partition_current->data;
		if (setjmp(current_partition->tcb.context) == 0) {
			longjmp(kcb[core_id]->context, 1);
		}
	} else {
		longjmp(kcb[core_id]->context, 1);
	}
#endif
}

#if F_TIMER > 0
void timer1ctc_handler(void)
{
	_arinc_timer_tick();
}

#else

void timer0b_handler(void)
{
	_arinc_timer_tick();
}
#endif

void _hardware_init(void)
{
	_interrupt_set(0);
	
#ifndef DEBUG_PORT
	usart0->api->dev_init(usart0);
	usart0->api->dev_open(usart0, 0);
#endif
	_stdout_install(__putchar);
	_stdin_install(__getchar);
	_stdpoll_install(__kbhit);

#if F_TIMER > 0
	TIMER1PRE = TIMERPRE_DIV16;

	TIMER1 = 0;

	/* TIMER1 frequency: 100 interrupts/s (every 250000 cycles, 10ms timer @ 25MHz) */
	TIMER1CTC = (F_CPU / F_TIMER) / 16;
#endif
}

int _int_set(int s)
{
	static int int_status = 0;

	(!s) ? int_status-- : int_status++;
	(int_status < 0) ? _interrupt_set(0) : _interrupt_set(1);

	return int_status;
}

void _timer_enable(void)
{
#if F_TIMER > 0
	TIMERMASK |= MASK_TIMER1CTC;		/* enable interrupt mask for TIMER1 CTC events */
#else
	TIMERMASK |= MASK_TIMER0B;		/* enable interrupt mask for TIMER0B events */
#endif
}

void _timer_disable(void)
{
#if F_TIMER > 0
	TIMERMASK &= ~MASK_TIMER1CTC;		/* disable interrupt mask for TIMER1 CTC events */
#else
	TIMERMASK &= ~MASK_TIMER0B;		/* disable interrupt mask for TIMER0B events */
#endif
}

void _interrupt_tick(void)
{
	struct tcb_s *task = kcb->task_current->data;
	
	_read_us();
	/* task is run for the first time */
	if ((uint32_t)task->task == task->context[CONTEXT_RA])
		_interrupt_set(1);
}

extern void __dispatch_init(jmp_buf env);

void _dispatch_init(jmp_buf env)
{
	if ((kcb->preemptive == 'y'))
		_timer_enable();
	
	__dispatch_init(env);
}

void _context_init(jmp_buf *ctx, size_t sp, size_t ss, size_t ra)
{
	uint32_t *ctx_p;
	
	ctx_p = (uint32_t *)ctx;
	
	ctx_p[CONTEXT_SP] = sp + ss;
	ctx_p[CONTEXT_RA] = ra;
}

void _interrupt_tick_partition(void)
{
#ifndef MULTICORE
	if (kcb->partition_current == NULL) return;
	struct pcb_s *partition = kcb->partition_current->data;
#else
	if (kcb[_cpu_id()]->partition_current == NULL) return;
	struct pcb_s *partition = kcb[_cpu_id()]->partition_current->data;
#endif
	_read_us();
	/* partition is run for the first time */
	if ((uint32_t)partition->tcb.task == partition->tcb.context[CONTEXT_RA])
		_interrupt_set(1);
}

void _interrupt_tick_process(void)
{
#ifndef MULTICORE
	if (kcb->partition_current == NULL) return;
	struct pcb_s *partition = kcb->partition_current->data;
#else
	if (kcb[_cpu_id()]->partition_current == NULL) return;
	struct pcb_s *partition = kcb[_cpu_id()]->partition_current->data;
#endif
	_read_us();
	/* process is run for the first time */
	struct process_s *process = partition->process_current->data;
	if ((uint32_t)process->tcb.task == process->tcb.context[CONTEXT_RA])
		_interrupt_set(1);
}

void _pmp_init(uint32_t end_addr)
{
	(void)end_addr;
}

void _pmp_partition_activate(uint32_t kernel_end_addr, uint32_t partition_start_addr, uint32_t partition_end_addr)
{
	(void)kernel_end_addr;
	(void)partition_start_addr;
	(void)partition_end_addr;
}

void _mprv_activate(void)
{
}
