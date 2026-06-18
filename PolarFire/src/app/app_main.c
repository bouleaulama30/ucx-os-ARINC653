
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

#elif defined(TEST_PERF2)


#include "performance_lib.h"

#ifndef NB_ITER
#define NB_ITER 3000
#endif

perf_task_retval_t mutex_initialize_test();
perf_task_retval_t sender();
perf_task_retval_t receiver();

perf_task_handle_t tasks_handle[2];
perf_sem_t aux_sem;
perf_mutex_t mutex;

DECLARE_TIME_COUNTERS(perf_time_t, s_to_r)
DECLARE_TIME_COUNTERS(perf_time_t, r_to_s)

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

  perf_initialize_test(mutex_initialize_test);
  SET_PARTITION_MODE (NORMAL, &errCode);
  if (errCode != NO_ERROR){
    PERF_PRINT_STRING("\nError changing partition mode");
    PERF_PRINT_NUMBER(errCode);
    PERF_PRINT_EOL();
  }
  while(1);
}

__attribute__((section(".p1_code")))
perf_task_retval_t mutex_initialize_test()
{
  perf_sem_create(&aux_sem, 0);
  perf_mutex_create(&mutex);

  tasks_handle[0] = perf_create_task(sender,
      "S",
      BASE_PRIO - 1 /* sender is the low priority task. */
    );

  tasks_handle[1] = perf_create_task(receiver,
      "R",
      BASE_PRIO /* receiver is the high priority task. */
    );

  return TASK_DEFAULT_RETURN;
}

__attribute__((section(".p1_code")))
perf_task_retval_t sender()
{
  int32_t i;

  /* 1b - Benchmark. */
  for (i = 0; i < NB_ITER * 2; i++)
  {
    perf_mutex_acquire(&mutex);
    perf_sem_signal(&aux_sem);
    WRITE_T2_COUNTER(r_to_s)
    WRITE_T1_COUNTER(s_to_r)
    perf_mutex_release(&mutex);
  }

  perf_task_suspend_self();

  while(1);
}

__attribute__((section(".p1_code")))
perf_task_retval_t receiver()
{
  int32_t i;
  DECLARE_TIME_STATS(uint64_t)

  /* 1a - Benchmark. */
  for (i = 0; i < NB_ITER; i++)
  {
    perf_sem_wait(&aux_sem);
    perf_mutex_acquire(&mutex);
    WRITE_T2_COUNTER(s_to_r)
    perf_mutex_release(&mutex);
    COMPUTE_TIME_STATS(s_to_r, i);
    perf_cycle_reset_counter();
  }

  REPORT_BENCHMARK_RESULTS("-- Mutex release unblock --")
  RESET_TIME_STATS()

  for (i = 0; i < NB_ITER; i++)
  {
    perf_sem_wait(&aux_sem);
    WRITE_T1_COUNTER(r_to_s)
    perf_mutex_acquire(&mutex);
    perf_mutex_release(&mutex);
    COMPUTE_TIME_STATS(r_to_s, i)
    perf_cycle_reset_counter();

  }

  REPORT_BENCHMARK_RESULTS("-- Mutex request block --")

  perf_task_suspend_self();

  while(1);
}

#elif defined(TEST_PERF3)


#include "performance_lib.h"

#ifndef NB_ITER
#define NB_ITER 300
#endif

perf_task_retval_t mutex_processing_initialize_tests();
perf_task_retval_t task();

perf_task_handle_t task_handle;
perf_mutex_t mutex;

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

  /* Create task */
  task_handle = perf_create_task(task,
      "S",
      BASE_PRIO - 1 /* `task` is the only task */
    );

}

__attribute__((section(".p1_code")))
perf_task_retval_t task()
{
  int32_t i;
  DECLARE_TIME_COUNTERS(perf_time_t, acqui);
  DECLARE_TIME_COUNTERS(perf_time_t, recv);
  DECLARE_TIME_STATS(uint64_t);

  /* 1 - Measure mutex acquisition time */
  for (i = 0; i < NB_ITER; i++)
  {
    WRITE_T1_COUNTER(acqui);
    perf_mutex_acquire(&mutex);
    WRITE_T2_COUNTER(acqui);
    perf_mutex_release(&mutex);
    COMPUTE_TIME_STATS(acqui, i);

    perf_cycle_reset_counter();

  }

  REPORT_BENCHMARK_RESULTS("--- mutex acquisition ---");
  RESET_TIME_STATS();

  /* 2 - Measure mutex release time */
  for (i = 0; i < NB_ITER; i++)
  {
    perf_mutex_acquire(&mutex);
    WRITE_T1_COUNTER(recv);
    perf_mutex_release(&mutex);
    WRITE_T2_COUNTER(recv);
    COMPUTE_TIME_STATS(recv, i);

    perf_cycle_reset_counter();

  }

  REPORT_BENCHMARK_RESULTS("--- mutex release ---");

  perf_task_suspend_self();

  while(1);
}

#elif defined(TEST_PERF4)


#include "performance_lib.h"

#ifndef NB_ITER
#define NB_ITER 5000
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

#elif defined(TEST_PERF5)

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

#elif defined(TEST_PERF6)

#include "performance_lib.h"

#ifndef NB_ITER
#define NB_ITER 100
#endif

#define NB_TASK 2

perf_task_retval_t monitor();
perf_task_retval_t task();
perf_task_retval_t jitter_initialize_test();

perf_task_handle_t tasks_handle[NB_TASK + 1];
char tasks_name[NB_TASK][5];

