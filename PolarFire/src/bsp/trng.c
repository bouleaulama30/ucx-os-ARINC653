#include "trng.h"
#include "mpfs_memory_map.h"


#ifdef RANDOM_TRNG
void trng_init(void)
{
    FABRIC_REG_TRNG_CTRL = TRNG_CTRL_CLEAR;
    FABRIC_REG_TRNG_CTRL = TRNG_CTRL_EN;
}

bool trng_ready(void)
{
    return (FABRIC_REG_TRNG_STAT & TRNG_STAT_VALID) != 0u;
}

bool trng_failed(void)
{
    return (FABRIC_REG_TRNG_STAT & TRNG_STAT_FAIL) != 0u;
}

uint32_t trng_read_word(void)
{
    return FABRIC_REG_TRNG_DATA;
}

#else
int platform_trng_read(uint8_t *out, size_t n) {
    (void)out;
    (void)n;
    return -1;
}

#endif
