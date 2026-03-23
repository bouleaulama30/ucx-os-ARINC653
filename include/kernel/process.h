/* task management API */

#include "../arinc/arinc_partition.h"
#include "../arinc/arinc_process.h"
#include "../arinc/arinc_time.h"

#ifndef PROCESS
#define PROCESS

int32_t ucx_process_spawn(void *task, uint16_t stack_size, struct process_s *process, struct pcb_s *current_partition);
uint16_t process_schedule(void);
#endif