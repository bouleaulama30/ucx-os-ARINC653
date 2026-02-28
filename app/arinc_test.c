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
void test_id_1(void)
{
	uint64_t start = ucx_uptime();
	uint64_t end;
	APEX_INTEGER id;
	RETURN_CODE_TYPE return_code;

	
	while (1) {
		end = ucx_uptime();
		GET_MY_PARTITION_ID(&id, &return_code);
		printf("partition %d, time %lu ms]\n", id, (unsigned long)(end - start));
		start = end;
		ucx_task_yield();
	}
}

__attribute__((section(".p2_code")))
void test_id_2(void)
{
	uint64_t start = ucx_uptime();
	uint64_t end;
	APEX_INTEGER id;
	RETURN_CODE_TYPE return_code;

	
	while (1) {
		end = ucx_uptime();
		GET_MY_PARTITION_ID(&id, &return_code);
		printf("partition %d, time %lu ms]\n", id, (unsigned long)(end - start));
		start = end;
		ucx_task_yield();
	}
}



// on met la tache dans la section code de la p1
__attribute__((section(".p1_code")))
void task0(void)
{   
	int32_t cnt = 100000;
    RETURN_CODE_TYPE return_code;
	APEX_INTEGER id;
	GET_MY_PARTITION_ID(&id, &return_code);
	

	while (1) {
		if(cnt == 100002){
			// SET_PARTITION_MODE(IDLE, &return_code);
		}
		printf("[task %d %ld, partition %d, address cnt: 0x%p]\n\n", id, cnt++, id, &cnt);
		// print_time();
		ucx_task_yield();
	}
}

// on met la tache dans la section code de la p2
__attribute__((section(".p2_code")))
void task1(void)
{
	int32_t cnt = 200000;
	
	APEX_INTEGER id;
	RETURN_CODE_TYPE return_code;
	
	PARTITION_STATUS_TYPE status;
	
	GET_MY_PARTITION_ID(&id, &return_code);
	SET_PARTITION_MODE(NORMAL, &return_code);
	GET_PARTITION_STATUS(&status, &return_code);
	
	while (1) {
		printf("[task %d %ld, address cnt: 0x%p ,period=%ld duration=%ld, mode=%d]\n\n", id, cnt++, &cnt,(long)status.PERIOD, (long)status.DURATION, status.OPERATING_MODE);
		// print_time();
		ucx_task_yield();
	}
}

void task2(void)
{
	int32_t cnt = 300000;

	APEX_INTEGER id;
	RETURN_CODE_TYPE return_code;

	GET_MY_PARTITION_ID(&id, &return_code);

	while (1) {
		printf("[task %d %ld, partition %d, address cnt: 0x%p]\n", id, cnt++, id, &cnt);
		ucx_task_yield();
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
				   main_process,
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
				   main_process,
				   P2_CONFIG.is_system_partition);

	return 1;
}
