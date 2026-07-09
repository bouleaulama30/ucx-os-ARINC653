/* __________________________________________________________________________
* DEVELOPMENT HISTORY:
* --------------------
*
* $Author:: $: Felipe Gohring de Magalhaes
* $Rev:: $: Revision of last commit
* $Date:: $: 10/25/2018
* $Comments (Refer to applicable SCR/PR) $: Initial version of the file.
* __________________________________________________________________________
*/
/* __________________________________________________________________________
* MODULE DESCRIPTION:
* -------------------
* Filename : P1_benchmark.c
* Original Author : Felipe Gohring de Magalhaes
*
* This module contains partition 1 main function.
*
* ASSUMPTIONS, CONSTRAINTS OR LIMITATIONS:
* ----------------------------------------
* 
*
* REFERENCES:
* -----------
* None.
* __________________________________________________________________________
*/
/* INCLUDE SECTION */
/*Include here any system specific lib that is not with performance_lib.h*/
#if defined(TEST_PERF1)

#include "p1_benchmark_perf1.h"

#elif defined(TEST_PERF2)

#include "p1_benchmark_perf2.h"

#elif defined(TEST_PERF3)

#include "p1_benchmark_perf3.h"

#elif defined(TEST_PERF4)

#include "p1_benchmark_perf4.h"

#elif defined(TEST_PERF5)

#include "p1_benchmark_perf5.h"

#elif defined(TEST_PERF6)

#include "p1_benchmark_perf6.h"

#elif defined(TEST_PERF7)

#include "p1_benchmark_perf7.h"

#elif defined(TEST_PERF8)

#include "p1_benchmark_perf8.h"

#elif defined(TEST_PERF9)

#include "p1_benchmark_perf9.h"

#elif defined(TEST_PERF10)

#include "p1_benchmark_perf10.h"

#elif defined(TEST_PERF11)

#include "p1_benchmark_perf11.h"

#elif defined(TEST_PERF12)

#include "p1_benchmark_perf12.h"

#elif defined(TEST_PERF13)

#include "p1_benchmark_perf13.h"
#include "p2_benchmark_perf13.h"

#elif defined(TEST_PERF14)

#include "p1_benchmark_perf14.h"

#elif defined(TEST_PERF15)

#include "p1_benchmark_perf15.h"

#elif defined(TEST_PERF16)

#include "p1_benchmark_perf16.h"

#elif defined(TEST_PERF17)

#include "p1_benchmark_perf17.h"

#elif defined(TEST_PERF18)

#include "p1_benchmark_perf18.h"

#elif defined(TEST_PERF19)

#include "p1_benchmark_perf19.h"
#include "p2_benchmark_perf19.h"

#endif

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
                   MAIN_FUNCTION,
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

    #if defined(TEST_PERF13) || defined(TEST_PERF19)

     partition_init(P2_CONFIG.period,
                   P2_CONFIG.duration,
                   P2_CONFIG.identifier,
                   P2_CONFIG.num_assigned_cores,
                   P2_CONFIG.name,
                   P2_CONFIG.region_name_code_mem,
                   (void*)_p2_code_start,
                   (size_t)p1_code_size,
                   P2_CONFIG.access_code_mem,
                   P2_CONFIG.region_name_data_mem,
                   (void*)_p2_data_start,
                   p2_data_size,
                   P2_CONFIG.access_data_mem,
                //    test_spatial_violation_p2,
                   #if defined(TEST_PERF13)
                   main_process,
                   #elif defined(TEST_PERF19)
                   p2_main,
                   #endif
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

    #endif

    return 1;
}

/* __________________________________________________________________________
* END OF FILE:
* -------------
* ___________________________________________________________________________
*/
