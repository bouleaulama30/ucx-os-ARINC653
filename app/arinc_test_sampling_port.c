#include <ucx.h>
#include "arinc_test_support.h"

typedef struct {
	uint32_t seq;
	uint32_t sent_at_ms;
	uint32_t sender_pid;
} sampling_message_t;

static arinc_test_suite_result_t sampling_port_p1_suite;
static arinc_test_suite_result_t sampling_port_p2_suite;

static void sampling_suite_check_bool(arinc_test_suite_result_t *suite, const char *case_name, int condition)
{
	arinc_test_suite_check(suite, case_name, condition ? 1 : 0);
}

static void sampling_expect_port_status(arinc_test_suite_result_t *suite,
							   const char *max_size_case,
							   const char *direction_case,
							   const char *refresh_case,
							   const SAMPLING_PORT_STATUS_TYPE *status,
							   PORT_DIRECTION_TYPE expected_direction)
{
	arinc_test_suite_check(suite, max_size_case, arinc_test_check_int(64, status->MAX_MESSAGE_SIZE));
	arinc_test_suite_check(suite, direction_case, arinc_test_check_int(expected_direction, status->PORT_DIRECTION));
	arinc_test_suite_check(suite, refresh_case, arinc_test_check_int(200, status->REFRESH_PERIOD));
}

// Test 1: GET_SAMPLING_PORT_ID - Verify we can get valid IDs for both sender and receiver ports
__attribute__((section(".p1_code")))
void test_get_sampling_port_id_p1(void)
{
	RETURN_CODE_TYPE return_code;
	SAMPLING_PORT_ID_TYPE port_id;
	SAMPLING_PORT_ID_TYPE invalid_port_id;

	GET_SAMPLING_PORT_ID("P1_OUT_TEMP", &port_id, &return_code);

	arinc_test_suite_begin(&sampling_port_p1_suite, "sampling_port_get_id");
	arinc_test_suite_check(&sampling_port_p1_suite, "get_id_valid_port", arinc_test_check_int(NO_ERROR, return_code));
	arinc_test_suite_check(&sampling_port_p1_suite, "get_id_valid_port_id", port_id != NULL_SAMPLING_PORT_ID);

	GET_SAMPLING_PORT_ID("INVALID_PORT", &invalid_port_id, &return_code);
	arinc_test_suite_check(&sampling_port_p1_suite, "get_id_invalid_port_rc", arinc_test_check_int(INVALID_CONFIG, return_code));
	arinc_test_suite_end(&sampling_port_p1_suite);
}

// Test 2: GET_SAMPLING_PORT_STATUS - Verify port status properties
__attribute__((section(".p1_code")))
void test_get_sampling_port_status_p1(void)
{
	RETURN_CODE_TYPE return_code;
	SAMPLING_PORT_ID_TYPE port_id;
	SAMPLING_PORT_STATUS_TYPE port_status;

	GET_SAMPLING_PORT_ID("P1_OUT_TEMP", &port_id, &return_code);
	if (return_code != NO_ERROR) {
		printf("[P1] Cannot get port ID for status test\n");
		return;
	}

	GET_SAMPLING_PORT_STATUS(port_id, &port_status, &return_code);
	arinc_test_suite_begin(&sampling_port_p1_suite, "sampling_port_status");
	arinc_test_suite_check(&sampling_port_p1_suite, "status_return_code", arinc_test_check_int(NO_ERROR, return_code));
	sampling_expect_port_status(&sampling_port_p1_suite,
								"status_max_size",
								"status_direction",
								"status_refresh",
								&port_status,
								SOURCE);
	arinc_test_suite_check(&sampling_port_p1_suite, "status_initial_validity", arinc_test_check_int(INVALID, port_status.LAST_MSG_VALIDITY));
	arinc_test_suite_end(&sampling_port_p1_suite);
}

// Test 3: WRITE_SAMPLING_MESSAGE - Sender process (P1)
__attribute__((section(".p1_code")))
void test_write_sampling_message_p1(void)
{
	RETURN_CODE_TYPE return_code;
	SAMPLING_PORT_ID_TYPE port_id;
	PROCESS_ID_TYPE process_id;
	sampling_message_t msg;

	GET_MY_ID(&process_id, &return_code);
	GET_SAMPLING_PORT_ID("P1_OUT_TEMP", &port_id, &return_code);

	if (return_code != NO_ERROR) {
		printf("[P1] Cannot get port ID\n");
		return;
	}

	msg.seq = 1;
	msg.sent_at_ms = (uint32_t)ucx_uptime();
	msg.sender_pid = (uint32_t)process_id;

	WRITE_SAMPLING_MESSAGE(port_id, (MESSAGE_ADDR_TYPE)&msg, sizeof(msg), &return_code);

	arinc_test_suite_begin(&sampling_port_p1_suite, "sampling_port_write");
	arinc_test_suite_check(&sampling_port_p1_suite, "write_return_code", arinc_test_check_int(NO_ERROR, return_code));
	arinc_test_suite_end(&sampling_port_p1_suite);
}


