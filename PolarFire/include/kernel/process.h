/* task management API */

#include "../arinc/arinc_partition.h"
#include "../arinc/arinc_process.h"
#include "../arinc/arinc_time.h"

#ifndef PROCESS
#define PROCESS

int32_t ucx_process_spawn(void *task, uint16_t stack_size, struct process_s *process, struct pcb_s *current_partition, int is_error_handler);
uint16_t process_schedule(void);

static inline void yield_to_partition(struct pcb_s *partition, struct process_s *current_process) {
    if (setjmp(current_process->tcb.context) == 0) {
        longjmp(partition->partition_context, 1);
    }
}

#endif