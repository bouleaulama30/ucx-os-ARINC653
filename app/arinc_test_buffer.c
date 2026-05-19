#include <ucx.h>
#include "arinc_test_support.h"

typedef struct {
	uint32_t seq;
	uint32_t checksum;
} buffer_message_t;

static arinc_test_suite_result_t buffer_p1_suite;

static void buffer_expect_status(arinc_test_suite_result_t *suite,
							   const char *nb_case,
							   const char *max_nb_case,
							   const char *max_size_case,
							   const char *waiting_case,
							   const BUFFER_STATUS_TYPE *status,
							   MESSAGE_RANGE_TYPE expected_nb,
							   MESSAGE_RANGE_TYPE expected_max_nb,
							   MESSAGE_SIZE_TYPE expected_max_size)
{
	arinc_test_suite_check(suite, nb_case, arinc_test_check_int(expected_nb, status->NB_MESSAGE));
	arinc_test_suite_check(suite, max_nb_case, arinc_test_check_int(expected_max_nb, status->MAX_NB_MESSAGE));
	arinc_test_suite_check(suite, max_size_case, arinc_test_check_int(expected_max_size, status->MAX_MESSAGE_SIZE));
	arinc_test_suite_check(suite, waiting_case, arinc_test_check_int(0, status->WAITING_PROCESSES));
}

__attribute__((section(".p1_code")))
static void test_get_buffer_id_p1(void)
{
	RETURN_CODE_TYPE return_code;
	BUFFER_ID_TYPE buffer_id;
	BUFFER_ID_TYPE invalid_buffer_id;

	arinc_test_suite_begin(&buffer_p1_suite, "buffer_get_id");

	GET_BUFFER_ID("Buffer1", &buffer_id, &return_code);
	arinc_test_suite_check(&buffer_p1_suite, "get_id_valid_rc", arinc_test_check_int(NO_ERROR, return_code));
	arinc_test_suite_check(&buffer_p1_suite, "get_id_valid_id", buffer_id != NULL_BUFFER_ID);

	GET_BUFFER_ID("InvalidBuffer", &invalid_buffer_id, &return_code);
	arinc_test_suite_check(&buffer_p1_suite, "get_id_invalid_rc", arinc_test_check_int(INVALID_CONFIG, return_code));

	arinc_test_suite_end(&buffer_p1_suite);
}

__attribute__((section(".p1_code")))
static void test_get_buffer_status_p1(BUFFER_ID_TYPE buffer_id)
{
	RETURN_CODE_TYPE return_code;
	BUFFER_STATUS_TYPE buffer_status;

	GET_BUFFER_STATUS(buffer_id, &buffer_status, &return_code);
	arinc_test_suite_begin(&buffer_p1_suite, "buffer_status");
	arinc_test_suite_check(&buffer_p1_suite, "status_rc", arinc_test_check_int(NO_ERROR, return_code));
	buffer_expect_status(&buffer_p1_suite,
						 "status_nb_message",
						 "status_max_nb_message",
						 "status_max_message_size",
						 "status_waiting_processes",
						 &buffer_status,
						 0,
						 BUFFER_MAX_NB_MESSAGE,
						 BUFFER_MAX_MESSAGE_SIZE);
	arinc_test_suite_end(&buffer_p1_suite);
}

__attribute__((section(".p1_code")))
static void test_send_buffer_p1(BUFFER_ID_TYPE buffer_id)
{
	RETURN_CODE_TYPE return_code;
	BUFFER_STATUS_TYPE buffer_status;
	MESSAGE_SIZE_TYPE message_length;
	buffer_message_t message;

	arinc_test_suite_begin(&buffer_p1_suite, "buffer_send");

	message.seq = 1;
	message.checksum = 0x11111111u;
	message_length = sizeof(message);
	SEND_BUFFER(buffer_id, (MESSAGE_ADDR_TYPE)&message, message_length, 0, &return_code);
	arinc_test_suite_check(&buffer_p1_suite, "send_seq_1_rc", arinc_test_check_int(NO_ERROR, return_code));

	message.seq = 2;
	message.checksum = 0x22222222u;
	SEND_BUFFER(buffer_id, (MESSAGE_ADDR_TYPE)&message, message_length, 0, &return_code);
	arinc_test_suite_check(&buffer_p1_suite, "send_seq_2_rc", arinc_test_check_int(NO_ERROR, return_code));

	message.seq = 3;
	message.checksum = 0x33333333u;
	SEND_BUFFER(buffer_id, (MESSAGE_ADDR_TYPE)&message, message_length, 0, &return_code);
	arinc_test_suite_check(&buffer_p1_suite, "send_seq_3_rc", arinc_test_check_int(NO_ERROR, return_code));

	GET_BUFFER_STATUS(buffer_id, &buffer_status, &return_code);
	arinc_test_suite_check(&buffer_p1_suite, "status_after_send_rc", arinc_test_check_int(NO_ERROR, return_code));
	buffer_expect_status(&buffer_p1_suite,
						 "status_after_send_nb_message",
						 "status_after_send_max_nb_message",
						 "status_after_send_max_message_size",
						 "status_after_send_waiting_processes",
						 &buffer_status,
						 3,
						 BUFFER_MAX_NB_MESSAGE,
						 BUFFER_MAX_MESSAGE_SIZE);

	message.seq = 4;
	message.checksum = 0x44444444u;
	SEND_BUFFER(buffer_id, (MESSAGE_ADDR_TYPE)&message, message_length, 0, &return_code);
	arinc_test_suite_check(&buffer_p1_suite, "send_when_full_rc", arinc_test_check_int(NOT_AVAILABLE, return_code));

	arinc_test_suite_end(&buffer_p1_suite);
}