// Test 4: READ_SAMPLING_MESSAGE and message validity - Receiver process (P2)
__attribute__((section(".p2_code")))
void test_read_sampling_message_p2(void)
{
	RETURN_CODE_TYPE return_code;
	SAMPLING_PORT_ID_TYPE port_id;
	SAMPLING_PORT_STATUS_TYPE port_status;
	MESSAGE_SIZE_TYPE message_length;
	VALIDITY_TYPE validity;
	sampling_message_t rx_msg = {0, 0, 0};

	GET_SAMPLING_PORT_ID("P2_IN_TEMP", &port_id, &return_code);
	
	if (return_code != NO_ERROR) {
		printf("[P2] Cannot get port ID\n");
		return;
	}

	message_length = sizeof(rx_msg);
	READ_SAMPLING_MESSAGE(port_id, (MESSAGE_ADDR_TYPE)&rx_msg, &message_length, &validity, &return_code);

	arinc_test_suite_begin(&sampling_port_p2_suite, "sampling_port_read");
	arinc_test_suite_check(&sampling_port_p2_suite, "read_return_code", arinc_test_check_int(NO_ERROR, return_code));
	arinc_test_suite_check(&sampling_port_p2_suite, "read_message_length", arinc_test_check_int(sizeof(sampling_message_t), message_length));
	arinc_test_suite_check(&sampling_port_p2_suite, "read_message_validity_valid", arinc_test_check_int(VALID, validity));
	arinc_test_suite_check(&sampling_port_p2_suite, "read_message_seq", arinc_test_check_int(1, rx_msg.seq));
	arinc_test_suite_check(&sampling_port_p2_suite, "read_message_sender_pid", rx_msg.sender_pid != 0);

	GET_SAMPLING_PORT_STATUS(port_id, &port_status, &return_code);
	arinc_test_suite_check(&sampling_port_p2_suite, "status_after_read_rc", arinc_test_check_int(NO_ERROR, return_code));
	arinc_test_suite_check(&sampling_port_p2_suite, "status_after_read_validity", arinc_test_check_int(VALID, port_status.LAST_MSG_VALIDITY));

	TIMED_WAIT(250, &return_code);
	message_length = sizeof(rx_msg);
	READ_SAMPLING_MESSAGE(port_id, (MESSAGE_ADDR_TYPE)&rx_msg, &message_length, &validity, &return_code);

	arinc_test_suite_check(&sampling_port_p2_suite, "read_expired_return_code", arinc_test_check_int(NO_ERROR, return_code));
	arinc_test_suite_check(&sampling_port_p2_suite, "read_message_validity_expired", arinc_test_check_int(INVALID, validity));

	GET_SAMPLING_PORT_STATUS(port_id, &port_status, &return_code);
	arinc_test_suite_check(&sampling_port_p2_suite, "status_after_expiry_rc", arinc_test_check_int(NO_ERROR, return_code));
	arinc_test_suite_check(&sampling_port_p2_suite, "status_after_expiry_validity", arinc_test_check_int(INVALID, port_status.LAST_MSG_VALIDITY));

	arinc_test_suite_end(&sampling_port_p2_suite);
}

__attribute__((section(".p1_code")))
void p1_process1(void)
{
	test_get_sampling_port_id_p1();
	test_get_sampling_port_status_p1();
	test_write_sampling_message_p1();
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
	test_read_sampling_message_p2();
	STOP_SELF();
}

__attribute__((section(".p2_code")))
void p2_process2(void)
{
	STOP_SELF();
}

int app_main(void)
{
	// la partie data est pour l'instant la stack de la task de l'entry point de P1 donc elle grandit vers le bas
	size_t p1_data_size =  _p1_data_end -_p1_data_start;
	size_t p1_code_size =  _p1_code_end -_p1_code_start;

	// la partie data est pour l'instant la stack de la task de l'entry point de P1 donc elle grandit vers le bas
	size_t p2_data_size =  _p2_data_end -_p2_data_start;
	size_t p2_code_size =  _p2_code_end -_p2_code_start;

	partition_init(DEFAULT_PARTITION_CONFIG.period,
				   DEFAULT_PARTITION_CONFIG.duration,
				   DEFAULT_PARTITION_CONFIG.identifier,
				   DEFAULT_PARTITION_CONFIG.num_assigned_cores,
				   DEFAULT_PARTITION_CONFIG.name,
				   DEFAULT_PARTITION_CONFIG.region_name_code_mem,
				   (void*)_p1_code_start,
				   (size_t)p1_code_size,
				   DEFAULT_PARTITION_CONFIG.access_code_mem,
				   DEFAULT_PARTITION_CONFIG.region_name_data_mem,
				   (void*)_p1_data_start,
				   p1_data_size,
				   DEFAULT_PARTITION_CONFIG.access_data_mem,
				//    test_spatial_violation_p2,
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
				   (void*)_p2_code_start,
				   (size_t)p2_code_size,
				   P2_CONFIG.access_code_mem,
				   P2_CONFIG.region_name_data_mem,
				   (void*)_p2_data_start,
				   p2_data_size,
				   P2_CONFIG.access_data_mem,
				//    test_spatial_violation_p1,
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
