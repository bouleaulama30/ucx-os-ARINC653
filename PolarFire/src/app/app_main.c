#include <ucx.h>

static void print_time_prefix(void)
{
	uint64_t uptime_ms = ucx_uptime();
	uint64_t secs = uptime_ms / 1000;
	uint64_t msecs = uptime_ms % 1000;

	printf("[%lu.%03lu s] ",
		   (unsigned long)secs,
		   (unsigned long)msecs);
}

#define APP_PRINTF(...) do { \
	print_time_prefix(); \
	printf(__VA_ARGS__); \
} while (0)
//void print_time()
//{
//	uint32_t secs, msecs, time;
//
//	time = ucx_uptime();
//	// if (!time_initialized) {
//    //     start_time = time;
//    //     time_initialized = 1;
//    // }
//	// time -= start_time;
//	secs = time / 1000;
//	msecs = time - secs * 1000;
//
//	printf("%ld.%03lds\n", secs, msecs);
//}

// ============================================================================
// PARTITION 1 : PROCESSUS 1 : LE DECLENCHEUR (Périodique, plus lent)
// ============================================================================
__attribute__((section(".p1_code")))
void p1_process1(void) {
    RETURN_CODE_TYPE return_code;
	EVENT_ID_TYPE wake_event_id;
	GET_EVENT_ID("WakeUpEvent", &wake_event_id, &return_code);
	if (return_code != NO_ERROR)
		RAISE_APPLICATION_ERROR(APPLICATION_ERROR, (MESSAGE_ADDR_TYPE)"Simulated error in Process 1", 34, &return_code);
	
    while (1) {
		APP_PRINTF("[P1 Processus 1 - Declencheur] C'est l'heure ! Je reveille le Processus 2 !\n");
        
        // Déclenche l'événement. Le Processus 2 va immédiatement passer à l'état READY
        SET_EVENT(wake_event_id, &return_code);
        
        // S'endort jusqu'à la prochaine période (ex: tous les 100 ticks)
        TIMED_WAIT(0, &return_code);
    }
}



// ============================================================================
// PARTITION 1 : PROCESSUS 2 : LE TRAVAILLEUR (Apériodique, s'endort et attend)
// ============================================================================
__attribute__((section(".p1_code")))
void p1_process2(void) {
    RETURN_CODE_TYPE return_code;
	EVENT_ID_TYPE wake_event_id;
	GET_EVENT_ID("WakeUpEvent", &wake_event_id, &return_code);
	if (return_code != NO_ERROR)
		RAISE_APPLICATION_ERROR(APPLICATION_ERROR, (MESSAGE_ADDR_TYPE)"Simulated error in Process 2", 34, &return_code);

    while (1) {
		APP_PRINTF("[P1 Processus 2 - Travailleur] Je m'endors... zZz...\n");
        
        // Bloque le processus indéfiniment (INFINITE_TIME_VALUE) jusqu'au réveil
        WAIT_EVENT(wake_event_id, INFINITE_TIME_VALUE, &return_code);

        if (return_code == NO_ERROR) {
			APP_PRINTF("[P1 Processus 2 - Travailleur] REVEILLE ! J'execute ma tache lourde...\n");
            
            // On a fini le travail. On réinitialise l'événement 
            // pour pouvoir s'endormir au prochain tour de boucle.
            RESET_EVENT(wake_event_id, &return_code);
        }
    }
}


// ============================================================================
// PARTITION 1 : PROCESSUS 3 : LE CAPTEUR (Périodique, très rapide)
// ============================================================================
__attribute__((section(".p1_code")))
void p1_process3(void) {
    RETURN_CODE_TYPE return_code;
    int sensor_value = 0;

    while (1) {
        sensor_value += 5;
		APP_PRINTF("[P1 Processus 3 - Capteur] Lecture en cours... Valeur = %d\n", sensor_value);
        
        // S'endort jusqu'à la prochaine période (ex: tous les 20 ticks)
        PERIODIC_WAIT(&return_code); 
    }
}

// ============================================================================
// PARTITION 2 : PROCESSUS 1 : IDLE
// ============================================================================
__attribute__((section(".p2_code")))
void p2_process1(void) {
    RETURN_CODE_TYPE return_code;
	    while (1) {
		APP_PRINTF("[P2 Processus 1] Tache Unique avant IDLE\n");
        
		STOP_SELF();
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
				//    test_spatial_violation_p1,
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
