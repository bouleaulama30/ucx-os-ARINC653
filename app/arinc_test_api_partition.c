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

__attribute__((section(".p1_code")))
static const char *return_code_to_str(RETURN_CODE_TYPE rc)
{
	switch (rc) {
	case NO_ERROR:
		return "NO_ERROR";
	case INVALID_PARAM:
		return "INVALID_PARAM";
	case NO_ACTION:
		return "NO_ACTION";
	case INVALID_MODE:
		return "INVALID_MODE";
	default:
		return "UNKNOWN";
	}
}

__attribute__((section(".p1_code")))
static const char *operating_mode_to_str(OPERATING_MODE_TYPE mode)
{
	switch (mode) {
	case IDLE:
		return "IDLE";
	case COLD_START:
		return "COLD_START";
	case WARM_START:
		return "WARM_START";
	case NORMAL:
		return "NORMAL";
	default:
		return "UNKNOWN";
	}
}


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

// Fonction de test dédiée à SET_PARTITION_MODE
__attribute__((section(".p1_code")))
void test_set_partition_mode(void)
{
	RETURN_CODE_TYPE return_code;
	PARTITION_STATUS_TYPE status;
	int test_num = 1;

	printf("\n--- START TEST SET_PARTITION_MODE ---\n");

	// TEST 1: Invalid parameter
	printf("[TEST %d] SET_PARTITION_MODE(INVALID_PARAM)...\n", test_num++);
	SET_PARTITION_MODE((OPERATING_MODE_TYPE)99, &return_code);
	printf("-> Expected: INVALID_PARAM, Received: %s [%s]\n",
		   return_code_to_str(return_code),
		   (return_code == INVALID_PARAM) ? "PASS" : "FAIL");

	// TEST 2: Set to NORMAL from IDLE/COLD_START
	printf("[TEST %d] SET_PARTITION_MODE(NORMAL)...\n", test_num++);
	SET_PARTITION_MODE(NORMAL, &return_code);
	printf("-> Expected: NO_ERROR, Received: %s [%s]\n",
		   return_code_to_str(return_code),
		   (return_code == NO_ERROR) ? "PASS" : "FAIL");
	GET_PARTITION_STATUS(&status, &return_code);
	printf("-> Expected Mode: NORMAL, Received: %s [%s]\n",
		   operating_mode_to_str(status.OPERATING_MODE),
		   (status.OPERATING_MODE == NORMAL) ? "PASS" : "FAIL");

	// TEST 3: Set to NORMAL again (NO_ACTION)
	printf("[TEST %d] SET_PARTITION_MODE(NORMAL) again...\n", test_num++);
	SET_PARTITION_MODE(NORMAL, &return_code);
	printf("-> Expected: NO_ACTION, Received: %s [%s]\n",
		   return_code_to_str(return_code),
		   (return_code == NO_ACTION) ? "PASS" : "FAIL");

	// TEST 4: Set to COLD_START then WARM_START (INVALID_MODE)
	printf("[TEST %d] SET_PARTITION_MODE(COLD_START)...\n", test_num++);
	SET_PARTITION_MODE(COLD_START, &return_code);
	printf("-> Expected: NO_ERROR, Received: %s [%s]\n",
		   return_code_to_str(return_code),
		   (return_code == NO_ERROR) ? "PASS" : "FAIL");

	printf("[TEST %d] SET_PARTITION_MODE(WARM_START) from COLD_START...\n", test_num++);
	SET_PARTITION_MODE(WARM_START, &return_code);
	printf("-> Expected: INVALID_MODE, Received: %s [%s]\n",
		   return_code_to_str(return_code),
		   (return_code == INVALID_MODE) ? "PASS" : "FAIL");

	// Restore to IDLE for subsequent tests
	SET_PARTITION_MODE(IDLE, &return_code);

	printf("--- END TEST SET_PARTITION_MODE ---\n\n");
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
				   test_set_partition_mode,
				   DEFAULT_PARTITION_CONFIG.is_system_partition);

	return 1;
}
