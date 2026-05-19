#include <ucx.h>
#include "arinc_test_support.h"

extern uint8_t _p1_code_start[];
extern uint8_t _p1_code_end[];

extern uint8_t _p1_data_start[];
extern uint8_t _p1_data_end[];

extern uint8_t _p2_code_start[];
extern uint8_t _p2_code_end[];

extern uint8_t _p2_data_start[];
extern uint8_t _p2_data_end[];

/* Partition entry points defined in arinc/static_conf.c */
extern void p1_main_process(struct pcb_s *partition);
extern void p2_main_process(struct pcb_s *partition);

__attribute__((section(".p1_code")))
static const char *return_code_to_str(RETURN_CODE_TYPE rc)
{
	switch (rc) {
	case NO_ERROR:
		return "NO_ERROR";
	case INVALID_PARAM:
		return "INVALID_PARAM";
	case NO_ACTION:
		return "NO_ACTION";
	case INVALID_MODE:
		return "INVALID_MODE";
	case NOT_AVAILABLE:
		return "NOT_AVAILABLE";
	case TIMED_OUT:
		return "TIMED_OUT";
	default:
		return "UNKNOWN";
	}
}

__attribute__((section(".p1_code")))
static const char *process_state_to_str(PROCESS_STATE_TYPE state)
{
	switch (state) {
	case DORMANT:
		return "DORMANT";
	case READY:
		return "READY";
	case RUNNING:
		return "RUNNING";
	case WAITING:
		return "WAITING";
	default:
		return "UNKNOWN";
	}
}

static arinc_test_suite_result_t timed_wait_suite;
static char timed_wait_trace[6];
static unsigned int timed_wait_trace_count;
static int timed_wait_suite_started;

__attribute__((section(".p1_code")))
static void timed_wait_record(char process_name)
{
	if (timed_wait_trace_count < (sizeof(timed_wait_trace) / sizeof(timed_wait_trace[0]))) {
		timed_wait_trace[timed_wait_trace_count++] = process_name;
	}
}

__attribute__((section(".p1_code")))
static int timed_wait_trace_is_valid(void)
{
	static const char expected_abab[] = { 'A', 'B', 'A', 'B', 'A', 'B' };
	static const char expected_baba[] = { 'B', 'A', 'B', 'A', 'B', 'A' };
	unsigned int index;
	int match_abab = 1;
	int match_baba = 1;

	if (timed_wait_trace_count != 6) {
		return 0;
	}

	for (index = 0; index < 6; ++index) {
		if (timed_wait_trace[index] != expected_abab[index]) {
			match_abab = 0;
		}
		if (timed_wait_trace[index] != expected_baba[index]) {
			match_baba = 0;
		}
	}

	return match_abab || match_baba;
}

__attribute__((section(".p1_code")))
void test_round_robin_A(void) {
    RETURN_CODE_TYPE ret;
    int compteur = 0;

	if (!timed_wait_suite_started) {
		arinc_test_suite_begin(&timed_wait_suite, "time_timed_wait_round_robin");
		timed_wait_suite_started = 1;
	}

    while (compteur < 3) {
        printf("A\n");
        timed_wait_record('A');
        compteur++;
		TIMED_WAIT(0, &ret);
    }

	TIMED_WAIT(50, &ret);

	arinc_test_suite_check(&timed_wait_suite, "Round-robin FIFO + TIMED_WAIT(0/50)", timed_wait_trace_is_valid());
	arinc_test_suite_end(&timed_wait_suite);
	printf("[ARINC_TEST] completed suite time_timed_wait_round_robin\n");
    
    STOP_SELF();
}

__attribute__((section(".p1_code")))
void test_round_robin_B(void) {
    RETURN_CODE_TYPE ret;
    int compteur = 0;

    while (compteur < 3) {
        printf("B\n");
		timed_wait_record('B');
        compteur++;
		TIMED_WAIT(0, &ret);
    }

    STOP_SELF();
}

