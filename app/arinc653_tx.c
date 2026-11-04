#include <ucx.h>

#define ARINC_TX_DATA   (*(volatile uint32_t *)0x50000000)
#define ARINC_TX_DELAY  (*(volatile uint32_t *)0x50000004)
#define ARINC_TX_SPEED  (*(volatile uint32_t *)0x50000008)

static uint8_t reverse_bits_8(uint8_t b)
{
	b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
	b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
	b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
	return b;
}

static uint8_t calculate_odd_parity(uint32_t word_31bits)
{
	uint32_t temp = word_31bits & 0x7FFFFFFF;
	uint32_t count = 0;
	while (temp) {
		count += (temp & 1);
		temp >>= 1;
	}
	return (count % 2 == 0) ? 1 : 0;
}

static uint32_t build_arinc_word_label312(uint32_t alt_meters)
{
	uint32_t alt_feet = (alt_meters * 32808U) / 10000U;
	uint8_t label_oct_val = 0xCA; // Octal 312 = 0xCA
	uint8_t label_rev = reverse_bits_8(label_oct_val); // 0x53
	uint8_t sdi = 0;
	uint8_t ssm = 2; // Normal Operation (0b10)

	uint32_t data16 = alt_feet & 0xFFFFU;

	uint32_t word = ((uint32_t)label_rev) |
	                (((uint32_t)(sdi & 0x03)) << 8) |
	                ((data16 & 0xFFFF) << 10) |
	                (((uint32_t)(ssm & 0x03)) << 29);

	uint8_t parity = calculate_odd_parity(word);
	word |= (((uint32_t)parity) << 31);
	return word;
}

// ============================================================================
// PARTITION 1 : PROCESSUS 1 : Émetteur ARINC 429 (Label 312 Altitude)
// ============================================================================
__attribute__((section(".p1_code")))
void p1_process1(void)
{
	RETURN_CODE_TYPE return_code;
	uint32_t current_alt_m = 500;
	uint32_t step = 1;

	// Configuration temporelle ARINC 429 (Horloge 25 MHz) :
	// 100 kbps => half_period = 125 cycles, gap = 1000 cycles
	ARINC_TX_SPEED = 125;
	ARINC_TX_DELAY = 1000;

	printf("\n=======================================================\n");
	printf("   UCX-OS ARINC 653 - CORE 0 TX (Label 312 Altitude)   \n");
	printf("   Partition 1 / Process 1 (ARINC 429 Altitude TX)     \n");
	printf("=======================================================\n\n");

	while (1) {
		uint32_t arinc_word = build_arinc_word_label312(current_alt_m);
		uint32_t alt_ft = (current_alt_m * 32808U) / 10000U;

		printf("[CORE 0 ARINC653 TX #%u] Alt = %u m (%u ft) | Word = 0x%08x\n",
		       step, current_alt_m, alt_ft, arinc_word);

		// Écriture matérielle
		ARINC_TX_DATA = arinc_word;

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
