/* task management API */
struct process_s;
int32_t ucx_process_spawn(void *task, uint16_t stack_size, struct process_s *process);