__attribute__((section(".p1_code")))
static void test_receive_buffer_p1(BUFFER_ID_TYPE buffer_id)
{
	RETURN_CODE_TYPE return_code;
	BUFFER_STATUS_TYPE buffer_status;
	MESSAGE_SIZE_TYPE message_length;
	buffer_message_t message = {0, 0};

	arinc_test_suite_begin(&buffer_p1_suite, "buffer_receive");

	message_length = sizeof(message);
	RECEIVE_BUFFER(buffer_id, 0, (MESSAGE_ADDR_TYPE)&message, &message_length, &return_code);
	arinc_test_suite_check(&buffer_p1_suite, "receive_seq_1_rc", arinc_test_check_int(NO_ERROR, return_code));
	arinc_test_suite_check(&buffer_p1_suite, "receive_seq_1_len", arinc_test_check_int(sizeof(message), message_length));
	arinc_test_suite_check(&buffer_p1_suite, "receive_seq_1_value", arinc_test_check_int(1, message.seq));

	message_length = sizeof(message);
	RECEIVE_BUFFER(buffer_id, 0, (MESSAGE_ADDR_TYPE)&message, &message_length, &return_code);
	arinc_test_suite_check(&buffer_p1_suite, "receive_seq_2_rc", arinc_test_check_int(NO_ERROR, return_code));
	arinc_test_suite_check(&buffer_p1_suite, "receive_seq_2_value", arinc_test_check_int(2, message.seq));

	message_length = sizeof(message);
	RECEIVE_BUFFER(buffer_id, 0, (MESSAGE_ADDR_TYPE)&message, &message_length, &return_code);
	arinc_test_suite_check(&buffer_p1_suite, "receive_seq_3_rc", arinc_test_check_int(NO_ERROR, return_code));
	arinc_test_suite_check(&buffer_p1_suite, "receive_seq_3_value", arinc_test_check_int(3, message.seq));

	GET_BUFFER_STATUS(buffer_id, &buffer_status, &return_code);
	arinc_test_suite_check(&buffer_p1_suite, "status_after_receive_rc", arinc_test_check_int(NO_ERROR, return_code));
	buffer_expect_status(&buffer_p1_suite,
						 "status_after_receive_nb_message",
						 "status_after_receive_max_nb_message",
						 "status_after_receive_max_message_size",
						 "status_after_receive_waiting_processes",
						 &buffer_status,
						 0,
						 BUFFER_MAX_NB_MESSAGE,
						 BUFFER_MAX_MESSAGE_SIZE);

	message_length = sizeof(message);
	RECEIVE_BUFFER(buffer_id, 0, (MESSAGE_ADDR_TYPE)&message, &message_length, &return_code);
	arinc_test_suite_check(&buffer_p1_suite, "receive_empty_rc", arinc_test_check_int(NOT_AVAILABLE, return_code));
	arinc_test_suite_check(&buffer_p1_suite, "receive_empty_len", arinc_test_check_int(0, message_length));

	arinc_test_suite_end(&buffer_p1_suite);
}

__attribute__((section(".p1_code")))
void p1_process1(void)
{
	RETURN_CODE_TYPE return_code;
	BUFFER_ID_TYPE buffer_id;

	test_get_buffer_id_p1();
	GET_BUFFER_ID("Buffer1", &buffer_id, &return_code);
	if (return_code != NO_ERROR) {
		STOP_SELF();
	}

	test_get_buffer_status_p1(buffer_id);
	test_send_buffer_p1(buffer_id);
	test_receive_buffer_p1(buffer_id);
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