DECLARE_TIME_COUNTERS(perf_time_t, _)


/* __________________________________________________________________________
*
* FUNCTION NAME : main_process
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

  perf_initialize_test(jitter_initialize_test);
  SET_PARTITION_MODE (NORMAL, &errCode);
  if (errCode != NO_ERROR){
    PERF_PRINT_STRING("\nError changing partition mode");
    PERF_PRINT_NUMBER(errCode);
    PERF_PRINT_EOL();
  }
  while(1);
}

__attribute__((section(".p1_code")))
perf_task_retval_t jitter_initialize_test()
{
  int32_t i;

  for (i = 0 ; i < NB_TASK; i++)
  {
    tasks_name[i][0] = 65;
    tasks_name[i][1] = (65 + i) % 255;
    tasks_name[i][2] = (66 + i) % 255;
    tasks_name[i][3] = (67 + i) % 255;
    tasks_name[i][4] = '\0';
    tasks_handle[i] = perf_create_task(task, tasks_name[i], BASE_PRIO - 1);
  }

  tasks_handle[NB_TASK] = perf_create_task(monitor, "MON", BASE_PRIO);
}

__attribute__((section(".p1_code")))
perf_task_retval_t task()
{
  int32_t i;

  for (i = 0; i < NB_ITER; i++)
  {
    perf_task_yield();
  }

  perf_task_suspend_self();

  while(1);
}

__attribute__((section(".p1_code")))
perf_task_retval_t monitor()
{
  int32_t i;
  long diff;

  perf_time_t time_t1;
  perf_time_t time_t2;
  DECLARE_TIME_STATS(uint64_t);

  for (i = 0; i < NB_ITER; i++)
  {
    time_t1 = GET_CURRENT_TICKS();
    time_t1 = perf_add_times(&time_t1, 500000000);

    perf_task_delay(400);

    time_t2 = GET_CURRENT_TICKS();
    diff = perf_time_diff(&time_t1, &time_t2);
    PERF_PRINT_NUMBER(diff);
    PERF_PRINT_EOL();

    perf_task_yield();
  }

  REPORT_BENCHMARK_RESULTS("--Monitor--");
  (void) cycles;
  perf_task_suspend_self();

  while(1);
}

#elif defined(TEST_PERF7)

#include "performance_lib.h"

#ifndef NB_ITER
#define NB_ITER 1000
#endif

perf_task_retval_t sem_initialize_test();
perf_task_retval_t sender();
perf_task_retval_t receiver();

perf_task_handle_t tasks_handle[2];
perf_sem_t sem;

DECLARE_TIME_COUNTERS(perf_time_t, s_to_r)
DECLARE_TIME_COUNTERS(perf_time_t, r_to_s)

/* __________________________________________________________________________
*
* FUNCTION NAME : main_process
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

  perf_initialize_test(sem_initialize_test);
  SET_PARTITION_MODE (NORMAL, &errCode);
  if (errCode != NO_ERROR){
    PERF_PRINT_STRING("\nError changing partition mode");
    PERF_PRINT_NUMBER(errCode);
    PERF_PRINT_EOL();
  }
  while(1);
}

__attribute__((section(".p1_code")))
perf_task_retval_t sem_initialize_test()
{
  perf_sem_create(&sem, 0);

  tasks_handle[0] = perf_create_task(sender,
      "S",
      BASE_PRIO - 1 /* sender is the low priority task. */
    );

  tasks_handle[1] = perf_create_task(receiver,
      "R",
      BASE_PRIO /* receiver is the high priority task. */
    );

  return TASK_DEFAULT_RETURN;
}

__attribute__((section(".p1_code")))
perf_task_retval_t sender()
{
  int32_t i;

  /* 2b - Benchmark. */
  for (i = 0; i < NB_ITER + 1; i++)
  {
    perf_sem_signal(&sem);
    WRITE_T2_COUNTER(r_to_s)
  }

  for (i = 0; i < NB_ITER; i++)
  {
    WRITE_T1_COUNTER(s_to_r)
    perf_sem_signal(&sem);
  }

  perf_task_suspend_self();

  while(1);
}

__attribute__((section(".p1_code")))
perf_task_retval_t receiver()
{
  int32_t i;
  DECLARE_TIME_STATS(uint64_t)

  /* 1 - Let sender start */
  perf_sem_wait(&sem);

  /* 2a - Benchmark. */
  for (i = 0; i < NB_ITER; i++)
  {
    WRITE_T1_COUNTER(r_to_s)
    perf_sem_wait(&sem);
    COMPUTE_TIME_STATS(r_to_s, i);
    perf_cycle_reset_counter();

  }

  REPORT_BENCHMARK_RESULTS("-- Sem: Wait block --")
  RESET_TIME_STATS()

  for (i = 0; i < NB_ITER; i++)
  {
    perf_sem_wait(&sem);
    WRITE_T2_COUNTER(s_to_r)
    COMPUTE_TIME_STATS(s_to_r, i)
    perf_cycle_reset_counter();

  }

  REPORT_BENCHMARK_RESULTS("-- Sem: Signal unblock --")

  perf_task_suspend_self();

  while(1);
}

#elif defined(TEST_PERF8)


#include "performance_lib.h"

#ifndef NB_ITER
#define NB_ITER 1000
#endif

#ifndef NB_TASK
#define NB_TASK 1
#endif

perf_task_retval_t sem_prio_initialize_test();
perf_task_retval_t TA_high_prio();
perf_task_retval_t TB_med_prio();
perf_task_retval_t TC_low_prio();

