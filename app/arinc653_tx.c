#include <ucx.h>

// ============================================================================
// PARTITION 1 : PROCESSUS 1 : Émetteur Altitude (via Sampling Port APEX)
// L'encapsulation ARINC 429 (Label 312, SDI, SSM, Parité) est intégralement
// déportée en matériel dans le module VHDL `arinc429_encap_decap.vhd`.
// Si le bus matériel est remplacé par de l'AFDX, ce code reste 100% identique.
// ============================================================================
__attribute__((section(".p1_code")))
void p1_process1(void)
{
	RETURN_CODE_TYPE return_code;
	SAMPLING_PORT_ID_TYPE port_id;
	uint32_t current_alt_m = 500;
	uint32_t step = 1;

	GET_SAMPLING_PORT_ID("P1_OUT_ALT", &port_id, &return_code);

	printf("\n=======================================================\n");
	printf("   UCX-OS ARINC 653 - CORE 0 TX (Sampling Port APEX)   \n");
	printf("   Port : P1_OUT_ALT (ID=%d, Encapsulation VHDL)       \n", (int)port_id);
	printf("=======================================================\n\n");

	while (1) {
		WRITE_SAMPLING_MESSAGE(port_id,
		                       (MESSAGE_ADDR_TYPE)&current_alt_m,
		                       sizeof(current_alt_m),
		                       &return_code);

		printf("[CORE 0 ARINC653 TX #%u] WRITE_SAMPLING_MESSAGE(P1_OUT_ALT) -> Alt = %u m (Ret=%d)\n",
		       step, current_alt_m, (int)return_code);

		current_alt_m += 50;
		if (current_alt_m > 5000) {
			current_alt_m = 100;
		}
		step++;

		// Temporisation gérée par le RTOS APEX ARINC 653 (100 ticks = ~1 sec)
		TIMED_WAIT(100, &return_code);
	}
}

// ============================================================================
// PARTITION 2 : PROCESSUS 1 : Surveillance / Télémétrie de bord
// ============================================================================
__attribute__((section(".p2_code")))
void p2_process1(void)
{
	RETURN_CODE_TYPE return_code;
	uint32_t mon_count = 0;

	while (1) {
		mon_count++;
		printf("[CORE 0 ARINC653 P2] Telemetry Monitor Heartbeat #%u (OK)\n", mon_count);
		TIMED_WAIT(300, &return_code); // 300 ticks = ~3 sec
	}
}

void error_handler_function(void)
{
	STOP_SELF();
}

int app_main(void)
{
	size_t p1_data_size = _p1_data_end - _p1_data_start;
	size_t p1_code_size = _p1_code_end - _p1_code_start;
	size_t p2_data_size = _p2_data_end - _p2_data_start;
	size_t p2_code_size = _p2_code_end - _p2_code_start;

	partition_init(DEFAULT_PARTITION_CONFIG.period,
	               DEFAULT_PARTITION_CONFIG.duration,
	               DEFAULT_PARTITION_CONFIG.identifier,
	               DEFAULT_PARTITION_CONFIG.num_assigned_cores,
	               DEFAULT_PARTITION_CONFIG.name,
	               DEFAULT_PARTITION_CONFIG.region_name_code_mem,
	               (void*)_p1_code_start,
	               p1_code_size,
	               DEFAULT_PARTITION_CONFIG.access_code_mem,
	               DEFAULT_PARTITION_CONFIG.region_name_data_mem,
	               (void*)_p1_data_start,
	               p1_data_size,
	               DEFAULT_PARTITION_CONFIG.access_data_mem,
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
	               DEFAULT_PARTITION_CONFIG.max_errors);

	partition_init(P2_CONFIG.period,
	               P2_CONFIG.duration,
	               P2_CONFIG.identifier,
	               P2_CONFIG.num_assigned_cores,
	               P2_CONFIG.name,
	               P2_CONFIG.region_name_code_mem,
	               (void*)_p2_code_start,
	               p2_code_size,
	               P2_CONFIG.access_code_mem,
	               P2_CONFIG.region_name_data_mem,
	               (void*)_p2_data_start,
	               p2_data_size,
	               P2_CONFIG.access_data_mem,
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
	               P2_CONFIG.max_errors);

	return 1;
}
