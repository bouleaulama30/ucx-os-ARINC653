#ifndef P1_BENCHMARK_PERF5_H
#define P1_BENCHMARK_PERF5_H

#include "performance_lib.h"

#ifndef NB_ITER
#define NB_ITER 50
#endif

perf_task_retval_t mutex_processing_initialize_tests();

perf_task_retval_t TA_high_prio();
perf_task_retval_t TB_med_prio();
perf_task_retval_t TC_low_prio();

perf_task_handle_t tasks_handle[3];

perf_mutex_t mutex;
perf_sem_t aux_sem;
perf_sem_t aux_sem2;

DECLARE_TIME_COUNTERS(perf_time_t, time);
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
void MAIN_FUNCTION()
{
  RETURN_CODE_TYPE errCode;
  perf_initialize_test(mutex_processing_initialize_tests);
  SET_PARTITION_MODE (NORMAL, &errCode);
  if (errCode != NO_ERROR){
    PERF_PRINT_STRING("\nError changing partition mode");
    PERF_PRINT_NUMBER(errCode);
    PERF_PRINT_EOL();
  }
  while(1);
}

__attribute__((section(".p1_code")))
perf_task_retval_t mutex_processing_initialize_tests()
{
  /* Create resources */
  perf_mutex_create(&mutex);
  perf_sem_create(&aux_sem, 0);
  perf_sem_create(&aux_sem2, 0);

  /* Create task */
  tasks_handle[0] = perf_create_task(TC_low_prio,
      "L",
      BASE_PRIO - 2 /* TC_low_prio is the low priority task. */
    );

  tasks_handle[1] = perf_create_task(TB_med_prio,
      "M",
      BASE_PRIO - 1 /* TB_med_prio is the med priority task. */
    );

  tasks_handle[2] = perf_create_task(TA_high_prio,
      "H",
      BASE_PRIO /* TA_high_prio is the high priority task. */
    );

  return TASK_DEFAULT_RETURN;
}

__attribute__((section(".p1_code")))
perf_task_retval_t TC_low_prio()
{
  int32_t i;
  DECLARE_TIME_STATS(uint64_t);

  for (i = 0; i < NB_ITER; i++)
  {
    perf_mutex_acquire(&mutex);
    perf_sem_signal(&aux_sem);
    WRITE_T2_COUNTER(time);
    COMPUTE_TIME_STATS(time, i);
    perf_cycle_reset_counter();

    perf_mutex_release(&mutex);
  }

  REPORT_BENCHMARK_RESULTS("--- Mutex PIP---");

  perf_task_suspend_self();

  while(1);
}

__attribute__((section(".p1_code")))
perf_task_retval_t TB_med_prio()
{
  int32_t i;

  for (i = 0; i < NB_ITER; i++)
  {
    perf_sem_wait(&aux_sem2);
  }

  perf_task_suspend_self();

  while(1);
}

__attribute__((section(".p1_code")))
perf_task_retval_t TA_high_prio()
{
  int32_t i;

  for (i = 0; i < NB_ITER; i++)
  {
    perf_sem_wait(&aux_sem);
    perf_sem_signal(&aux_sem2);
    WRITE_T1_COUNTER(time);
    perf_mutex_acquire(&mutex);
    perf_mutex_release(&mutex);
  }

  perf_task_suspend_self();

  while(1);
}

#endif /* P1_BENCHMARK_PERF5_H */
