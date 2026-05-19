#include <ucx.h>
#include "arinc_test_support.h"

static arinc_test_suite_result_t semaphore_p1_suite;

static void semaphore_expect_status(arinc_test_suite_result_t *suite,
									const char *current_case,
									const char *max_case,
									const char *waiting_case,
									const SEMAPHORE_STATUS_TYPE *status,
									SEMAPHORE_VALUE_TYPE expected_current,
									SEMAPHORE_VALUE_TYPE expected_max)
{
	arinc_test_suite_check(suite, current_case, arinc_test_check_int(expected_current, status->CURRENT_VALUE));
	arinc_test_suite_check(suite, max_case, arinc_test_check_int(expected_max, status->MAXIMUM_VALUE));
	arinc_test_suite_check(suite, waiting_case, arinc_test_check_int(0, status->WAITING_PROCESSES));
}

__attribute__((section(".p1_code")))
static void test_get_semaphore_id_p1(void)
{
	RETURN_CODE_TYPE return_code;
	SEMAPHORE_ID_TYPE semaphore_id;
	SEMAPHORE_ID_TYPE invalid_semaphore_id;

	arinc_test_suite_begin(&semaphore_p1_suite, "semaphore_get_id");

	GET_SEMAPHORE_ID("Semaphore1", &semaphore_id, &return_code);
	arinc_test_suite_check(&semaphore_p1_suite, "get_id_valid_rc", arinc_test_check_int(NO_ERROR, return_code));
	arinc_test_suite_check(&semaphore_p1_suite, "get_id_valid_id", semaphore_id != NULL_SEMAPHORE_ID);

	GET_SEMAPHORE_ID("InvalidSemaphore", &invalid_semaphore_id, &return_code);
	arinc_test_suite_check(&semaphore_p1_suite, "get_id_invalid_rc", arinc_test_check_int(INVALID_CONFIG, return_code));

	arinc_test_suite_end(&semaphore_p1_suite);
}

__attribute__((section(".p1_code")))
static void test_get_semaphore_status_p1(SEMAPHORE_ID_TYPE semaphore_id)
{
	RETURN_CODE_TYPE return_code;
	SEMAPHORE_STATUS_TYPE semaphore_status;

	arinc_test_suite_begin(&semaphore_p1_suite, "semaphore_status");
	GET_SEMAPHORE_STATUS(semaphore_id, &semaphore_status, &return_code);
	arinc_test_suite_check(&semaphore_p1_suite, "status_rc", arinc_test_check_int(NO_ERROR, return_code));
	semaphore_expect_status(&semaphore_p1_suite,
							"status_current_value",
							"status_maximum_value",
							"status_waiting_processes",
							&semaphore_status,
							0,
							1);
	arinc_test_suite_end(&semaphore_p1_suite);
}

__attribute__((section(".p1_code")))
static void test_wait_semaphore_p1(SEMAPHORE_ID_TYPE semaphore_id)
{
	RETURN_CODE_TYPE return_code;
	SEMAPHORE_STATUS_TYPE semaphore_status;

	arinc_test_suite_begin(&semaphore_p1_suite, "semaphore_wait");

	WAIT_SEMAPHORE(semaphore_id, 0, &return_code);
	arinc_test_suite_check(&semaphore_p1_suite, "wait_empty_rc", arinc_test_check_int(NOT_AVAILABLE, return_code));

	SIGNAL_SEMAPHORE(semaphore_id, &return_code);
	arinc_test_suite_check(&semaphore_p1_suite, "signal_before_wait_rc", arinc_test_check_int(NO_ERROR, return_code));

	GET_SEMAPHORE_STATUS(semaphore_id, &semaphore_status, &return_code);
	arinc_test_suite_check(&semaphore_p1_suite, "status_after_signal_rc", arinc_test_check_int(NO_ERROR, return_code));
	semaphore_expect_status(&semaphore_p1_suite,
							"status_after_signal_current_value",
							"status_after_signal_maximum_value",
							"status_after_signal_waiting_processes",
							&semaphore_status,
							1,
							1);

	WAIT_SEMAPHORE(semaphore_id, 0, &return_code);
	arinc_test_suite_check(&semaphore_p1_suite, "wait_after_signal_rc", arinc_test_check_int(NO_ERROR, return_code));

	GET_SEMAPHORE_STATUS(semaphore_id, &semaphore_status, &return_code);
	arinc_test_suite_check(&semaphore_p1_suite, "status_after_wait_rc", arinc_test_check_int(NO_ERROR, return_code));
	semaphore_expect_status(&semaphore_p1_suite,
							"status_after_wait_current_value",
							"status_after_wait_maximum_value",
							"status_after_wait_waiting_processes",
							&semaphore_status,
							0,
							1);

	SIGNAL_SEMAPHORE(semaphore_id, &return_code);
	arinc_test_suite_check(&semaphore_p1_suite, "signal_after_wait_rc", arinc_test_check_int(NO_ERROR, return_code));

	SIGNAL_SEMAPHORE(semaphore_id, &return_code);
	arinc_test_suite_check(&semaphore_p1_suite, "signal_when_full_rc", arinc_test_check_int(INVALID_CONFIG, return_code));

	arinc_test_suite_end(&semaphore_p1_suite);
}