perf_task_handle_t tasks_handle[2];

perf_task_handle_t med_tasks_handle[NB_TASK];
char workload_tasks_name[NB_TASK][4];

perf_sem_t aux_sem;
perf_sem_t sem;

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

  perf_initialize_test(sem_prio_initialize_test);
  SET_PARTITION_MODE (NORMAL, &errCode);
  if (errCode != NO_ERROR){
    PERF_PRINT_STRING("\nError changing partition mode");
    PERF_PRINT_NUMBER(errCode);
    PERF_PRINT_EOL();
  }
  while(1);
}

__attribute__((section(".p1_code")))
perf_task_retval_t sem_prio_initialize_test()
{
  int32_t i;

  perf_sem_create(&sem, 0);
  perf_sem_create(&aux_sem, 0);

  tasks_handle[0] = perf_create_task(TC_low_prio,
      "L",
      BASE_PRIO - 2 /* TC_low_prio is the low priority task. */
    );

  for (i = 0; i < NB_TASK; i++)
  {
    workload_tasks_name[i][0] = 65;
    workload_tasks_name[i][1] = (65 + i) % 255;
    workload_tasks_name[i][2] = 0;
    workload_tasks_name[i][3] = 0;

    med_tasks_handle[i] = perf_create_task(TB_med_prio,
        workload_tasks_name[i],
        BASE_PRIO - 1
      );
  }

  tasks_handle[1] = perf_create_task(TA_high_prio,
      "H",
      BASE_PRIO /* TA_high_prio is the high priority task. */
    );

  return TASK_DEFAULT_RETURN;
}

__attribute__((section(".p1_code")))
perf_task_retval_t TC_low_prio()
{
  int i;

  for (i = 0; i < NB_ITER; i++)
  {
    perf_sem_signal(&aux_sem);
  }

  perf_task_suspend_self();

  while(1);
}

__attribute__((section(".p1_code")))
perf_task_retval_t TB_med_prio()
{
  int i;

  for (i = 0; i < NB_ITER; i++)
  {
    perf_sem_wait(&sem);
  }

  perf_task_suspend_self();

  while(1);
}

__attribute__((section(".p1_code")))
perf_task_retval_t TA_high_prio()
{
  int i;
  DECLARE_TIME_COUNTERS(perf_time_t, time);
  DECLARE_TIME_STATS(uint64_t);

  for (i = 0; i < NB_ITER; i++)
  {
    perf_sem_wait(&aux_sem);

    WRITE_T1_COUNTER(time);
    perf_sem_signal(&sem);
    WRITE_T2_COUNTER(time);

    COMPUTE_TIME_STATS(time, i);
    perf_cycle_reset_counter();
  }

  REPORT_BENCHMARK_RESULTS("--- sem singaling with prio ---");

  perf_task_suspend_self();

  while(1);
}

#elif defined(TEST_PERF9)

#include "performance_lib.h"


#ifndef NB_ITER
#define NB_ITER 1000
#endif

perf_task_retval_t sem_processing_initialize_test();
perf_task_retval_t task();

perf_task_handle_t task_handle;
perf_sem_t sem;

/* __________________________________________________________________________
*
* FUNCTION NAME : main_process
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
  perf_initialize_test(sem_processing_initialize_test);
  SET_PARTITION_MODE (NORMAL, &errCode);
  if (errCode != NO_ERROR){
    PERF_PRINT_STRING("\nError changing partition mode");
    PERF_PRINT_NUMBER(errCode);
    PERF_PRINT_EOL();
  }
  while(1);
}

__attribute__((section(".p1_code")))
perf_task_retval_t sem_processing_initialize_test()
{
  perf_sem_create(&sem, 0);

  task_handle = perf_create_task(task,
      "S",
      BASE_PRIO - 1 /* `task` is the only task */
    );

  return TASK_DEFAULT_RETURN;
}

__attribute__((section(".p1_code")))
perf_task_retval_t task()
{
  int32_t i;
  DECLARE_TIME_COUNTERS(perf_time_t, inc);
  DECLARE_TIME_COUNTERS(perf_time_t, dec);
  DECLARE_TIME_STATS(uint64_t);
  uint64_t _workload_results[NB_ITER];

  /* 1b - Measure semaphore signaling time */
  for (i = 0; i < NB_ITER; i++)
  {
    WRITE_T1_COUNTER(inc);
    perf_sem_signal(&sem);
    WRITE_T2_COUNTER(inc);
    perf_sem_wait(&sem);
    COMPUTE_TIME_STATS(inc, i);
    DO_WORKLOAD(i)
    perf_cycle_reset_counter();
  }

  REPORT_BENCHMARK_RESULTS("--- Sem: Signal ---");

  RESET_TIME_STATS();

  /* 2b - Measure semaphore acquisition time */
  for (i = 0; i < NB_ITER; i++)
  {
    perf_sem_signal(&sem);
    WRITE_T1_COUNTER(dec);
    perf_sem_wait(&sem);
    WRITE_T2_COUNTER(dec);
    COMPUTE_TIME_STATS(dec, i);
    DO_WORKLOAD(i)
    perf_cycle_reset_counter();
  }

  REPORT_BENCHMARK_RESULTS("--- Sem: Wait ---");
  (void)_workload_results;
  perf_task_suspend_self();

  while(1);
}

#elif defined(TEST_PERF10)


#include "performance_lib.h"


#ifndef NB_ITER
#define NB_ITER 50
#endif

