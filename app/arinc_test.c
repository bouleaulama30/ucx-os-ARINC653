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
void test_spatial_violation_p2(void) {
    printf("--- Test 2: Tentative d'ecriture sur P2 (0x%08x) ---\n", (unsigned int)_p2_code_start);
    printf("ATTENTION: Le systeme DOIT crasher ou lever une exception maintenant.\n");
    
    volatile int *ptr = (int *)_p2_code_start;
    
    // Si l'isolation matérielle est active, cette ligne stoppe l'exécution
    *ptr = 0xDEADBEEF; 

    // Si on arrive ici, c'est un échec de l'isolation
    printf("[CRITICAL FAIL] P1 a reussi a ecrire dans P2 !\n");
}

__attribute__((section(".p2_code")))
void test_spatial_violation_p1(void) {
    printf("--- Test 2: Tentative d'ecriture sur P1 (0x%08x) ---\n", (unsigned int)_p1_data_start);
    printf("ATTENTION: Le systeme DOIT crasher ou lever une exception maintenant.\n");
    
    volatile int *ptr = (int *)_p1_data_start;
    
    // Si l'isolation matérielle est active, cette ligne stoppe l'exécution
    *ptr = 0xDEADBEEF; 

    // Si on arrive ici, c'est un échec de l'isolation
    printf("[CRITICAL FAIL] P2 a reussi a ecrire dans P1 !\n");
}

__attribute__((section(".p1_code")))
void p1_process1(void)
{   
	RETURN_CODE_TYPE return_code;
	APEX_INTEGER partition_id;
	APEX_INTEGER process_id;
	QUEUING_PORT_ID_TYPE queuing_port_id;
	MESSAGE_SIZE_TYPE message_length;
	char message[64];

	GET_MY_PARTITION_ID(&partition_id, &return_code);
	GET_MY_ID(&process_id, &return_code);

	while (1) {
		GET_QUEUING_PORT_ID("P1_IN_CMDS", &queuing_port_id, &return_code);
		if (return_code == NO_ERROR) {
			break;
		}
		printf("[P1/Process1] GET_QUEUING_PORT_ID('P1_IN_CMDS') rc=%d (retry)\n", return_code);
		TIMED_WAIT(2, &return_code);
	}

	printf("\n--- START TEST QUEUING PORT (P2 send -> P1 receive) ---\n");
	printf("[P1/Process1] partition=%d pid=%d queuing_port_id=%d\n", partition_id, process_id, queuing_port_id);

	while (1) {
		RECEIVE_QUEUING_MESSAGE(queuing_port_id,
		                       10,
		                       (MESSAGE_ADDR_TYPE)message,
		                       &message_length,
		                       &return_code);

		if (return_code == NO_ERROR) {
			printf("[P1/Process1] RECEIVE_QUEUING_MESSAGE rc=%d len=%d msg='%s'\n",
			       return_code,
			       message_length,
			       message);
		} else {
			printf("[P1/Process1] RECEIVE_QUEUING_MESSAGE rc=%d\n", return_code);
		}

		TIMED_WAIT(10, &return_code);
	}
}

__attribute__((section(".p1_code")))
void p1_process2(void)
{   
	RETURN_CODE_TYPE return_code;
	printf("[P1/Process2] idle for queuing test\n");

	while (1) {
		TIMED_WAIT(20, &return_code);
	}
}

__attribute__((section(".p1_code")))
void p1_process3(void)
{   
	RETURN_CODE_TYPE return_code;
	printf("[P1/Process3] idle for queuing test\n");

	while (1) {
		TIMED_WAIT(20, &return_code);
	}
}

__attribute__((section(".p2_code")))
void p2_process1(void)
{   
	RETURN_CODE_TYPE return_code;
	APEX_INTEGER partition_id;
	APEX_INTEGER process_id;
	QUEUING_PORT_ID_TYPE queuing_port_id;
	MESSAGE_SIZE_TYPE message_length;
	char message[64];
	uint32_t seq = 0;

	GET_MY_PARTITION_ID(&partition_id, &return_code);
	GET_MY_ID(&process_id, &return_code);

	while (1) {
		GET_QUEUING_PORT_ID("P2_OUT_CMDS", &queuing_port_id, &return_code);
		if (return_code == NO_ERROR) {
			break;
		}
		printf("[P2/Process1] GET_QUEUING_PORT_ID('P2_OUT_CMDS') rc=%d (retry)\n", return_code);
		TIMED_WAIT(2, &return_code);
	}

	printf("[P2/Process1] partition=%d pid=%d queuing_port_id=%d\n", partition_id, process_id, queuing_port_id);

	while (1) {
		seq++;
		sprintf(message, "cmd=%lu from=P2", (unsigned long)seq);
		message_length = (MESSAGE_SIZE_TYPE)(strlen(message) + 1);

		SEND_QUEUING_MESSAGE(queuing_port_id,
		                     (MESSAGE_ADDR_TYPE)message,
		                     message_length,
		                     0,
		                     &return_code);

		if (return_code == NO_ERROR) {
			printf("[P2/Process1] SEND_QUEUING_MESSAGE rc=%d seq=%lu len=%d msg='%s'\n",
			       return_code,
			       (unsigned long)seq,
			       message_length,
			       message);
		} else {
			printf("[P2/Process1] SEND_QUEUING_MESSAGE rc=%d seq=%lu\n",
			       return_code,
			       (unsigned long)seq);
		}

		TIMED_WAIT(10, &return_code);
	}
}

void error_handler_function(void) {
	printf("[ERROR HANDLER] Error handler is executing.\n");
	while (1) {
		// Loop indefinitely to simulate error handling
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
				   P2_CONFIG.max_errors

				   );

	return 1;
}
