#include <ucx.h>

// extern uint32_t start_time;
// extern int time_initialized;

void print_time()
{
	uint32_t secs, msecs, time;
	
	time = ucx_uptime();
	// if (!time_initialized) {
    //     start_time = time;
    //     time_initialized = 1;
    // }
	// time -= start_time;
	secs = time / 1000;
	msecs = time - secs * 1000;
	
	printf("%ld.%03lds\n", secs, msecs);
}


__attribute__((section(".p1_code")))
void p1_process1(void)
{   
	RETURN_CODE_TYPE return_code;
	APEX_INTEGER partition_id;
	APEX_INTEGER process_id;
	QUEUING_PORT_ID_TYPE queuing_port_id;
	MESSAGE_SIZE_TYPE message_length;
	uint32_t received_seq = 0;
	char received_message[32] = {0};

	GET_MY_PARTITION_ID(&partition_id, &return_code);
	GET_MY_ID(&process_id, &return_code);
	GET_QUEUING_PORT_ID("P1_IN_CMDS", &queuing_port_id, &return_code);

	printf("[P1/Process1] GET_QUEUING_PORT_ID('P1_IN_CMDS') rc=%d id=%d\n", return_code, queuing_port_id);

	while (1) {
		message_length = 0;
		RECEIVE_QUEUING_MESSAGE(queuing_port_id, 200, (MESSAGE_ADDR_TYPE)received_message, &message_length, &return_code);

		if (return_code == NO_ERROR) {
			received_message[(message_length < sizeof(received_message)) ? message_length : (sizeof(received_message) - 1)] = '\0';
			printf("[P1/Process1] RECEIVE_QUEUING_MESSAGE rc=%d len=%d seq=%lu msg='%s'\n",
				   return_code,
				   message_length,
				   (unsigned long)received_seq,
				   received_message);
			received_seq++;
		} else if (return_code == NOT_AVAILABLE) {
			printf("[P1/Process1] RECEIVE_QUEUING_MESSAGE rc=%d (queue vide)\n", return_code);
		} else {
			printf("[P1/Process1] RECEIVE_QUEUING_MESSAGE rc=%d FAIL\n", return_code);
		}

		TIMED_WAIT(10, &return_code);
	}
}

__attribute__((section(".p1_code")))
void p1_process2(void)
{   
	RETURN_CODE_TYPE return_code;
	APEX_INTEGER partition_id;
	APEX_INTEGER process_id;
	SAMPLING_PORT_ID_TYPE port_id;
	MESSAGE_SIZE_TYPE write_len;

	struct {
		uint32_t seq;
		uint32_t sent_at_ms;
		uint32_t sender_pid;
	} tx_msg;

	GET_MY_PARTITION_ID(&partition_id, &return_code);
	GET_MY_ID(&process_id, &return_code);
	GET_SAMPLING_PORT_ID("P1_OUT_TEMP", &port_id, &return_code);

	printf("[P1/Process2] GET_SAMPLING_PORT_ID('P1_OUT_TEMP') rc=%d id=%d\n", return_code, port_id);

	tx_msg.seq = 0;
	while (1) {
		tx_msg.seq++;
		tx_msg.sent_at_ms = ucx_uptime();
		tx_msg.sender_pid = (uint32_t)process_id;
		write_len = sizeof(tx_msg);

		WRITE_SAMPLING_MESSAGE(port_id, (MESSAGE_ADDR_TYPE)&tx_msg, write_len, &return_code);
		printf("[P1/Process2] WRITE_SAMPLING_MESSAGE seq=%lu len=%d rc=%d %s\n",
			   tx_msg.seq,
			   write_len,
			   return_code,
			   (return_code == NO_ERROR) ? "PASS" : "FAIL");

		TIMED_WAIT(10, &return_code);
	}
}

__attribute__((section(".p2_code")))
void p2_process1(void)
{   
	RETURN_CODE_TYPE return_code;
	APEX_INTEGER partition_id;
	APEX_INTEGER process_id;
	QUEUING_PORT_ID_TYPE queuing_port_id;
	SAMPLING_PORT_ID_TYPE sampling_port_id;
	MESSAGE_SIZE_TYPE message_length;
	uint32_t seq = 0;
	char message[32];
	struct {
		uint32_t seq;
		uint32_t sent_at_ms;
		uint32_t sender_pid;
	} tx_msg;
	VALIDITY_TYPE validity;

	GET_MY_PARTITION_ID(&partition_id, &return_code);
	GET_MY_ID(&process_id, &return_code);
	GET_QUEUING_PORT_ID("P2_OUT_CMDS", &queuing_port_id, &return_code);
	printf("[P2/Process1] GET_QUEUING_PORT_ID('P2_OUT_CMDS') rc=%d id=%d\n", return_code, queuing_port_id);

	GET_SAMPLING_PORT_ID("P2_IN_TEMP", &sampling_port_id, &return_code);
	printf("[P2/Process1] GET_SAMPLING_PORT_ID('P2_IN_TEMP') rc=%d id=%d\n", return_code, sampling_port_id);


	while (1) {
		seq++;
		message_length = (MESSAGE_SIZE_TYPE)sprintf(message, "cmd-seq=%lu from=P2p1", (unsigned long)seq);

		SEND_QUEUING_MESSAGE(queuing_port_id, (MESSAGE_ADDR_TYPE)message, 18 + 1, 10, &return_code);
		printf("[P2/Process1] SEND_QUEUING_MESSAGE rc=%d seq=%lu len=%d msg='%s'\n",
			   return_code,
			   (unsigned long)seq,
			   message_length + 1,
			   message);

		message_length = sizeof(tx_msg);
		READ_SAMPLING_MESSAGE(sampling_port_id, (MESSAGE_ADDR_TYPE)&tx_msg, &message_length, &validity, &return_code);
		printf("[P2/Process1] READ_SAMPLING_MESSAGE rc=%d len=%d validity=%d seq=%lu sent_at_ms=%lu sender_pid=%lu\n",
			   return_code,
			   message_length,
			   validity,
			   (unsigned long)tx_msg.seq,
			   (unsigned long)tx_msg.sent_at_ms,
			   (unsigned long)tx_msg.sender_pid);

		TIMED_WAIT(10, &return_code);
	}
}

__attribute__((section(".p2_code")))
void p2_process2(void)
{   
	RETURN_CODE_TYPE return_code;
	APEX_INTEGER partition_id;
	APEX_INTEGER process_id;
	QUEUING_PORT_ID_TYPE queuing_port_id;
	MESSAGE_SIZE_TYPE message_length;
	uint32_t seq = 0;
	char message[32];

	GET_MY_PARTITION_ID(&partition_id, &return_code);
	GET_MY_ID(&process_id, &return_code);
	GET_QUEUING_PORT_ID("P2_OUT_CMDS", &queuing_port_id, &return_code);

	printf("[P2/Process2] GET_QUEUING_PORT_ID('P2_OUT_CMDS') rc=%d id=%d\n", return_code, queuing_port_id);

	while (1) {
		seq++;
		message_length = (MESSAGE_SIZE_TYPE)sprintf(message, "cmd-seq=%lu from=P2p2", (unsigned long)seq);

		SEND_QUEUING_MESSAGE(queuing_port_id, (MESSAGE_ADDR_TYPE)message, 18 + 1, 10, &return_code);
		printf("[P2/Process2] SEND_QUEUING_MESSAGE rc=%d seq=%lu len=%d msg='%s'\n",
			   return_code,
			   (unsigned long)seq,
			   message_length + 1,
			   message);

		TIMED_WAIT(10, &return_code);
	}
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
