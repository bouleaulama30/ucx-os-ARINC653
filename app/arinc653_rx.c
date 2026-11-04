#include <ucx.h>

#define ARINC_RX_DATA     (*(volatile uint32_t *)0x50000010)
#define ARINC_RX_STATUS   (*(volatile uint32_t *)0x50000014)
#define ARINC_RX_LATCHED  (*(volatile uint32_t *)0x50000018)

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

// ============================================================================
// PARTITION 1 : PROCESSUS 1 : Récepteur et Décodeur ARINC 429
// ============================================================================
__attribute__((section(".p1_code")))
void p1_process1(void)
{
	RETURN_CODE_TYPE return_code;
	uint32_t count = 0;

	printf("\n=======================================================\n");
	printf("   UCX-OS ARINC 653 - CORE 1 RX (Continuous Polling)   \n");
	printf("   Partition 1 / Process 1 (ARINC 429 Altitude RX)     \n");
	printf("=======================================================\n\n");

	while (1) {
		count++;
		uint32_t raw_word     = ARINC_RX_DATA;
		uint32_t status       = ARINC_RX_STATUS;
		uint32_t latched_word = ARINC_RX_LATCHED;

		printf("[CORE 1 ARINC653 RX #%u] DATA=0x%08x | STATUS=0x%08x | LATCHED=0x%08x\n",
		       count, raw_word, status, latched_word);

		// Si un mot est présent (direct ou verrouillé)
		uint32_t active_word = (raw_word != 0U) ? raw_word : latched_word;

		if (active_word != 0U) {
			uint8_t rx_label_raw    = active_word & 0xFFU;
			uint8_t rx_label_oct    = reverse_bits_8(rx_label_raw);
			uint16_t rx_alt_feet    = (uint16_t)((active_word >> 10) & 0xFFFFU);
			uint32_t rx_alt_meters  = ((uint32_t)rx_alt_feet * 10000U) / 32808U;
			uint8_t computed_parity = calculate_odd_parity(active_word & 0x7FFFFFFF);
			uint8_t actual_parity   = (uint8_t)((active_word >> 31) & 0x01U);

			printf("  >>> [ARINC 653] RECU MOT ARINC 429 VALIDE <<<\n");
			printf("      Mot Brut          = 0x%08x\n", active_word);
			printf("      Label Octal       = %03o (raw: 0x%02x)\n", rx_label_oct, rx_label_raw);
			printf("      Altitude Decodee  = %u m (%u ft)\n", rx_alt_meters, rx_alt_feet);
			printf("      Parite            = %s\n\n", (computed_parity == actual_parity) ? "OK (ODD)" : "FAIL");
		}

		// Temporisation gérée par le RTOS APEX ARINC 653 (50 ticks = ~500 ms)
		TIMED_WAIT(50, &return_code);
	}
}

// ============================================================================
// PARTITION 2 : PROCESSUS 1 : Afficheur instruments de vol
// ============================================================================
__attribute__((section(".p2_code")))
void p2_process1(void)
{
	RETURN_CODE_TYPE return_code;
	uint32_t display_count = 0;

	while (1) {
		display_count++;
		printf("[CORE 1 ARINC653 P2] Primary Flight Display (PFD) Active #%u\n", display_count);
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
