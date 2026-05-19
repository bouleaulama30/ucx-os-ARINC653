#include <ucx.h>
#include "arinc_test_support.h"

#define WRITE_QUEUING_MESSAGE SEND_QUEUING_MESSAGE
#define READ_QUEUING_MESSAGE RECEIVE_QUEUING_MESSAGE

typedef struct {
	uint32_t seq;
	uint32_t sent_at_ms;
	uint32_t sender_pid;
} queuing_message_t;

static arinc_test_suite_result_t queuing_port_p1_suite;
static arinc_test_suite_result_t queuing_port_p2_suite;

static void queuing_expect_status(arinc_test_suite_result_t *suite,
							   const char *nb_case,
							   const char *max_case,
							   const char *direction_case,
							   const char *waiting_case,
							   const QUEUING_PORT_STATUS_TYPE *status,
							   int expected_nb,
							   PORT_DIRECTION_TYPE expected_direction)
{
	if (expected_nb >= 0) {
		arinc_test_suite_check(suite, nb_case, arinc_test_check_int(expected_nb, status->NB_MESSAGE));
	}
	arinc_test_suite_check(suite, max_case, arinc_test_check_int(10, status->MAX_NB_MESSAGE));
	arinc_test_suite_check(suite, direction_case, arinc_test_check_int(expected_direction, status->PORT_DIRECTION));
	arinc_test_suite_check(suite, waiting_case, arinc_test_check_int(0, status->WAITING_PROCESSES));
}

__attribute__((section(".p1_code")))
void test_get_queuing_port_id_p1(void)
{
	RETURN_CODE_TYPE return_code;
	QUEUING_PORT_ID_TYPE port_id;
	QUEUING_PORT_ID_TYPE invalid_port_id;

	GET_QUEUING_PORT_ID("P1_IN_CMDS", &port_id, &return_code);

	arinc_test_suite_begin(&queuing_port_p1_suite, "queuing_port_get_id");
	arinc_test_suite_check(&queuing_port_p1_suite, "get_id_valid_port", arinc_test_check_int(NO_ERROR, return_code));
	arinc_test_suite_check(&queuing_port_p1_suite, "get_id_valid_port_id", port_id != NULL_QUEUING_PORT_ID);

	GET_QUEUING_PORT_ID("INVALID_QUEUE", &invalid_port_id, &return_code);
	arinc_test_suite_check(&queuing_port_p1_suite, "get_id_invalid_port_rc", arinc_test_check_int(INVALID_CONFIG, return_code));
	arinc_test_suite_end(&queuing_port_p1_suite);
}

__attribute__((section(".p1_code")))
void test_get_queuing_port_status_p1(void)
{
	RETURN_CODE_TYPE return_code;
	QUEUING_PORT_ID_TYPE port_id;
	QUEUING_PORT_STATUS_TYPE port_status;

	GET_QUEUING_PORT_ID("P1_IN_CMDS", &port_id, &return_code);
	if (return_code != NO_ERROR) {
		return;
	}

	GET_QUEUING_PORT_STATUS(port_id, &port_status, &return_code);
	arinc_test_suite_begin(&queuing_port_p1_suite, "queuing_port_status_p1");
	arinc_test_suite_check(&queuing_port_p1_suite, "status_return_code", arinc_test_check_int(NO_ERROR, return_code));
	queuing_expect_status(&queuing_port_p1_suite,
						  "status_nb_message",
						  "status_max_nb_message",
						  "status_direction",
						  "status_waiting_processes",
						  &port_status,
						  0,
						  DESTINATION);
	arinc_test_suite_end(&queuing_port_p1_suite);
}

