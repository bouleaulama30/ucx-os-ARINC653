#include "mpfs_uart.h"

#define UART_REG(ofs_word) \
    (*(volatile uint32_t *)(MPFS_UART_BASE + (((uint32_t)(ofs_word)) << 2)))

void mpfs_uart_init(uint32_t baud)
{
    uint32_t divisor;

    if (baud == 0u) {
        baud = 115200u;
    }

    divisor = (MPFS_UART_INPUT_CLK_HZ + (8u * baud)) / (16u * baud);
    if (divisor == 0u) {
        divisor = 1u;
    }

    /* Enter divisor latch access mode */
    UART_REG(MMUART_LCR_OFS) = MMUART_LCR_DLAB;

    /* Program baud divisor */
    UART_REG(MMUART_RBR_THR_DLL_OFS) = (divisor & 0xFFu);
    UART_REG(MMUART_IER_DLM_OFS)     = ((divisor >> 8) & 0xFFu);

    /* 8 data bits, 1 stop bit, no parity */
    UART_REG(MMUART_LCR_OFS) = MMUART_LCR_8N1;

    /* Enable and reset FIFOs */
    UART_REG(MMUART_IIR_FCR_OFS) = MMUART_FCR_EN |
                                   MMUART_FCR_RXRST |
                                   MMUART_FCR_TXRST;

    /* Disable UART interrupts for now (polled mode) */
    UART_REG(MMUART_IER_DLM_OFS) = 0u;
}

int mpfs_uart_putc(int ch)
{
    if (ch == '\n') {
        (void)mpfs_uart_putc('\r');
    }

    while ((UART_REG(MMUART_LSR_OFS) & MMUART_LSR_THRE) == 0u) {
        ;
    }

    UART_REG(MMUART_RBR_THR_DLL_OFS) = (uint32_t)((uint8_t)ch);
    return ch;
}

int mpfs_uart_kbhit(void)
{
    return ((UART_REG(MMUART_LSR_OFS) & MMUART_LSR_DR) != 0u) ? 1 : 0;
}

int mpfs_uart_getc_nonblock(void)
{
    if (!mpfs_uart_kbhit()) {
        return -1;
    }

    return (int)(UART_REG(MMUART_RBR_THR_DLL_OFS) & 0xFFu);
}
