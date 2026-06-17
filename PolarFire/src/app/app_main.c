
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
#include "performance_lib.h"
#include "P1.h"


#ifndef NB_ITER
#define NB_ITER 100000
#endif

#ifndef NB_TASK
#define NB_TASK 5
#endif

perf_task_retval_t task();
perf_task_retval_t round_robin_stress_initialize_test();

perf_task_handle_t tasks_handle[NB_TASK];
char_t tasks_name[NB_TASK][5];


volatile int32_t tasks_idx;

/* __________________________________________________________________________
*
* FUNCTION NAME : main
* DESCRIPTION   : Partition 1 function.
* PARAMETERS : None.
*   INPUT :
*   OUTPUT :
* RETURN : None.
* __________________________________________________________________________
*/

__attribute__((section(".p1_code")))
void MAIN_FUNCTION(void)
{
  RETURN_CODE_TYPE errCode;
  perf_initialize_test(round_robin_stress_initialize_test);
  SET_PARTITION_MODE (NORMAL, &errCode);
  if (errCode != NO_ERROR){
    PERF_PRINT_STRING("\nError changing partition mode");
    PERF_PRINT_NUMBER(errCode);
    PERF_PRINT_EOL();
  }
  while(1);
}

__attribute__((section(".p1_code")))
perf_task_retval_t round_robin_stress_initialize_test()
{

  int32_t i;

  tasks_idx = 0;

  for (i = 0 ; i < NB_TASK; i++)
  {
    tasks_name[i][0] = 65;
    tasks_name[i][1] = (65 + i) % 255;
    tasks_name[i][2] = (66 + i) % 255;
    tasks_name[i][3] = (67 + i) % 255;
    tasks_name[i][4] = '\0';
    tasks_handle[i] = perf_create_task(task, tasks_name[i], BASE_PRIO);
  }
  PERF_PRINT_STRING("Process Context Switch. No of processes: ");
  PERF_PRINT_NUMBER(NB_TASK);
  PERF_PRINT_EOL();

}

__attribute__((section(".p1_code")))
perf_task_retval_t task()
{
  int32_t i;
  DECLARE_TIME_MEASURE();
  INITIALIZE_TIME_VARS("CNTX SWITCH");
  int32_t id =  tasks_idx++;

  for (i = 0; i < NB_ITER; i++)
  {
    INIT_TIME_MEASURE();
    perf_task_yield();
    FINISH_TIME_MEASURE();
    VALIDATE_TIME_MEASURE(1);
  }

  if (id == 0){
    PRINT_PERFORMANCE_INFO();
  }
  else{
    PERF_PRINT_STRING("Process ");
    PERF_PRINT_NUMBER(id);
    PERF_PRINT_STRING(" done.");
    PERF_PRINT_EOL();
  }

  perf_task_suspend_self();

  while(1);

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

    return 1;
}

/* __________________________________________________________________________
* END OF FILE:
* -------------
* ___________________________________________________________________________
*/