#ifndef NB_WORKLOAD_TASK
#define NB_WORKLOAD_TASK 2
#endif

#ifndef DELAY_MS
#define DELAY_MS 10
#endif

perf_task_retval_t sem_initialize_test();
perf_task_retval_t sender();
perf_task_retval_t receiver();
perf_task_retval_t workload_task();

perf_task_handle_t tasks_handle[2];
perf_task_handle_t workload_tasks_handle[NB_WORKLOAD_TASK];
char workload_tasks_name[NB_WORKLOAD_TASK][4];

perf_sem_t sem;

DECLARE_TIME_COUNTERS(perf_time_t, _)

/* __________________________________________________________________________
*
* FUNCTION NAME : main_process
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

  perf_initialize_test(sem_initialize_test);
  SET_PARTITION_MODE (NORMAL, &errCode);
  if (errCode != NO_ERROR){
    PERF_PRINT_STRING("\nError changing partition mode");
    PERF_PRINT_NUMBER(errCode);
    PERF_PRINT_EOL();
  }
  while(1);
}

__attribute__((section(".p1_code")))
perf_task_retval_t sem_initialize_test()
{
  int32_t i;

  perf_sem_create(&sem, 0);

  tasks_handle[0] = perf_create_task(sender,
      "S",
      BASE_PRIO - 1 /* sender is the low priority task. */
    );

  tasks_handle[1] = perf_create_task(receiver,
      "R",
      BASE_PRIO /* receiver is the high priority task. */
    );

  for (i = 0 ; i < NB_WORKLOAD_TASK; i++)
  {
    workload_tasks_name[i][0] = 65;
    workload_tasks_name[i][1] = (65 + i) % 255;
    workload_tasks_name[i][2] = 0;
    workload_tasks_name[i][3] = 0;
    /* Workoad tasks are the lowest priority */
    workload_tasks_handle[i] = perf_create_task(workload_task, workload_tasks_name[i], BASE_PRIO - 2);
  }

  return TASK_DEFAULT_RETURN;
}

__attribute__((section(".p1_code")))
perf_task_retval_t sender()
{
  int32_t i;

  /* 2b - Benchmark. */
  for (i = 0; i < NB_ITER + 1; i++)
  {
    WRITE_T1_COUNTER(_)
    perf_sem_signal(&sem);
    perf_task_delay(DELAY_MS);
  }

  perf_task_suspend_self();

  while(1);
}

__attribute__((section(".p1_code")))
perf_task_retval_t receiver()
{
  int32_t i;
  DECLARE_TIME_STATS(uint64_t)

  /* 2a - Benchmark. */
  for (i = 0; i < NB_ITER; i++)
  {
    perf_sem_wait(&sem);
    WRITE_T2_COUNTER(_)
    COMPUTE_TIME_STATS(_, i);
    perf_cycle_reset_counter();

  }

  REPORT_BENCHMARK_RESULTS("-- Sem workload --")
  perf_task_suspend_self();

  while(1);
}

__attribute__((section(".p1_code")))
perf_task_retval_t workload_task()
{
  int32_t i;
  uint64_t _workload_results[100];

  while (1)
  {
    DO_WORKLOAD(i % 100)
    perf_task_yield();
    i++;
  }

  perf_task_suspend_self();
  (void)_workload_results;
  while(1);
}

#elif defined(TEST_PERF11)


#include "performance_lib.h"

SEMAPHORE_ID_TYPE sem_id1;
SEMAPHORE_ID_TYPE sem_id2;
SEMAPHORE_ID_TYPE sem_id3;
SEMAPHORE_ID_TYPE sem_id4;
SEMAPHORE_NAME_TYPE Semaphore1Name = "Semaphore1";
SEMAPHORE_NAME_TYPE Semaphore2Name = "Semaphore2";
SEMAPHORE_NAME_TYPE Semaphore3Name = "Semaphore3";
SEMAPHORE_NAME_TYPE Semaphore4Name = "Semaphore4";
SEMAPHORE_VALUE_TYPE CurrentValue = 0;
SEMAPHORE_VALUE_TYPE MaxValue = 1;
QUEUING_DISCIPLINE_TYPE     sem_queue;


MUTEX_ID_TYPE mut_id1;
MUTEX_ID_TYPE mut_id2;
MUTEX_ID_TYPE mut_id3;
MUTEX_ID_TYPE mut_id4;
MUTEX_NAME_TYPE Mutex1Name = "Mutex1";
MUTEX_NAME_TYPE Mutex2Name = "Mutex2";
MUTEX_NAME_TYPE Mutex3Name = "Mutex3";
MUTEX_NAME_TYPE Mutex4Name = "Mutex4";
PRIORITY_TYPE priorityValue = MAX_PRIORITY_VALUE;
QUEUING_DISCIPLINE_TYPE     mut_queue;

void test_create_sem(T_uint8 calc_dev);
void test_signal_sem(T_uint8 calc_dev);
void test_wait_on_sem(T_uint8 calc_dev);
void test_sem_id(T_uint8 calc_dev);
void test_sem_status(T_uint8 calc_dev); 


void test_create_mut(T_uint8 calc_dev);
void test_acquire_mut(T_uint8 id, T_uint8 calc_dev);
void test_release_mut(T_uint8 id, T_uint8 calc_dev);
void test_id_mut(T_uint8 calc_dev);
void test_status_mut(T_uint8 calc_dev); 
void test_reset_mut(T_uint8 calc_dev); 

void printResults(void);