int app_main(void)
{
	size_t p1_data_size = _p1_data_end - _p1_data_start;
	size_t p1_code_size = _p1_code_end - _p1_code_start;
	size_t p2_data_size = _p2_data_end - _p2_data_start;
	size_t p2_code_size = _p2_code_end - _p2_code_start;

	partition_init(DEFAULT_PARTITION_CONFIG.period,
				   DEFAULT_PARTITION_CONFIG.duration * 8,
				   DEFAULT_PARTITION_CONFIG.identifier,
				   DEFAULT_PARTITION_CONFIG.num_assigned_cores,
				   DEFAULT_PARTITION_CONFIG.name,
				   DEFAULT_PARTITION_CONFIG.region_name_code_mem,
				   (void *)_p1_code_start,
				   (size_t)p1_code_size,
				   DEFAULT_PARTITION_CONFIG.access_code_mem,
				   DEFAULT_PARTITION_CONFIG.region_name_data_mem,
				   (void *)_p1_data_start,
				   p1_data_size,
				   DEFAULT_PARTITION_CONFIG.access_data_mem,
				   p1_main_process,
				   DEFAULT_PARTITION_CONFIG.is_system_partition,

				   DEFAULT_PARTITION_CONFIG.sampling_ports,
				   DEFAULT_PARTITION_CONFIG.max_sampling_ports,
				   DEFAULT_PARTITION_CONFIG.sampling_port_count,
				   DEFAULT_PARTITION_CONFIG.max_sampling_port_data_size,

				   DEFAULT_PARTITION_CONFIG.queuing_ports,
				   DEFAULT_PARTITION_CONFIG.max_queuing_ports,
				   DEFAULT_PARTITION_CONFIG.queuing_port_count,
				   DEFAULT_PARTITION_CONFIG.max_queuing_port_data_size,

				   DEFAULT_PARTITION_CONFIG.blackboards,
				   DEFAULT_PARTITION_CONFIG.max_blackboards,
				   DEFAULT_PARTITION_CONFIG.blackboard_count,
				   DEFAULT_PARTITION_CONFIG.max_blackboard_data_size,
				   DEFAULT_PARTITION_CONFIG.blackboards_data,
				   DEFAULT_PARTITION_CONFIG.blackboards_size_data,

				   DEFAULT_PARTITION_CONFIG.buffers,
				   DEFAULT_PARTITION_CONFIG.max_buffers,
				   DEFAULT_PARTITION_CONFIG.buffer_count,
				   DEFAULT_PARTITION_CONFIG.max_buffer_data_size,
				   DEFAULT_PARTITION_CONFIG.buffers_data,
				   DEFAULT_PARTITION_CONFIG.buffers_size_data,

				   DEFAULT_PARTITION_CONFIG.semaphores,
				   DEFAULT_PARTITION_CONFIG.max_semaphores,
				   DEFAULT_PARTITION_CONFIG.semaphore_count,
				   DEFAULT_PARTITION_CONFIG.semaphores_counter,

				   DEFAULT_PARTITION_CONFIG.events,
				   DEFAULT_PARTITION_CONFIG.max_events,
				   DEFAULT_PARTITION_CONFIG.event_count,

				   DEFAULT_PARTITION_CONFIG.mutexes,
				   DEFAULT_PARTITION_CONFIG.max_mutexes,
				   DEFAULT_PARTITION_CONFIG.mutex_count,

				   DEFAULT_PARTITION_CONFIG.error_list,
				   DEFAULT_PARTITION_CONFIG.error_list_cb,
				   DEFAULT_PARTITION_CONFIG.partition_hm_table,
				   DEFAULT_PARTITION_CONFIG.max_errors
				   );

	partition_init(P2_CONFIG.period,
				   P2_CONFIG.duration,
				   P2_CONFIG.identifier,
				   P2_CONFIG.num_assigned_cores,
				   P2_CONFIG.name,
				   P2_CONFIG.region_name_code_mem,
				   (void *)_p2_code_start,
				   (size_t)p2_code_size,
				   P2_CONFIG.access_code_mem,
				   P2_CONFIG.region_name_data_mem,
				   (void *)_p2_data_start,
				   p2_data_size,
				   P2_CONFIG.access_data_mem,
				   p2_main_process,
				   P2_CONFIG.is_system_partition,

				   P2_CONFIG.sampling_ports,
				   P2_CONFIG.max_sampling_ports,
				   P2_CONFIG.sampling_port_count,
				   P2_CONFIG.max_sampling_port_data_size,

				   P2_CONFIG.queuing_ports,
				   P2_CONFIG.max_queuing_ports,
				   P2_CONFIG.queuing_port_count,
				   P2_CONFIG.max_queuing_port_data_size,

				   P2_CONFIG.blackboards,
				   P2_CONFIG.max_blackboards,
				   P2_CONFIG.blackboard_count,
				   P2_CONFIG.max_blackboard_data_size,
				   P2_CONFIG.blackboards_data,
				   P2_CONFIG.blackboards_size_data,

				   P2_CONFIG.buffers,
				   P2_CONFIG.max_buffers,
				   P2_CONFIG.buffer_count,
				   P2_CONFIG.max_buffer_data_size,
				   P2_CONFIG.buffers_data,
				   P2_CONFIG.buffers_size_data,

				   P2_CONFIG.semaphores,
				   P2_CONFIG.max_semaphores,
				   P2_CONFIG.semaphore_count,
				   P2_CONFIG.semaphores_counter,

				   P2_CONFIG.events,
				   P2_CONFIG.max_events,
				   P2_CONFIG.event_count,

				   P2_CONFIG.mutexes,
				   P2_CONFIG.max_mutexes,
				   P2_CONFIG.mutex_count,

				   P2_CONFIG.error_list,
				   P2_CONFIG.error_list_cb,
				   P2_CONFIG.partition_hm_table,
				   P2_CONFIG.max_errors
				   );

	return 1;
}