__attribute__((section(".p2_code")))
void test_get_queuing_port_id_p2(void)
{
	RETURN_CODE_TYPE return_code;
	QUEUING_PORT_ID_TYPE port_id;

	GET_QUEUING_PORT_ID("P2_OUT_CMDS", &port_id, &return_code);

	arinc_test_suite_begin(&queuing_port_p2_suite, "queuing_port_get_id_p2");
	arinc_test_suite_check(&queuing_port_p2_suite, "get_id_valid_port", arinc_test_check_int(NO_ERROR, return_code));
	arinc_test_suite_check(&queuing_port_p2_suite, "get_id_valid_port_id", port_id != NULL_QUEUING_PORT_ID);
	arinc_test_suite_end(&queuing_port_p2_suite);
}

__attribute__((section(".p2_code")))
void test_get_queuing_port_status_p2(void)
{
	RETURN_CODE_TYPE return_code;
	QUEUING_PORT_ID_TYPE port_id;
	QUEUING_PORT_STATUS_TYPE port_status;

	GET_QUEUING_PORT_ID("P2_OUT_CMDS", &port_id, &return_code);
	if (return_code != NO_ERROR) {
		return;
	}

	GET_QUEUING_PORT_STATUS(port_id, &port_status, &return_code);
	arinc_test_suite_begin(&queuing_port_p2_suite, "queuing_port_status_p2");
	arinc_test_suite_check(&queuing_port_p2_suite, "status_return_code", arinc_test_check_int(NO_ERROR, return_code));
	queuing_expect_status(&queuing_port_p2_suite,
						  "status_nb_message",
						  "status_max_nb_message",
						  "status_direction",
						  "status_waiting_processes",
						  &port_status,
						  0,
						  SOURCE);
	arinc_test_suite_end(&queuing_port_p2_suite);
}

__attribute__((section(".p2_code")))
void test_write_queuing_message_p2(void)
{
	RETURN_CODE_TYPE return_code;
	QUEUING_PORT_ID_TYPE port_id;
	QUEUING_PORT_STATUS_TYPE port_status;
	PROCESS_ID_TYPE process_id;
	queuing_message_t message;

	GET_MY_ID(&process_id, &return_code);
	GET_QUEUING_PORT_ID("P2_OUT_CMDS", &port_id, &return_code);
	if (return_code != NO_ERROR) {
		return;
	}

	arinc_test_suite_begin(&queuing_port_p2_suite, "queuing_port_write");

	message.seq = 1;
	message.sent_at_ms = (uint32_t)ucx_uptime();
	message.sender_pid = (uint32_t)process_id;
	WRITE_QUEUING_MESSAGE(port_id, (MESSAGE_ADDR_TYPE)&message, sizeof(message), 0, &return_code);
	arinc_test_suite_check(&queuing_port_p2_suite, "write_seq_1_rc", arinc_test_check_int(NO_ERROR, return_code));

	message.seq = 2;
	message.sent_at_ms = (uint32_t)ucx_uptime();
	message.sender_pid = (uint32_t)process_id;
	WRITE_QUEUING_MESSAGE(port_id, (MESSAGE_ADDR_TYPE)&message, sizeof(message), 0, &return_code);
	arinc_test_suite_check(&queuing_port_p2_suite, "write_seq_2_rc", arinc_test_check_int(NO_ERROR, return_code));

	message.seq = 3;
	message.sent_at_ms = (uint32_t)ucx_uptime();
	message.sender_pid = (uint32_t)process_id;
	WRITE_QUEUING_MESSAGE(port_id, (MESSAGE_ADDR_TYPE)&message, sizeof(message), 0, &return_code);
	arinc_test_suite_check(&queuing_port_p2_suite, "write_seq_3_rc", arinc_test_check_int(NO_ERROR, return_code));

	GET_QUEUING_PORT_STATUS(port_id, &port_status, &return_code);
	arinc_test_suite_check(&queuing_port_p2_suite, "status_after_write_rc", arinc_test_check_int(NO_ERROR, return_code));
	queuing_expect_status(&queuing_port_p2_suite,
						  "status_after_write_nb_message",
						  "status_after_write_max_nb_message",
						  "status_after_write_direction",
						  "status_after_write_waiting_processes",
						  &port_status,
						  -1,
						  SOURCE);

	arinc_test_suite_end(&queuing_port_p2_suite);
}

