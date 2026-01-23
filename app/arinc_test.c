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

// on met la tache dans la section code de la p1
__attribute__((section(".p1_code")))
void task0(void)
{
	int32_t cnt = 100000;

	APEX_INTEGER id;
	RETURN_CODE_TYPE return_code;

	GET_MY_PARTITION_ID(&id, &return_code);

	while (1) {
		printf("[task %d %ld, partition %d]\n", ucx_task_id(), cnt++, id);
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

	GET_MY_PARTITION_ID(&id, &return_code);

	while (1) {
		printf("[task %d %ld, partition %d]\n", ucx_task_id(), cnt++, id);
		ucx_task_yield();
	}
}

int app_main(void)
{
	// la partie data est pour l'instant la stack de la task de l'entry point de P1 donc elle grandit vers le bas
	size_t p1_data_size =  _p1_data_start -_p1_data_end;
	size_t p1_code_size =  _p1_code_end -_p1_code_start;

	// la partie data est pour l'instant la stack de la task de l'entry point de P1 donc elle grandit vers le bas
	size_t p2_data_size =  _p2_data_start -_p2_data_end;
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

	


	// start UCX/OS, preemptive mode
	return 1;
}
