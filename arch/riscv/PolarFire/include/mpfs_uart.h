#ifndef MPFS_UART_H
#define MPFS_UART_H

#include <stdint.h>
#include "mpfs_memory_map.h"

/*
 * Default UART selection and input clock.
 */
#ifndef MPFS_UART_BASE
#define MPFS_UART_BASE         MMUART4_LO_BASE //MMUART0_LO_BASE
#endif

#ifndef MPFS_UART_INPUT_CLK_HZ
#define MPFS_UART_INPUT_CLK_HZ 150000000UL
#endif

void mpfs_uart_init(uint32_t baud);
int  mpfs_uart_putc(int ch);
int  mpfs_uart_kbhit(void);
int  mpfs_uart_getc_nonblock(void);

#endif