/* __________________________________________________________________________
*
* FUNCTION NAME : main_process
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
  PROCESS_ID_TYPE        process_id;
  PROCESS_ATTRIBUTE_TYPE process_att;
  RETURN_CODE_TYPE       ret_type;

  PARTITION_STATUS_TYPE partitionStatus;
  GET_PARTITION_STATUS(&partitionStatus, &ret_type);

  PERF_PRINT_STRING("Init P1 partition\n\r");

  /* Set processes  */
  process_att.ENTRY_POINT   = printResults;
  process_att.DEADLINE      = SOFT;
  process_att.PERIOD        = partitionStatus.PERIOD;
  process_att.STACK_SIZE    = 16384;
  process_att.TIME_CAPACITY = partitionStatus.PERIOD;
  process_att.BASE_PRIORITY = MIN_PRIORITY_VALUE;
  strncpy(process_att.NAME, "PRINT", MAX_NAME_LENGTH);
 
  CREATE_PROCESS(&process_att, &process_id, &ret_type);

  if(ret_type != NO_ERROR)
  {
      PERF_PRINT_STRING("Cannot create PRINT process  "); 
      PERF_PRINT_NUMBER(ret_type);
      PERF_PRINT_EOL();
  }

  PERF_PRINT_STRING("Running APEX analysis\n\r");
  test_create_sem(1);
  test_create_mut(1);

  START(process_id, &ret_type);
  if(ret_type != NO_ERROR)
  {
      PERF_PRINT_STRING("Cannot start PRINT process  "); 
      PERF_PRINT_NUMBER(ret_type);
      PERF_PRINT_EOL();
  }

  PERF_PRINT_STRING("P1 partition switching to normal mode\n\r");
  SET_PARTITION_MODE(NORMAL, &ret_type);

  if(ret_type != NO_ERROR)
  {
      PERF_PRINT_STRING("Cannot switch P1 partition to NORMAL state "); 
      PERF_PRINT_NUMBER(ret_type);
      PERF_PRINT_EOL();
  }

  while(1);
}

__attribute__((section(".p1_code")))
void printResults(void)
{
  RETURN_CODE_TYPE retCode = NO_ERROR;

  while(TRUE){
    test_signal_sem(1);
    test_wait_on_sem(1);
    test_sem_id(1);
    test_sem_status(1); 
    test_acquire_mut(0,1);
    test_release_mut(0,1);
    test_acquire_mut(1,1);
    test_release_mut(1,1);
    test_acquire_mut(2,1);
    test_release_mut(2,1);
    test_acquire_mut(3,1);
    test_release_mut(3,1);
    test_id_mut(1);
    test_status_mut(1); 
    test_reset_mut(1); 

    PRINT_PERFORMANCE_INFO();
    PERIODIC_WAIT(&retCode);
  }
}

/*SEMPAHORES*/
__attribute__((section(".p1_code")))
 void test_create_sem(T_uint8 calc_dev)
  {
  RETURN_CODE_TYPE   err_code;

  DECLARE_TIME_MEASURE()
  INITIALIZE_TIME_VARS("SEM_CREATE");

  sem_queue = FIFO;

  INIT_TIME_MEASURE();
  CREATE_SEMAPHORE (Semaphore1Name, CurrentValue, MaxValue, sem_queue, &sem_id1, &err_code);
  FINISH_TIME_MEASURE();
  if(err_code == NO_ERROR){
    VALIDATE_TIME_MEASURE(calc_dev)
  }
  else{
    PERF_PRINT_STRING("CREATE SEM: ");
    PERF_PRINT_NUMBER(err_code);
    PERF_PRINT_EOL();
  } 

  INIT_TIME_MEASURE();
  CREATE_SEMAPHORE (Semaphore2Name, CurrentValue, MaxValue, sem_queue, &sem_id2, &err_code);
  FINISH_TIME_MEASURE();
  if(err_code == NO_ERROR){
    VALIDATE_TIME_MEASURE(calc_dev)
  }
  else{
    PERF_PRINT_STRING("CREATE SEM: ");
    PERF_PRINT_NUMBER(err_code);
    PERF_PRINT_EOL();
  } 

  INIT_TIME_MEASURE();
  CREATE_SEMAPHORE (Semaphore3Name, CurrentValue, MaxValue, sem_queue, &sem_id3, &err_code);
  FINISH_TIME_MEASURE();
  if(err_code == NO_ERROR){
    VALIDATE_TIME_MEASURE(calc_dev)
  }
  else{
    PERF_PRINT_STRING("CREATE SEM: ");
    PERF_PRINT_NUMBER(err_code);
    PERF_PRINT_EOL();
  }

  INIT_TIME_MEASURE();
  CREATE_SEMAPHORE (Semaphore4Name, CurrentValue, MaxValue, sem_queue, &sem_id4, &err_code);
  FINISH_TIME_MEASURE();
  if(err_code == NO_ERROR){
    VALIDATE_TIME_MEASURE(calc_dev)
  }
  else{
    PERF_PRINT_STRING("CREATE SEM: ");
    PERF_PRINT_NUMBER(err_code);
    PERF_PRINT_EOL();
  } 
}

