#include <ucx.h>

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

int app_main(void)
{
	partition_init(DEFAULT_PARTITION_CONFIG.period,
				   DEFAULT_PARTITION_CONFIG.duration,
				   DEFAULT_PARTITION_CONFIG.identifier,
				   DEFAULT_PARTITION_CONFIG.num_assigned_cores,
				   DEFAULT_PARTITION_CONFIG.name,
				   DEFAULT_PARTITION_CONFIG.region_name_code_mem,
				   DEFAULT_PARTITION_CONFIG.size_code_mem,
				   DEFAULT_PARTITION_CONFIG.access_code_mem,
				   DEFAULT_PARTITION_CONFIG.region_name_data_mem,
				   DEFAULT_PARTITION_CONFIG.size_data_mem,
				   DEFAULT_PARTITION_CONFIG.access_data_mem,
				   task0,
				   DEFAULT_PARTITION_CONFIG.is_system_partition);

	
	printf("task0 has id %d\n", ucx_task_idref(task0));


	// start UCX/OS, preemptive mode
	return 1;
}
