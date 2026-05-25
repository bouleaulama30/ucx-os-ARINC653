#ifndef FABRIC_ACCEL_H
#define FABRIC_ACCEL_H

#include <stdint.h>
#include <stdbool.h>

void fabric_accel_set_leds(uint32_t value);
void fabric_accel_write_operand(uint32_t value);
void fabric_accel_start(void);
bool fabric_accel_done(void);
uint32_t fabric_accel_read_result(void);

#endif
