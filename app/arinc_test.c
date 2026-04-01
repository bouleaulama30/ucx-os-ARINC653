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
void process_test0(void)
{   
	int32_t cnt = 100000;
    RETURN_CODE_TYPE return_code;
	APEX_INTEGER paritition_id;
    APEX_INTEGER process_id;
	
    // APEX_INTEGER other_process_id;
	PROCESS_STATUS_TYPE other_process_status;
	GET_MY_PARTITION_ID(&paritition_id, &return_code);
    GET_MY_ID(&process_id, &return_code);

	SAMPLING_PORT_ID_TYPE sampling_port_id;
	SAMPLING_PORT_STATUS_TYPE sampling_port_status;

	QUEUING_PORT_ID_TYPE queuing_port_id;
	QUEUING_PORT_STATUS_TYPE queuing_port_status;

	GET_SAMPLING_PORT_ID("P1_OUT_TEMP", &sampling_port_id, &return_code);
	printf("PORT SAMPLING ID %d\n", sampling_port_id);

	GET_SAMPLING_PORT_STATUS(1, &sampling_port_status, &return_code);
	printf("PORT SAMPLING NAME FROM STATUS %d\n", sampling_port_status.MAX_MESSAGE_SIZE);
	
	GET_QUEUING_PORT_ID("P1_IN_CMDS", &queuing_port_id, &return_code);
	printf("PORT QUEUING ID %d\n", queuing_port_id);

	GET_QUEUING_PORT_STATUS(1, &queuing_port_status, &return_code);
	printf("PORT QUEUING NAME FROM STATUS %d\n", queuing_port_status.MAX_MESSAGE_SIZE);

	while (1) {
		// if(cnt % 2 == 0){
		// 	printf("[process %d %ld, partition %d, address cnt: 0x%p]\n\n", process_id, cnt++, paritition_id, &cnt);
		// 	SET_PRIORITY(1, 3, &return_code);
		// 	// SET_PRIORITY(1, 3, &return_code);

		// }
		
		// if(cnt == 100001)
		// 	SET_PARTITION_MODE(IDLE, &return_code);
			// STOP(1, &return_code);
		if(cnt == 100005){
			GET_PROCESS_STATUS(0, &other_process_status, &return_code);
			printf("CURRENT DEADLINE OF PROCESS %s: %d\n", other_process_status.ATTRIBUTES.NAME, other_process_status.DEADLINE_TIME);
			REPLENISH(10, &return_code);
			GET_PROCESS_STATUS(0, &other_process_status, &return_code);
			printf("NEW DEADLINE OF PROCESS %s: %d\n", other_process_status.ATTRIBUTES.NAME, other_process_status.DEADLINE_TIME);
			// DELAYED_START(1, 50 ,&return_code);
		}

		// if(cnt == 100011){
		// 	printf("SUSPEND TIME %d\n", ucx_uptime());
		// 	SUSPEND_SELF(50, &return_code);
		// 	printf("return code suspend_self %d:", return_code);
		// }
		printf("[process %d %ld, partition %d, address cnt: 0x%p]\n\n", process_id, cnt++, paritition_id, &cnt);
		TIMED_WAIT(5, &return_code);
		// ucx_task_yield();
	}
}

__attribute__((section(".p1_code")))
void process_test1(void)
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

	printf("[P1/Process1] GET_SAMPLING_PORT_ID('P1_OUT_TEMP') rc=%d id=%d\n", return_code, port_id);

	tx_msg.seq = 0;
	while (1) {
		tx_msg.seq++;
		tx_msg.sent_at_ms = ucx_uptime();
		tx_msg.sender_pid = (uint32_t)process_id;
		write_len = sizeof(tx_msg);

		WRITE_SAMPLING_MESSAGE(port_id, (MESSAGE_ADDR_TYPE)&tx_msg, write_len, &return_code);
		printf("[P1/Process1] WRITE_SAMPLING_MESSAGE seq=%lu len=%d rc=%d %s\n",
			   tx_msg.seq,
			   write_len,
			   return_code,
			   (return_code == NO_ERROR) ? "PASS" : "FAIL");

		PERIODIC_WAIT(&return_code);
	}
}

__attribute__((section(".p2_code")))
void process_test2(void)
{   
int32_t cnt = 300000;
    RETURN_CODE_TYPE return_code;
	APEX_INTEGER paritition_id;
    APEX_INTEGER process_id;
	
    // APEX_INTEGER other_process_id;
	PROCESS_STATUS_TYPE other_process_status;
	GET_MY_PARTITION_ID(&paritition_id, &return_code);
    GET_MY_ID(&process_id, &return_code);
	GET_PROCESS_STATUS(1, &other_process_status, &return_code);
	
	if(return_code == NO_ERROR){
		printf("Le nom du process: %s, with priority %d\n", other_process_status.ATTRIBUTES.NAME, other_process_status.CURRENT_PRIORITY);
	}
	while (1) {

		

		printf("[process %d %ld, partition %d, address cnt: 0x%p]\n\n", process_id, cnt++, paritition_id, &cnt);

		TIMED_WAIT(5, &return_code);
		// ucx_task_yield();

	}
}

__attribute__((section(".p2_code")))
void process_test3(void)
{   
	RETURN_CODE_TYPE return_code;
	APEX_INTEGER partition_id;
	APEX_INTEGER process_id;
	SAMPLING_PORT_ID_TYPE port_id;
	MESSAGE_SIZE_TYPE read_len;
	VALIDITY_TYPE validity;
	uint32_t last_seq = 0;

	struct {
		uint32_t seq;
		uint32_t sent_at_ms;
		uint32_t sender_pid;
	} rx_msg;

	GET_MY_PARTITION_ID(&partition_id, &return_code);
	GET_MY_ID(&process_id, &return_code);
	GET_SAMPLING_PORT_ID("P2_IN_TEMP", &port_id, &return_code);

	printf("[P2/Process3] GET_SAMPLING_PORT_ID('P2_IN_TEMP') rc=%d id=%d\n", return_code, port_id);

	while (1) {
		read_len = 0;
		READ_SAMPLING_MESSAGE(port_id, (MESSAGE_ADDR_TYPE)&rx_msg, &read_len, &validity, &return_code);

		if (return_code == NO_ACTION) {
			printf("[P2/Process3] READ_SAMPLING_MESSAGE rc=%d (pas encore de message)\n", return_code);
		} else if (return_code == NO_ERROR) {
			int size_ok = (read_len == sizeof(rx_msg));
			int seq_ok = (rx_msg.seq >= last_seq);
			printf("[P2/Process3] READ_SAMPLING_MESSAGE rc=%d len=%d validity=%d seq=%lu sent_at=%lu sender=%lu %s\n",
				   return_code,
				   read_len,
				   validity,
				   rx_msg.seq,
				   rx_msg.sent_at_ms,
				   rx_msg.sender_pid,
				   (size_ok && seq_ok) ? "PASS" : "FAIL");
			last_seq = rx_msg.seq;
		} else {
			printf("[P2/Process3] READ_SAMPLING_MESSAGE rc=%d FAIL\n", return_code);
		}

		PERIODIC_WAIT(&return_code);
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
				   DEFAULT_PARTITION_CONFIG.is_system_partition);

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
				   P2_CONFIG.is_system_partition);

	return 1;
}
