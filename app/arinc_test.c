#include <ucx.h>

// Déclaration des symboles du linker script de la p1
extern uint8_t _p1_code_start[];
extern uint8_t _p1_code_end[];

extern uint8_t _p1_data_start[];
extern uint8_t _p1_data_end[];

// Déclaration des symboles du linker script de la p2
extern uint8_t _p2_code_start[];
extern uint8_t _p2_code_end[];

extern uint8_t _p2_data_start[];
extern uint8_t _p2_data_end[];


// Fonction de test pour les opérations de partition
__attribute__((section(".p1_code")))
void test_partition_operations(void) {
    APEX_INTEGER partition_id;
    RETURN_CODE_TYPE return_code;
    PARTITION_STATUS_TYPE status;
    int test_num = 1;
    int pass = 1;
    
    printf("\n--- START TEST SEQUENCE P1 ---\n");
    
    // TEST 1: GET_MY_PARTITION_ID
    printf("[TEST %d] GET_MY_PARTITION_ID...\n", test_num);
    GET_MY_PARTITION_ID(&partition_id, &return_code);
    printf("-> Expected: 1, Received: %d [%s]\n", 
           partition_id, (partition_id == 1) ? "PASS" : "FAIL");
    test_num++;
    
    // TEST 2: GET_PARTITION_STATUS (Initial)
    printf("[TEST %d] GET_PARTITION_STATUS (Initial)...\n", test_num);
    GET_PARTITION_STATUS(&status, &return_code);
    const char* mode_str = (status.OPERATING_MODE == IDLE) ? "IDLE" : 
                           (status.OPERATING_MODE == NORMAL) ? "NORMAL" : "COLD_START";
    printf("-> Expected: IDLE, Received: %s [%s]\n", 
           mode_str, (status.OPERATING_MODE == IDLE) ? "PASS" : "FAIL");
    test_num++;
    
    // TEST 3: SET_PARTITION_MODE(NORMAL)
    printf("[TEST %d] SET_PARTITION_MODE(NORMAL)...\n", test_num);
    SET_PARTITION_MODE(NORMAL, &return_code);
    printf("-> Return Code: %s [%s]\n", 
           (return_code == NO_ERROR) ? "NO_ERROR" : "ERROR", 
           (return_code == NO_ERROR) ? "PASS" : "FAIL");
    test_num++;
    
    // TEST 4: GET_PARTITION_STATUS (Post-Switch)
    printf("[TEST %d] GET_PARTITION_STATUS (Post-Switch)...\n", test_num);
    GET_PARTITION_STATUS(&status, &return_code);
    mode_str = (status.OPERATING_MODE == IDLE) ? "IDLE" : 
               (status.OPERATING_MODE == NORMAL) ? "NORMAL" : "COLD_START";
    printf("-> Expected: NORMAL, Received: %s [%s]\n", 
           mode_str, (status.OPERATING_MODE == NORMAL) ? "PASS" : "FAIL");
    
    printf("--- END TEST SEQUENCE P1 ---\n\n");
}

__attribute__((section(".p1_code")))
void test_spatial_violation_p2(void) {
    printf("--- Test 2: Tentative d'ecriture sur P2 (0x%08x) ---\n", (unsigned int)_p2_data_start);
    printf("ATTENTION: Le systeme DOIT crasher ou lever une exception maintenant.\n");
    
    volatile int *ptr = (int *)_p2_data_start;
    
    // Si l'isolation matérielle est active, cette ligne stoppe l'exécution
    *ptr = 0xDEADBEEF; 

    // Si on arrive ici, c'est un échec de l'isolation
    printf("[CRITICAL FAIL] P1 a reussi a ecrire dans P2 !\n");
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

	APEX_INTEGER id;
	RETURN_CODE_TYPE return_code;

	GET_MY_PARTITION_ID(&id, &return_code);

	while (1) {
		printf("[task %d %ld, partition %d, address cnt: 0x%p]\n", ucx_task_id(), cnt++, id, &cnt);
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
		printf("[task %d %ld, address cnt: 0x%p ,period=%ld duration=%ld, mode=%d]\n", ucx_task_id(), cnt++, &cnt,(long)status.PERIOD, (long)status.DURATION, status.OPERATING_MODE);
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
		printf("[task %d %ld, partition %d, address cnt: 0x%p]\n", ucx_task_id(), cnt++, id, &cnt);
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
				   task0,
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
				   task1,
				   P2_CONFIG.is_system_partition);

	return 1;
}
