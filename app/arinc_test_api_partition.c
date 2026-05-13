#include <ucx.h>
#include "arinc_test_support.h"

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
	case NOT_AVAILABLE:
		return "NOT_AVAILABLE";
	case NO_ACTION:
		return "NO_ACTION";
	case INVALID_CONFIG:
		return "INVALID_CONFIG";
	case INVALID_MODE:
		return "INVALID_MODE";
	case TIMED_OUT:
		return "TIMED_OUT";
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
static int test_partition_operations(void) {
    arinc_test_suite_result_t suite;
    APEX_INTEGER partition_id;
    RETURN_CODE_TYPE return_code;
    PARTITION_STATUS_TYPE status;
    int pass = 1;
	int case_pass;

	arinc_test_suite_begin(&suite, "partition_api");
    
    // TEST 1: GET_MY_PARTITION_ID
    GET_MY_PARTITION_ID(&partition_id, &return_code);
		printf("[ARINC_TEST] step=after_get_my_partition_id\n");
	case_pass = arinc_test_check_int(1, partition_id);
	case_pass &= arinc_test_check_str(return_code_to_str(NO_ERROR), return_code_to_str(return_code));
    arinc_test_suite_check(&suite, "GET_MY_PARTITION_ID", case_pass);
    pass &= case_pass;

		// TEST 2: GET_PARTITION_STATUS (Initial)
    GET_PARTITION_STATUS(&status, &return_code);
		printf("[ARINC_TEST] step=after_get_partition_status_initial\n");
    case_pass = arinc_test_check_str(operating_mode_to_str(COLD_START), operating_mode_to_str(status.OPERATING_MODE));
    case_pass &= arinc_test_check_str(return_code_to_str(NOT_AVAILABLE), return_code_to_str(return_code));
    arinc_test_suite_check(&suite, "GET_PARTITION_STATUS(initial)", case_pass);
    pass &= case_pass;

	arinc_test_suite_end(&suite);
	printf("[ARINC_TEST] completed suite partition_api\n");

	return pass;
}

// Fonction de test dédiée aux cas sûrs de SET_PARTITION_MODE en COLD_START.
// Les transitions vers NORMAL/IDLE/WARM_START ne peuvent pas être testées ici
// car elles redonnent le contrôle au scheduler via longjmp().
__attribute__((section(".p1_code")))
static int test_set_partition_mode(void)
{
	arinc_test_suite_result_t suite;
	RETURN_CODE_TYPE return_code;
	int pass = 1;
	int case_pass;

	arinc_test_suite_begin(&suite, "set_partition_mode_cold_start");

	// TEST 1: Invalid parameter
	SET_PARTITION_MODE((OPERATING_MODE_TYPE)99, &return_code);
	printf("[ARINC_TEST] step=after_set_partition_mode_invalid_param\n");
	case_pass = arinc_test_check_str(return_code_to_str(INVALID_PARAM), return_code_to_str(return_code));
	arinc_test_suite_check(&suite, "SET_PARTITION_MODE(INVALID_PARAM)", case_pass);
	pass &= case_pass;

	// TEST 2: WARM_START is invalid while the partition is still in COLD_START.
	SET_PARTITION_MODE(WARM_START, &return_code);
	printf("[ARINC_TEST] step=after_set_partition_mode_warm_start\n");
	case_pass = arinc_test_check_str(return_code_to_str(INVALID_MODE), return_code_to_str(return_code));
	arinc_test_suite_check(&suite, "SET_PARTITION_MODE(WARM_START from COLD_START)", case_pass);
	pass &= case_pass;

	arinc_test_suite_end(&suite);
	printf("[ARINC_TEST] completed suite set_partition_mode_cold_start\n");

	return pass;
}

__attribute__((section(".p1_code")))
static void run_partition_api_suite(void)
{
	(void)test_partition_operations();
	(void)test_set_partition_mode();

	RETURN_CODE_TYPE return_code;
	SET_PARTITION_MODE(IDLE, &return_code);
}
int app_main(void)
{
	// la partie data est pour l'instant la stack de la task de l'entry point de P1 donc elle grandit vers le bas
	size_t p1_data_size =  _p1_data_end -_p1_data_start;
	size_t p1_code_size =  _p1_code_end -_p1_code_start;
	SYSTEM_TIME_TYPE test_duration = DEFAULT_PARTITION_CONFIG.duration * 8;

	partition_init(DEFAULT_PARTITION_CONFIG.period,
				   test_duration,
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
				   run_partition_api_suite,
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

	return 1;
}
