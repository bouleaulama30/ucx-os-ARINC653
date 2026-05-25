#include "fabric_accel.h"
#include "mpfs_memory_map.h"

void fabric_accel_set_leds(uint32_t value)
{
    FABRIC_REG_LEDS = value;
}

void fabric_accel_write_operand(uint32_t value)
{
    FABRIC_REG_OPERAND = value;
}

void fabric_accel_start(void)
{
    FABRIC_REG_CONTROL = 1u;
}

bool fabric_accel_done(void)
{
    return ((FABRIC_REG_CONTROL & (1u << 1)) != 0u);
}

uint32_t fabric_accel_read_result(void)
{
    return FABRIC_REG_RESULT;
}