__attribute__((section(".p1_code")))
void test_signal_sem(T_uint8 calc_dev)
{
    DECLARE_TIME_MEASURE()
    INITIALIZE_TIME_VARS("SEMAPHORE_SIGNAL")

    RETURN_CODE_TYPE errCode = TIMED_OUT;
    SEMAPHORE_ID_TYPE SemaphoreId;

    GET_SEMAPHORE_ID(Semaphore1Name, &SemaphoreId, &errCode);
    INIT_TIME_MEASURE();
    SIGNAL_SEMAPHORE(SemaphoreId, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
        }
    else{
        PERF_PRINT_STRING("ERROR: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
    }
    
    GET_SEMAPHORE_ID(Semaphore2Name, &SemaphoreId, &errCode);
    INIT_TIME_MEASURE();
    SIGNAL_SEMAPHORE(SemaphoreId, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
        }
    else{
        PERF_PRINT_STRING("ERROR: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
}
    

    GET_SEMAPHORE_ID(Semaphore3Name, &SemaphoreId, &errCode);
    INIT_TIME_MEASURE();
    SIGNAL_SEMAPHORE(SemaphoreId, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
        }
    else{
        PERF_PRINT_STRING("ERROR: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
}
    
    GET_SEMAPHORE_ID(Semaphore4Name, &SemaphoreId, &errCode);
    INIT_TIME_MEASURE();
    SIGNAL_SEMAPHORE(SemaphoreId, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
        }
    else{
        PERF_PRINT_STRING("ERROR: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
}

}

__attribute__((section(".p1_code")))
void test_wait_on_sem(T_uint8 calc_dev)
{
    RETURN_CODE_TYPE errCode = TIMED_OUT;
    SEMAPHORE_ID_TYPE SemaphoreId;

    DECLARE_TIME_MEASURE()
    INITIALIZE_TIME_VARS("SEMAPHORE_WAIT")

    GET_SEMAPHORE_ID(Semaphore1Name, &SemaphoreId, &errCode);
    INIT_TIME_MEASURE();
    WAIT_SEMAPHORE(SemaphoreId,INFINITE_TIME_VALUE, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
        }
    else{
        PERF_PRINT_STRING("Sem WAIT: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
    }
    
    GET_SEMAPHORE_ID(Semaphore2Name, &SemaphoreId, &errCode);
    INIT_TIME_MEASURE();
    WAIT_SEMAPHORE(SemaphoreId,INFINITE_TIME_VALUE, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
        }
    else{
        PERF_PRINT_STRING("Sem WAIT: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
    }
    

    GET_SEMAPHORE_ID(Semaphore3Name, &SemaphoreId, &errCode);
    INIT_TIME_MEASURE();
    WAIT_SEMAPHORE(SemaphoreId,INFINITE_TIME_VALUE, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
        }
    else{
        PERF_PRINT_STRING("Sem WAIT: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
    }
    

    GET_SEMAPHORE_ID(Semaphore4Name, &SemaphoreId, &errCode);
    INIT_TIME_MEASURE();
    WAIT_SEMAPHORE(SemaphoreId,INFINITE_TIME_VALUE, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
        }
    else{
        PERF_PRINT_STRING("Sem WAIT: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
    }
}

__attribute__((section(".p1_code")))
void test_sem_id(T_uint8 calc_dev)
{
    RETURN_CODE_TYPE errCode = TIMED_OUT;
    SEMAPHORE_ID_TYPE SemaphoreId;

    DECLARE_TIME_MEASURE()
    INITIALIZE_TIME_VARS("SEMAPHORE_ID")

    INIT_TIME_MEASURE();
    GET_SEMAPHORE_ID(Semaphore1Name, &SemaphoreId, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
        }
    else{
        PERF_PRINT_STRING("SEM ID: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
    }
    
    INIT_TIME_MEASURE();
    GET_SEMAPHORE_ID(Semaphore2Name, &SemaphoreId, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
    }
    else{
        PERF_PRINT_STRING("SEM ID: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
    }
    
    INIT_TIME_MEASURE();
    GET_SEMAPHORE_ID(Semaphore3Name, &SemaphoreId, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
    }
    else{
        PERF_PRINT_STRING("SEM ID: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
    }
    
    INIT_TIME_MEASURE();
    GET_SEMAPHORE_ID(Semaphore4Name, &SemaphoreId, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
    }
    else{
        PERF_PRINT_STRING("SEM ID: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
    }
    
}

__attribute__((section(".p1_code")))
void test_sem_status(T_uint8 calc_dev)
{
    RETURN_CODE_TYPE errCode = TIMED_OUT;
    SEMAPHORE_ID_TYPE SemaphoreId;
    SEMAPHORE_STATUS_TYPE   status;

    DECLARE_TIME_MEASURE()
    INITIALIZE_TIME_VARS("SEM_STATUS")

    GET_SEMAPHORE_ID(Semaphore1Name, &SemaphoreId, &errCode);
    INIT_TIME_MEASURE();
    GET_SEMAPHORE_STATUS (SemaphoreId, &status, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
        }
    else{
        PERF_PRINT_STRING("SEM STATUS: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
    }
    
    GET_SEMAPHORE_ID(Semaphore2Name, &SemaphoreId, &errCode);
    INIT_TIME_MEASURE();
    GET_SEMAPHORE_STATUS (SemaphoreId, &status, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
        }
    else{
         PERF_PRINT_STRING("SEM STATUS: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
    }
    
    GET_SEMAPHORE_ID(Semaphore3Name, &SemaphoreId, &errCode);
    INIT_TIME_MEASURE();
    GET_SEMAPHORE_STATUS (SemaphoreId, &status, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
        }
    else{
        PERF_PRINT_STRING("SEM STATUS: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
    }
    GET_SEMAPHORE_ID(Semaphore4Name, &SemaphoreId, &errCode);
    INIT_TIME_MEASURE();
    GET_SEMAPHORE_STATUS (SemaphoreId, &status, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
        }
    else{
        PERF_PRINT_STRING("SEM STATUS: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
    }
}

/*MUTEX*/
__attribute__((section(".p1_code")))
void test_create_mut(T_uint8 calc_dev)
  {
  RETURN_CODE_TYPE   err_code;

  DECLARE_TIME_MEASURE()
  INITIALIZE_TIME_VARS("MUTEX_CREATE");

  mut_queue = FIFO;

  INIT_TIME_MEASURE();
  CREATE_MUTEX (Mutex1Name, priorityValue, mut_queue, &mut_id1, &err_code);
  FINISH_TIME_MEASURE();
  if(err_code == NO_ERROR){
    VALIDATE_TIME_MEASURE(calc_dev)
  }
  else{
    PERF_PRINT_STRING("CREATE MUTEX: ");
    PERF_PRINT_NUMBER(err_code);
    PERF_PRINT_EOL();
  } 

  INIT_TIME_MEASURE();
  CREATE_MUTEX (Mutex2Name, priorityValue, mut_queue, &mut_id2, &err_code);
  FINISH_TIME_MEASURE();
  if(err_code == NO_ERROR){
    VALIDATE_TIME_MEASURE(calc_dev)
  }
  else{
    PERF_PRINT_STRING("CREATE MUTEX: ");
    PERF_PRINT_NUMBER(err_code);
    PERF_PRINT_EOL();
  } 

  INIT_TIME_MEASURE();
  CREATE_MUTEX (Mutex3Name, priorityValue, mut_queue, &mut_id3, &err_code);
  FINISH_TIME_MEASURE();
  if(err_code == NO_ERROR){
    VALIDATE_TIME_MEASURE(calc_dev)
  }
  else{
    PERF_PRINT_STRING("CREATE MUTEX: ");
    PERF_PRINT_NUMBER(err_code);
    PERF_PRINT_EOL();
  }

  INIT_TIME_MEASURE();
  CREATE_MUTEX (Mutex4Name, priorityValue, mut_queue, &mut_id4, &err_code);
  FINISH_TIME_MEASURE();
  if(err_code == NO_ERROR){
    VALIDATE_TIME_MEASURE(calc_dev)
  }
  else{
    PERF_PRINT_STRING("CREATE MUTEX: ");
    PERF_PRINT_NUMBER(err_code);
    PERF_PRINT_EOL();
  } 
}

__attribute__((section(".p1_code")))
void test_acquire_mut(T_uint8 id, T_uint8 calc_dev)
{
  RETURN_CODE_TYPE errCode = TIMED_OUT;
  MUTEX_ID_TYPE MutexId;

  DECLARE_TIME_MEASURE()
  INITIALIZE_TIME_VARS("MUTEX_ACQUIRE")

  if (id == 0){
    GET_MUTEX_ID(Mutex1Name, &MutexId, &errCode);
    INIT_TIME_MEASURE();
    ACQUIRE_MUTEX(MutexId,INFINITE_TIME_VALUE, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
        }
    else{
        PERF_PRINT_STRING("ACQUIRE_MUTEX: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
    }
  }
  else if (id == 1){   
    GET_MUTEX_ID(Mutex2Name, &MutexId, &errCode);
    INIT_TIME_MEASURE();
    ACQUIRE_MUTEX(MutexId,INFINITE_TIME_VALUE, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
        }
    else{
        PERF_PRINT_STRING("ACQUIRE_MUTEX: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
    }
  }

  else if (id == 2){
    GET_MUTEX_ID(Mutex3Name, &MutexId, &errCode);
    INIT_TIME_MEASURE();
    ACQUIRE_MUTEX(MutexId,INFINITE_TIME_VALUE, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
        }
    else{
        PERF_PRINT_STRING("ACQUIRE_MUTEX: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
    }
  }

  else{
    GET_MUTEX_ID(Mutex4Name, &MutexId, &errCode);
    INIT_TIME_MEASURE();
    ACQUIRE_MUTEX(MutexId,INFINITE_TIME_VALUE, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
        }
    else{
        PERF_PRINT_STRING("ACQUIRE_MUTEX: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
    }
  }
}

__attribute__((section(".p1_code")))
void test_release_mut(T_uint8 id, T_uint8 calc_dev)
{
  RETURN_CODE_TYPE errCode = TIMED_OUT;
  MUTEX_ID_TYPE MutexId;

  DECLARE_TIME_MEASURE()
  INITIALIZE_TIME_VARS("MUTEX_RELEASE")

  if (id == 0){
    GET_MUTEX_ID(Mutex1Name, &MutexId, &errCode);
    INIT_TIME_MEASURE();
    RELEASE_MUTEX(MutexId, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
        }
    else{
        PERF_PRINT_STRING("RELEASE_MUTEX: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
    }
  }
  else if (id == 1){   
    GET_MUTEX_ID(Mutex2Name, &MutexId, &errCode);
    INIT_TIME_MEASURE();
    RELEASE_MUTEX(MutexId, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
        }
    else{
        PERF_PRINT_STRING("RELEASE_MUTEX: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
    }
  }

  else if (id == 2){
    GET_MUTEX_ID(Mutex3Name, &MutexId, &errCode);
    INIT_TIME_MEASURE();
    RELEASE_MUTEX(MutexId, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
        }
    else{
        PERF_PRINT_STRING("RELEASE_MUTEX: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
    }
  }
  else{
    GET_MUTEX_ID(Mutex4Name, &MutexId, &errCode);
    INIT_TIME_MEASURE();
    RELEASE_MUTEX(MutexId, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
        }
    else{
        PERF_PRINT_STRING("RELEASE_MUTEX: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
    }
  }
}


__attribute__((section(".p1_code")))
void test_id_mut(T_uint8 calc_dev)
{
    RETURN_CODE_TYPE errCode = TIMED_OUT;
    MUTEX_ID_TYPE MutexId;

    DECLARE_TIME_MEASURE()
    INITIALIZE_TIME_VARS("MUTEX_ID")
    
    INIT_TIME_MEASURE();
    GET_MUTEX_ID(Mutex1Name, &MutexId, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
        }
    else{
        PERF_PRINT_STRING("GET_MUTEX_ID: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
    }

    INIT_TIME_MEASURE();
    GET_MUTEX_ID(Mutex2Name, &MutexId, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
        }
    else{
        PERF_PRINT_STRING("GET_MUTEX_ID: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
    }

    INIT_TIME_MEASURE();
    GET_MUTEX_ID(Mutex3Name, &MutexId, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
        }
    else{
        PERF_PRINT_STRING("GET_MUTEX_ID: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
    }

    INIT_TIME_MEASURE();
    GET_MUTEX_ID(Mutex4Name, &MutexId, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
        }
    else{
        PERF_PRINT_STRING("GET_MUTEX_ID: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
    }
}

__attribute__((section(".p1_code")))
void test_status_mut(T_uint8 calc_dev)
{
    RETURN_CODE_TYPE errCode = TIMED_OUT;
    MUTEX_ID_TYPE MutexId;
    MUTEX_STATUS_TYPE status;

    DECLARE_TIME_MEASURE()
    INITIALIZE_TIME_VARS("MUTEX_STATUS")

    GET_MUTEX_ID(Mutex1Name, &MutexId, &errCode);
    INIT_TIME_MEASURE();
    GET_MUTEX_STATUS(MutexId, &status, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
        }
    else{
        PERF_PRINT_STRING("GET_MUTEX_STATUS: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
    }
    
    GET_MUTEX_ID(Mutex2Name, &MutexId, &errCode);
    INIT_TIME_MEASURE();
    GET_MUTEX_STATUS(MutexId, &status, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
        }
    else{
        PERF_PRINT_STRING("GET_MUTEX_STATUS: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
    }

    GET_MUTEX_ID(Mutex3Name, &MutexId, &errCode);
    INIT_TIME_MEASURE();
    GET_MUTEX_STATUS(MutexId, &status,  &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
        }
    else{
        PERF_PRINT_STRING("GET_MUTEX_STATUS: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
    }

    GET_MUTEX_ID(Mutex4Name, &MutexId, &errCode);
    INIT_TIME_MEASURE();
    GET_MUTEX_STATUS(MutexId, &status, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
        }
    else{
        PERF_PRINT_STRING("GET_MUTEX_STATUS: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
    }
}

__attribute__((section(".p1_code")))
void test_reset_mut(T_uint8 calc_dev)
{
    RETURN_CODE_TYPE errCode = TIMED_OUT;
    MUTEX_ID_TYPE MutexId;
    PROCESS_ID_TYPE processId;

    DECLARE_TIME_MEASURE()
    INITIALIZE_TIME_VARS("RESET_MUTEX")

    GET_PROCESS_ID("PRINT", &processId, &errCode);
    GET_MUTEX_ID(Mutex1Name, &MutexId, &errCode);
    ACQUIRE_MUTEX(MutexId,INFINITE_TIME_VALUE, &errCode);
    INIT_TIME_MEASURE();
    RESET_MUTEX(MutexId, processId, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
       VALIDATE_TIME_MEASURE(calc_dev)
       }
    else{
       PERF_PRINT_STRING("RESET_MUTEX: ");
       PERF_PRINT_NUMBER(errCode);
       PERF_PRINT_EOL();
    }

    GET_MUTEX_ID(Mutex2Name, &MutexId, &errCode);
    ACQUIRE_MUTEX(MutexId,INFINITE_TIME_VALUE, &errCode);
    INIT_TIME_MEASURE();
    RESET_MUTEX(MutexId, processId, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
        }
    else{
        PERF_PRINT_STRING("RESET_MUTEX: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
    }
  


    GET_MUTEX_ID(Mutex3Name, &MutexId, &errCode);
    ACQUIRE_MUTEX(MutexId,INFINITE_TIME_VALUE, &errCode);
    INIT_TIME_MEASURE();
    RESET_MUTEX(MutexId, processId, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
        }
    else{
        PERF_PRINT_STRING("RESET_MUTEX: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
    }
    

    GET_MUTEX_ID(Mutex4Name, &MutexId, &errCode);
    ACQUIRE_MUTEX(MutexId,INFINITE_TIME_VALUE, &errCode);
    INIT_TIME_MEASURE();
    RESET_MUTEX(MutexId, processId, &errCode);
    FINISH_TIME_MEASURE();
    if(errCode == NO_ERROR){
        VALIDATE_TIME_MEASURE(calc_dev)
        }
    else{
        PERF_PRINT_STRING("RESET_MUTEX: ");
        PERF_PRINT_NUMBER(errCode);
        PERF_PRINT_EOL();
    }
       
}

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

    return 1;
}

/* __________________________________________________________________________
* END OF FILE:
* -------------
* ___________________________________________________________________________
*/
