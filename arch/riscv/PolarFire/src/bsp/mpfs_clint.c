#include "mpfs_clint.h"
#include "mpfs_memory_map.h"

uint64_t mpfs_clint_mtime_read(void)
{
    volatile uint32_t * const mtime_lo =
        (volatile uint32_t *)(CLINT_MTIME_BASE + 0u);
    volatile uint32_t * const mtime_hi =
        (volatile uint32_t *)(CLINT_MTIME_BASE + 4u);

    uint32_t hi1;
    uint32_t lo;
    uint32_t hi2;

    do {
        hi1 = *mtime_hi;
        lo  = *mtime_lo;
        hi2 = *mtime_hi;
    } while (hi1 != hi2);

    return (((uint64_t)hi2) << 32) | (uint64_t)lo;
}

void mpfs_clint_mtimecmp_write(uint32_t hart_id, uint64_t value)
{
    volatile uint32_t * const cmp_lo =
        (volatile uint32_t *)(CLINT_MTIMECMP_BASE + (hart_id * 8u) + 0u);
    volatile uint32_t * const cmp_hi =
        (volatile uint32_t *)(CLINT_MTIMECMP_BASE + (hart_id * 8u) + 4u);

    /*
     * Safe update sequence:
     * write high to all 1s first, then low, then final high.
     */
    *cmp_hi = 0xFFFFFFFFu;
    *cmp_lo = (uint32_t)(value & 0xFFFFFFFFu);
    *cmp_hi = (uint32_t)(value >> 32);
}

void mpfs_clint_arm_tick(uint32_t hart_id, uint32_t tick_hz)
{
    uint64_t now;
    uint64_t delta;

    if (tick_hz == 0u) {
        tick_hz = 1000u;
    }

    now = mpfs_clint_mtime_read();
    delta = (uint64_t)MPFS_CLINT_TIMEBASE_HZ / (uint64_t)tick_hz;

    if (delta == 0u) {
        delta = 1u;
    }

    mpfs_clint_mtimecmp_write(hart_id, now + delta);
}

void mpfs_clint_reset(uint32_t hart_id)
{
    volatile uint32_t * const mtime_lo =
        (volatile uint32_t *)(CLINT_MTIME_BASE + 0u);
    volatile uint32_t * const mtime_hi =
        (volatile uint32_t *)(CLINT_MTIME_BASE + 4u);

    /* Avoid a stale compare value while mtime is reset. */
    mpfs_clint_mtimecmp_write(hart_id, UINT64_MAX);

    /* Reset global machine time to 0 at program startup. */
    *mtime_hi = 0u;
    *mtime_lo = 0u;
}
