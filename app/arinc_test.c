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
	GET_PROCESS_STATUS(1, &other_process_status, &return_code);
	
	if(return_code == NO_ERROR){
		printf("Le nom du process: %s, with priority %d\n", other_process_status.ATTRIBUTES.NAME, other_process_status.CURRENT_PRIORITY);
	}
	while (1) {
		// if(cnt % 2 == 0){
		// 	printf("[process %d %ld, partition %d, address cnt: 0x%p]\n\n", process_id, cnt++, paritition_id, &cnt);
		// 	SET_PRIORITY(1, 3, &return_code);
		// 	// SET_PRIORITY(1, 3, &return_code);

		// }
		
		if(cnt == 100001)
			SET_PARTITION_MODE(IDLE, &return_code);
			// STOP(1, &return_code);
		// if(cnt == 100005)
		// 	DELAYED_START(1, 50 ,&return_code);

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
	int32_t cnt = 200000;
    RETURN_CODE_TYPE return_code;
	APEX_INTEGER paritition_id;
    APEX_INTEGER process_id;
	GET_MY_PARTITION_ID(&paritition_id, &return_code);
    GET_MY_ID(&process_id, &return_code);
	while (1) {
		// if(cnt % 2 == 0){
		// 	printf("[process %d %ld, partition %d, address cnt: 0x%p]\n\n", process_id, cnt++, paritition_id, &cnt);
		// 	SET_PRIORITY(1, 1, &return_code);
		// 	// SET_PRIORITY(0, 3, &return_code);
		// }

		// if(cnt == 200001)
		// 	STOP(0, &return_code);
		// if(cnt == 200003)
		// 	DELAYED_START(0, 90 ,&return_code);

		printf("[process %d %ld, partition %d, address cnt: 0x%p]\n\n", process_id, cnt++, paritition_id, &cnt);
		TIMED_WAIT(5, &return_code);
		// ucx_task_yield();
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
		if(cnt % 2 == 0){
			// SET_PARTITION_MODE(IDLE, &return_code);
			printf("[process %d %ld, partition %d, address cnt: 0x%p]\n\n", process_id, cnt++, paritition_id, &cnt);
			SET_PRIORITY(1, 3, &return_code);
			// SET_PRIORITY(1, 3, &return_code);

		}
		
		if(cnt == 300001)
			SUSPEND(1, &return_code);
			// SET_PARTITION_MODE(COLD_START, &return_code);
		if(cnt == 300005)
			RESUME(1, &return_code);
		printf("[process %d %ld, partition %d, address cnt: 0x%p]\n\n", process_id, cnt++, paritition_id, &cnt);

		TIMED_WAIT(5, &return_code);
		// ucx_task_yield();

	}
}

__attribute__((section(".p2_code")))
void process_test3(void)
{   
	int32_t cnt = 400000;
    RETURN_CODE_TYPE return_code;
	APEX_INTEGER paritition_id;
    APEX_INTEGER process_id;
	GET_MY_PARTITION_ID(&paritition_id, &return_code);
    GET_MY_ID(&process_id, &return_code);
	while (1) {
		if(cnt % 2 == 0){
			printf("[process %d %ld, partition %d, address cnt: 0x%p]\n\n", process_id, cnt++, paritition_id, &cnt);
			SET_PRIORITY(1, 1, &return_code);
			// SET_PRIORITY(0, 3, &return_code);
		}
		printf("[process %d %ld, partition %d, address cnt: 0x%p]\n\n", process_id, cnt++, paritition_id, &cnt);
		TIMED_WAIT(5, &return_code);
		// ucx_task_yield();

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