__attribute__((section(".p1_code")))
void test_read_queuing_message_p1(void)
{
	RETURN_CODE_TYPE return_code;
	QUEUING_PORT_ID_TYPE port_id;
	QUEUING_PORT_STATUS_TYPE port_status;
	MESSAGE_SIZE_TYPE message_length;
	queuing_message_t rx_message = {0, 0, 0};

	GET_QUEUING_PORT_ID("P1_IN_CMDS", &port_id, &return_code);
	if (return_code != NO_ERROR) {
		return;
	}

	arinc_test_suite_begin(&queuing_port_p1_suite, "queuing_port_read");

	message_length = sizeof(rx_message);
	READ_QUEUING_MESSAGE(port_id, 250, (MESSAGE_ADDR_TYPE)&rx_message, &message_length, &return_code);
	arinc_test_suite_check(&queuing_port_p1_suite, "read_seq_1_rc", arinc_test_check_int(NO_ERROR, return_code));
	arinc_test_suite_check(&queuing_port_p1_suite, "read_seq_1_len", arinc_test_check_int(sizeof(rx_message), message_length));
	arinc_test_suite_check(&queuing_port_p1_suite, "read_seq_1_value", arinc_test_check_int(1, rx_message.seq));

	message_length = sizeof(rx_message);
	READ_QUEUING_MESSAGE(port_id, 0, (MESSAGE_ADDR_TYPE)&rx_message, &message_length, &return_code);
	arinc_test_suite_check(&queuing_port_p1_suite, "read_seq_2_rc", arinc_test_check_int(NO_ERROR, return_code));
	arinc_test_suite_check(&queuing_port_p1_suite, "read_seq_2_value", arinc_test_check_int(2, rx_message.seq));

	GET_QUEUING_PORT_STATUS(port_id, &port_status, &return_code);
	arinc_test_suite_check(&queuing_port_p1_suite, "status_after_two_reads_rc", arinc_test_check_int(NO_ERROR, return_code));
	queuing_expect_status(&queuing_port_p1_suite,
						  "status_after_two_reads_nb_message",
						  "status_after_two_reads_max_nb_message",
						  "status_after_two_reads_direction",
						  "status_after_two_reads_waiting_processes",
						  &port_status,
						  -1,
						  DESTINATION);

	CLEAR_QUEUING_PORT(port_id, &return_code);
	arinc_test_suite_check(&queuing_port_p1_suite, "clear_return_code", arinc_test_check_int(NO_ERROR, return_code));

	GET_QUEUING_PORT_STATUS(port_id, &port_status, &return_code);
	arinc_test_suite_check(&queuing_port_p1_suite, "status_after_clear_rc", arinc_test_check_int(NO_ERROR, return_code));
	queuing_expect_status(&queuing_port_p1_suite,
						  "status_after_clear_nb_message",
						  "status_after_clear_max_nb_message",
						  "status_after_clear_direction",
						  "status_after_clear_waiting_processes",
						  &port_status,
						  0,
						  DESTINATION);

	message_length = sizeof(rx_message);
	READ_QUEUING_MESSAGE(port_id, 0, (MESSAGE_ADDR_TYPE)&rx_message, &message_length, &return_code);
	arinc_test_suite_check(&queuing_port_p1_suite, "read_after_clear_rc", arinc_test_check_int(NOT_AVAILABLE, return_code));

	arinc_test_suite_end(&queuing_port_p1_suite);
}

__attribute__((section(".p1_code")))
void p1_process1(void)
{
	test_get_queuing_port_id_p1();
	test_get_queuing_port_status_p1();
	test_read_queuing_message_p1();
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
	test_get_queuing_port_id_p2();
	test_get_queuing_port_status_p2();
	test_write_queuing_message_p2();
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