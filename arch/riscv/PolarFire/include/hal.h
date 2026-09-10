#ifndef HAL_H
#define HAL_H

#include <stdint.h>
#include <stddef.h>

#ifndef __VER__
#define __VER__ "1.1.1-mpfs"
#endif

/* Linker symbols expected by UCX/OS */
extern uint32_t _stack_start;
extern uint32_t _stack_end;
extern uint32_t _stack;
extern uint32_t _heap_start;
extern uint32_t _heap_end;
extern uint32_t _heap_size;
extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;
extern uint32_t _end;
extern uint32_t _gp;

/* Target identity */
#define __ARCH__ "RV64 (PolarFire SoC U54_2)"

/* Scheduling / console configuration */
#ifndef USART_BAUD
#define USART_BAUD 115200u
#endif

#ifndef F_TIMER
#define F_TIMER    100u
#endif

/* CSR / trap bits */
#define MSTATUS_MIE_BIT      0x8UL
#define MIE_MTIE_BIT         0x80UL
#define MCAUSE_INTERRUPT_BIT (1ULL << 63)
#define MCAUSE_CODE_MASK     0x7FFULL

/* Interrupt helpers used by UCX */
#define _di() _interrupt_set(0)
#define _ei() _interrupt_set(1)

/* CSR helpers */
#define read_csr(reg) ({ \
    unsigned long __tmp; \
    __asm__ volatile ("csrr %0, " #reg : "=r"(__tmp)); \
    __tmp; \
})

#define write_csr(reg, val) do { \
    unsigned long __v = (unsigned long)(val); \
    __asm__ volatile ("csrw " #reg ", %0" :: "rK"(__v)); \
} while (0)

/* UCX context layout for RV64 setjmp/longjmp */
#define CONTEXT_SP 14
#define CONTEXT_RA 15
typedef uint64_t jmp_buf[20];

/* HAL API expected by UCX */
int32_t _interrupt_set(int32_t s);
int32_t setjmp(jmp_buf env);
void longjmp(jmp_buf env, int32_t val);
void _dispatch_init(jmp_buf env);
void _delay_ms(uint32_t msec);
void _delay_us(uint32_t usec);
uint64_t _read_us(void);
void _cpu_idle(void);
uint16_t _cpu_id(void);
void _hardware_init(void);
void _timer_enable(void);
void _timer_disable(void);
void _interrupt_tick(void);
void _interrupt_tick_partition(void);
void _interrupt_tick_process(void);
void _context_init(jmp_buf *ctx, size_t sp, size_t ss, size_t ra);

/* Called by trap entry */
void _irq_handler(uint64_t cause, uint64_t epc);

/* Default task stack size for this target */
#define DEFAULT_STACK_SIZE 4096u

/* Use UCX libc/lib wrappers */
#define strcpy(dst, src)            ucx_strcpy(dst, src)
#define strncpy(s1, s2, n)          ucx_strncpy(s1, s2, n)
#define strcat(dst, src)            ucx_strcat(dst, src)
#define strncat(dst, src, n)        ucx_strncat(dst, src, n)
#define strcmp(s1, s2)              ucx_strcmp(s1, s2)
#define strncmp(s1, s2, n)          ucx_strncmp(s1, s2, n)
#define strstr(string, find)        ucx_strstr(string, find)
#define strlen(s)                   ucx_strlen(s)
#define strchr(s, c)                ucx_strchr(s, c)
#define strpbrk(str, set)           ucx_strpbrk(str, set)
#define strsep(pp, delim)           ucx_strsep(pp, delim)
#define strtok(s, delim)            ucx_strtok(s, delim)
#define strtok_r(s, delim, holder)  ucx_strtok_r(s, delim, holder)
#define strtol(s, end, base)        ucx_strtol(s, end, base)
#define atoi(s)                     ucx_atoi(s)
#define itoa(i, s, base)            ucx_itoa(i, s, base)
#define memcpy(dst, src, n)         ucx_memcpy(dst, src, n)
#define memmove(dst, src, n)        ucx_memmove(dst, src, n)
#define memcmp(cs, ct, n)           ucx_memcmp(cs, ct, n)
#define memset(s, c, n)             ucx_memset(s, c, n)
#define abs(n)                      ucx_abs(n)
#define random()                    ucx_random()
#define srand(seed)                 ucx_srand(seed)
#define puts(str)                   ucx_puts(str)
#define gets(s)                     ucx_gets(s)
#define fgets(s, n, f)              ucx_fgets(s, n, f)
#define getline(s)                  ucx_getline(s)
#define printf(fmt, ...)            ucx_printf(fmt, ##__VA_ARGS__)
#define sprintf(out, fmt, ...)      ucx_sprintf(out, fmt, ##__VA_ARGS__)
#define malloc(n)                   ucx_malloc(n)
#define free(n)                     ucx_free(n)
#define calloc(n, t)                ucx_calloc(n, t)
#define realloc(p, s)               ucx_realloc(p, s)

void krnl_dispatcher(void);

#endif
