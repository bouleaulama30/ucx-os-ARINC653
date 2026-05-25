#include <hal.h>

#include <lib/console.h>
#include <lib/list.h>
#include <kernel/kernel.h>
#include <kernel/hm.h>
#include <arinc/arinc_partition.h>
#include <arinc/arinc_HM.h>
#include "mpfs_uart.h"
#include "mpfs_clint.h"

volatile uint64_t g_last_trap_cause = 0;
volatile uint64_t g_last_trap_epc   = 0;
volatile uint64_t g_last_trap_tval  = 0;

static int __putchar(int value)
{
    return mpfs_uart_putc(value);
}

static int __kbhit(void)
{
    return mpfs_uart_kbhit();
}

static int __getchar(void)
{
    int ch;

    do {
        ch = mpfs_uart_getc_nonblock();
    } while (ch < 0);

    return ch;
}

int32_t _interrupt_set(int32_t s)
{
    static int32_t int_status = 0;

    if (s) {
        int_status++;
    } else {
        int_status--;
    }

    if (int_status < 0) {
        __asm__ volatile ("csrci mstatus, 8");
    } else {
        __asm__ volatile ("csrsi mstatus, 8");
    }

    return int_status;
}

void _delay_ms(uint32_t msec)
{
    const uint64_t start = _read_us();
    const uint64_t target = (uint64_t)msec * 1000ULL;

    while ((_read_us() - start) < target) {
        ;
    }
}

void _delay_us(uint32_t usec)
{
    const uint64_t start = _read_us();

    while ((_read_us() - start) < (uint64_t)usec) {
        ;
    }
}

void _irq_handler(uint64_t cause, uint64_t epc)
{
    const uint64_t cause_code = (cause & MCAUSE_CODE_MASK);

    g_last_trap_cause = cause;
    g_last_trap_epc   = epc;
    g_last_trap_tval  = read_csr(mtval);

    /* Machine timer interrupt */
    if (((cause & MCAUSE_INTERRUPT_BIT) != 0ULL) && (cause_code == 7ULL)) {
        mpfs_clint_arm_tick(_cpu_id(), F_TIMER);

#ifndef MULTICORE
        if (kcb->partition_current != NULL) {
            // On sauve le contexte uniquement si on interrompt une vraie partition
            struct pcb_s *current_partition = kcb->partition_current->data;
            if (setjmp(current_partition->tcb.context) == 0) {
                longjmp(kcb->context, 1);
            }
        } else {
            // Si on était en IDLE, on relance juste le main pour vérifier le planning !
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
    } else {

    printf("\nTRAP cause=0x%lx epc=0x%lx mtval=0x%lx\n",
           (unsigned long)g_last_trap_cause,
           (unsigned long)g_last_trap_epc,
           (unsigned long)g_last_trap_tval);

    write_csr(mie, 0);
    __asm__ volatile ("csrci mstatus, 8");

    while (1) {
        __asm__ volatile ("wfi");
    }
    }
}

uint64_t _read_us(void)
{
    return (mpfs_clint_mtime_read() / (uint64_t)(MPFS_CLINT_TIMEBASE_HZ / 1000000UL));
}

void _cpu_idle(void)
{
    __asm__ volatile ("wfi");
}

uint16_t _cpu_id(void)
{
    return (uint16_t)read_csr(mhartid);
}

void _hardware_init(void)
{
    mpfs_uart_init(USART_BAUD);

    _stdout_install(__putchar);
    _stdin_install(__getchar);
    _stdpoll_install(__kbhit);
}

void _timer_enable(void)
{
    const uint64_t mie_reg = read_csr(mie);
    write_csr(mie, (mie_reg | MIE_MTIE_BIT));
}

void _timer_disable(void)
{
    const uint64_t mie_reg = read_csr(mie);
    write_csr(mie, (mie_reg & ~((uint64_t)MIE_MTIE_BIT)));
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
		asm volatile ("csrs mstatus, 8");
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
	/* partition is run for the first time */
	struct process_s *process = partition->process_current->data;
	if ((uint32_t)process->tcb.task == process->tcb.context[CONTEXT_RA])
		asm volatile ("csrs mstatus, 8");
}


void _interrupt_tick(void)
{
    struct tcb_s *task = (struct tcb_s *)kcb->task_current->data;

    /*
     * Match the UCX RV64 port behavior:
     * when a task runs for the first time, allow global interrupts.
     */
    if ((uint64_t)task->task == task->context[CONTEXT_RA]) {
        __asm__ volatile ("csrsi mstatus, 8");
    }
}

extern void __dispatch_init(jmp_buf env);

void _dispatch_init(jmp_buf env)
{
    if (kcb->preemptive == 'y') {
        mpfs_clint_arm_tick(_cpu_id(), F_TIMER);
        _timer_enable();
    }

    __dispatch_init(env);
}

void _context_init(jmp_buf *ctx, size_t sp, size_t ss, size_t ra)
{
    uint64_t *ctx_p = (uint64_t *)ctx;

    ctx_p[CONTEXT_SP] = (uint64_t)(sp + ss);
    ctx_p[CONTEXT_RA] = (uint64_t)ra;
}


// void _pmp_init(uint32_t end_addr){

// 	uint32_t pmpaddr0 = end_addr >> 2;

// 	w_pmpaddr0(pmpaddr0);

// 	uint8_t pmp0cfg = 0b00001111;
// 	uint32_t pmpcfg0 = pmp0cfg;
// 	w_pmpcfg0(pmpcfg0);
// }

// void _pmp_partition_activate(uint32_t kernel_end_addr, uint32_t partition_start_addr, uint32_t partition_end_addr){

// 	uint32_t pmpaddr0 = kernel_end_addr >> 2;
// 	uint32_t pmpaddr1 = partition_start_addr >> 2;
// 	uint32_t pmpaddr2 = partition_end_addr >> 2;

// 	w_pmpaddr0(pmpaddr0);
// 	w_pmpaddr1(pmpaddr1);
// 	w_pmpaddr2(pmpaddr2);



// 	uint8_t pmp0cfg = 0b00001111;
// 	uint8_t pmp2cfg = 0b00001111;
// 	uint32_t pmpcfg0 = (pmp2cfg << 16) | pmp0cfg;
// 	w_pmpcfg0(pmpcfg0);
// }

// void _mprv_activate(){
// 	uint32_t mstatus = r_mstatus();

// 	// mettre MPP en mode user
// 	mstatus &= ~0x1800;
// 	mstatus |= (1 << 17);

// 	w_mstatus(mstatus);
// }