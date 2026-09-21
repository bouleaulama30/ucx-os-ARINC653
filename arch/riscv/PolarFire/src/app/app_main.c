#include <ucx.h>

#define SHARED_MEM_ADDR  0x08060000U

typedef struct {
    volatile uint32_t magic_header;     // 0x4152494E ("ARIN")
    volatile uint32_t raw_arinc_word;   // Mot ARINC 429 32-bit brut
    volatile uint32_t altitude_meters;  // Altitude mesurée en mètres (0 à 5000m)
    volatile uint32_t altitude_feet;    // Altitude mesurée en pieds (0 à 16404 ft)
    volatile uint32_t flight_phase;     // Phase de vol (0: Montée, 1: Stationnaire, 2: Descente)
    volatile uint32_t update_counter;  // Incrémenté à chaque trame reçue
    volatile uint32_t status_flags;     // Bit 0: Valide, Bit 1: Parité OK, Bit 2: Label 312 OK
    volatile uint64_t timestamp_ticks;  // Horodatage matériel (mtime)
} shared_altitude_data_t;

#define g_shared_altitude (*(shared_altitude_data_t*)SHARED_MEM_ADDR)

typedef struct {
    uint32_t raw_arinc_word;   // Mot ARINC 429 32-bit
    uint32_t altitude_meters;  // Altitude en mètres
    uint32_t altitude_feet;    // Altitude en pieds
    uint32_t flight_phase;     // Phase de vol (0: Montée, 1: Stationnaire, 2: Descente)
    uint32_t update_counter;  // Compteur d'incrémentation
    uint32_t timestamp_ms;     // Up time en millisecondes
} altitude_telemetry_msg_t;

static const char* const g_phase_names[] = {
    "MONTEE",
    "HOVER",
    "DESCENTE"
};

// ============================================================================
// PARTITION 1 : PROCESSUS 1 : Lecture IPC & Ecriture dans le Sampling Port
// ============================================================================
__attribute__((section(".p1_code")))
void p1_process1(void) {
    RETURN_CODE_TYPE return_code;
    SAMPLING_PORT_ID_TYPE port_id;

    printf("[P1 Init] Verification Port P1_OUT_ALT...\n");

    while (1) {
        GET_SAMPLING_PORT_ID("P1_OUT_ALT", &port_id, &return_code);
        if (return_code == NO_ERROR) {
            break;
        }
        TIMED_WAIT(2, &return_code);
    }

    while (1) {
        // Core 2 L1 Cache Fence to ensure fresh L2 LIM read from Core 1 SPI Generator
        asm volatile ("fence rw, rw" ::: "memory");

        if (g_shared_altitude.magic_header == 0x4152494EU) {
            altitude_telemetry_msg_t msg;
            msg.raw_arinc_word  = g_shared_altitude.raw_arinc_word;
            msg.altitude_meters = g_shared_altitude.altitude_meters;
            msg.altitude_feet   = g_shared_altitude.altitude_feet;
            msg.flight_phase    = g_shared_altitude.flight_phase;
            msg.update_counter  = g_shared_altitude.update_counter;
            msg.timestamp_ms    = (uint32_t)ucx_uptime();

            WRITE_SAMPLING_MESSAGE(port_id, (MESSAGE_ADDR_TYPE)&msg, sizeof(msg), &return_code);

            if (return_code == NO_ERROR) {
                printf("[P1] IPC->Port OK | Alt: %u m | Count: %u\n",
                       (unsigned int)msg.altitude_meters, (unsigned int)msg.update_counter);
            }
        }

        PERIODIC_WAIT(&return_code);
    }
}

// ============================================================================
// PARTITION 2 : PROCESSUS 1 : Lecture du Sampling Port, Interprétation & Retour UART
// ============================================================================
__attribute__((section(".p2_code")))
void p2_process1(void) {
    RETURN_CODE_TYPE return_code;
    SAMPLING_PORT_ID_TYPE port_id;
    MESSAGE_SIZE_TYPE message_length;
    VALIDITY_TYPE validity;
    altitude_telemetry_msg_t rx_msg;

    printf("[P2 Init] Verification Port P2_IN_ALT...\n");

    while (1) {
        GET_SAMPLING_PORT_ID("P2_IN_ALT", &port_id, &return_code);
        if (return_code == NO_ERROR) {
            break;
        }
        TIMED_WAIT(2, &return_code);
    }

    while (1) {
        message_length = sizeof(rx_msg);
        READ_SAMPLING_MESSAGE(port_id, (MESSAGE_ADDR_TYPE)&rx_msg, &message_length, &validity, &return_code);

        if (return_code == NO_ERROR && validity == VALID) {
            uint32_t phase = rx_msg.flight_phase;
            const char* phase_str = (phase <= 2) ? g_phase_names[phase] : "UNK";

            printf("[P2] READ OK | Alt: %u m (%u ft) | Phase: %s | ARINC: 0x%08X | Trame: #%u | %u ms\n",
                   (unsigned int)rx_msg.altitude_meters,
                   (unsigned int)rx_msg.altitude_feet,
                   phase_str,
                   (unsigned int)rx_msg.raw_arinc_word,
                   (unsigned int)rx_msg.update_counter,
                   (unsigned int)rx_msg.timestamp_ms);
        }

        PERIODIC_WAIT(&return_code);
    }
}


int app_main(void)
{
	printf("\r\n=======================================================\r\n");
	printf("  UCX-OS ARINC 653 RTOS - Optimized Telemetry App      \r\n");
	printf("  Execution Hart / Core ID : %u (U54_%u)              \r\n", (unsigned int)_cpu_id(), (unsigned int)_cpu_id());
	printf("=======================================================\r\n\r\n");

	size_t p1_data_size =  _p1_data_end -_p1_data_start;
	size_t p1_code_size =  _p1_code_end -_p1_code_start;

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
				   DEFAULT_PARTITION_CONFIG.max_errors
				   );

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
				   P2_CONFIG.max_errors
				   );

	return 1;
}
