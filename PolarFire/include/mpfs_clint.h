#ifndef MPFS_CLINT_H
#define MPFS_CLINT_H

#include <stdint.h>

#ifndef MPFS_CLINT_TIMEBASE_HZ
#define MPFS_CLINT_TIMEBASE_HZ 1000000UL
#endif

uint64_t mpfs_clint_mtime_read(void);
void     mpfs_clint_mtimecmp_write(uint32_t hart_id, uint64_t value);
void     mpfs_clint_arm_tick(uint32_t hart_id, uint32_t tick_hz);

#endif