__attribute__((section(".p1_code")))
static void test_signal_semaphore_p1(SEMAPHORE_ID_TYPE semaphore_id)
{
	RETURN_CODE_TYPE return_code;
	SEMAPHORE_STATUS_TYPE semaphore_status;

	arinc_test_suite_begin(&semaphore_p1_suite, "semaphore_signal");

	WAIT_SEMAPHORE(semaphore_id, 0, &return_code);
	arinc_test_suite_check(&semaphore_p1_suite, "prepare_signal_rc", arinc_test_check_int(NO_ERROR, return_code));

	SIGNAL_SEMAPHORE(semaphore_id, &return_code);
	arinc_test_suite_check(&semaphore_p1_suite, "signal_empty_rc", arinc_test_check_int(NO_ERROR, return_code));

	GET_SEMAPHORE_STATUS(semaphore_id, &semaphore_status, &return_code);
	arinc_test_suite_check(&semaphore_p1_suite, "status_after_signal_rc", arinc_test_check_int(NO_ERROR, return_code));
	semaphore_expect_status(&semaphore_p1_suite,
							"status_after_signal_current_value",
							"status_after_signal_maximum_value",
							"status_after_signal_waiting_processes",
							&semaphore_status,
							1,
							1);

	SIGNAL_SEMAPHORE(semaphore_id, &return_code);
	arinc_test_suite_check(&semaphore_p1_suite, "signal_when_full_rc", arinc_test_check_int(INVALID_CONFIG, return_code));

	arinc_test_suite_end(&semaphore_p1_suite);
}

__attribute__((section(".p1_code")))
void p1_process1(void)
{
	RETURN_CODE_TYPE return_code;
	SEMAPHORE_ID_TYPE semaphore_id;

	test_get_semaphore_id_p1();
	GET_SEMAPHORE_ID("Semaphore1", &semaphore_id, &return_code);
	if (return_code != NO_ERROR) {
		STOP_SELF();
	}

	test_get_semaphore_status_p1(semaphore_id);
	test_wait_semaphore_p1(semaphore_id);
	test_signal_semaphore_p1(semaphore_id);
	STOP_SELF();
}

__attribute__((section(".p1_code")))
void p1_process2(void)
{
	STOP_SELF();
}

__attribute__((section(".p2_code")))
void p2_process1(void)
{
	STOP_SELF();
}

__attribute__((section(".p2_code")))
void p2_process2(void)
{
	STOP_SELF();
}

int app_main(void)
{
	size_t p1_data_size = _p1_data_end - _p1_data_start;
	size_t p1_code_size = _p1_code_end - _p1_code_start;
	size_t p2_data_size = _p2_data_end - _p2_data_start;
	size_t p2_code_size = _p2_code_end - _p2_code_start;

	partition_init(DEFAULT_PARTITION_CONFIG.period,
				   DEFAULT_PARTITION_CONFIG.duration,
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
				   DEFAULT_PARTITION_CONFIG.max_errors);

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
				   P2_CONFIG.max_errors);

	return 1;
}