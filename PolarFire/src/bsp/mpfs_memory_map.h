#ifndef MPFS_MEMORY_MAP_H
#define MPFS_MEMORY_MAP_H

#include <stdint.h>

/* --------------------------------------------------------------------------
 * Fabric accelerator
 * -------------------------------------------------------------------------- */
#define FABRIC_ACCEL_BASE      0x40000000UL

#define FABRIC_REG_CONTROL     (*(volatile uint32_t *)(FABRIC_ACCEL_BASE + 0x00u))
#define FABRIC_REG_OPERAND     (*(volatile uint32_t *)(FABRIC_ACCEL_BASE + 0x04u))
#define FABRIC_REG_RESULT      (*(volatile uint32_t *)(FABRIC_ACCEL_BASE + 0x08u))
#define FABRIC_REG_LEDS        (*(volatile uint32_t *)(FABRIC_ACCEL_BASE + 0x0Cu))

#define FABRIC_REG_TRNG_CTRL   (*(volatile uint32_t *)(FABRIC_ACCEL_BASE + 0x10u))
#define FABRIC_REG_TRNG_STAT   (*(volatile uint32_t *)(FABRIC_ACCEL_BASE + 0x14u))
#define FABRIC_REG_TRNG_DATA   (*(volatile uint32_t *)(FABRIC_ACCEL_BASE + 0x18u))
#define FABRIC_REG_TRNG_ID     (*(volatile uint32_t *)(FABRIC_ACCEL_BASE + 0x1Cu))

#define TRNG_CTRL_EN           (1u << 0)
#define TRNG_CTRL_CLEAR        (1u << 1)

#define TRNG_STAT_VALID        (1u << 0)
#define TRNG_STAT_FAIL         (1u << 1)
#define TRNG_STAT_OVERFLOW     (1u << 2)

/* --------------------------------------------------------------------------
 * MSS MMUART
 * -------------------------------------------------------------------------- */
#define MMUART0_LO_BASE        0x20000000UL
#define MMUART4_LO_BASE        0x20106000UL

/* 16550-style register spacing on MPFS MMUART as used by Linux DTS:
 * reg-io-width = 4, reg-shift = 2
 */
#define MMUART_RBR_THR_DLL_OFS 0x00u
#define MMUART_IER_DLM_OFS     0x01u
#define MMUART_IIR_FCR_OFS     0x02u
#define MMUART_LCR_OFS         0x03u
#define MMUART_MCR_OFS         0x04u
#define MMUART_LSR_OFS         0x05u

/* Line Status Register bits */
#define MMUART_LSR_DR          0x01u
#define MMUART_LSR_THRE        0x20u

/* Line Control Register bits */
#define MMUART_LCR_8N1         0x03u
#define MMUART_LCR_DLAB        0x80u

/* FIFO Control Register bits */
#define MMUART_FCR_EN          0x01u
#define MMUART_FCR_RXRST       0x02u
#define MMUART_FCR_TXRST       0x04u

/* --------------------------------------------------------------------------
 * CLINT
 * -------------------------------------------------------------------------- */
#define CLINT_BASE             0x02000000UL
#define CLINT_MSIP_BASE        (CLINT_BASE + 0x0000UL)
#define CLINT_MTIMECMP_BASE    (CLINT_BASE + 0x4000UL)
#define CLINT_MTIME_BASE       (CLINT_BASE + 0xBFF8UL)

#endif
