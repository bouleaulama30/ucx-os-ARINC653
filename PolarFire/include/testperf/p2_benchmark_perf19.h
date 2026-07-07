#ifndef P2_BENCHMARK_PERF19_H
#define P2_BENCHMARK_PERF19_H

#include "performance_lib.h"

// Parameters from TestParametersP2.h
#define MAX_INTERCRC_QUEUE_SIZE 8
#define MAX_LOGBUFFER_SIZE MAX_INTERCRC_QUEUE_SIZE
#define GCD_VALUE 3
#define MAX_MAXTRIX_ROW 5
#define MAX_MAXTRIX_COL 8
#define WRITE_DISK_ITER 1333170 
#define TARGET_TIME_OUT 1000000

// P2.h / declarations
void InitializeSampleApexP2Resources(void);
void InitializeSampleApexP2Processes(void);
void InitSyncWithApplicationA(void);
void InitFibbonacciSequenceMatrix(void);
void InitSquareSequenceMatrix(void);
void InitTimeReferencesP2(void);
void StartPerformanceMeasurementsP2(void);
void ProcessLogger(void);
void ProcessCRCMonitor(void);
void ProcessMatrixMultiplier(void);
void p2_main(void);

// Variables
PROCESS_ID_TYPE ProcessLoggerId = 0;
PROCESS_ID_TYPE ProcessCRCMonitorId = 0;
PROCESS_ID_TYPE ProcessMatrixMultiplierId = 0;

SAMPLING_PORT_ID_TYPE SamplingTargetSrcId = 0;
SAMPLING_PORT_ID_TYPE SamplingInitSyncADstId = 0;
SAMPLING_PORT_ID_TYPE SamplingInitSyncBSrcId = 0;
SAMPLING_PORT_ID_TYPE SamplingInterCRCDstId = 0;

SEMAPHORE_ID_TYPE SemaMutexId = 0;
SEMAPHORE_ID_TYPE SemaEmptyId = 0;
SEMAPHORE_ID_TYPE SemaFullId = 0;

static uint32_t countProcLog = 0;
static uint32_t countProcCrc = 0;
static uint32_t countProcMatrix = 0;

uint32_t logBuffer[MAX_LOGBUFFER_SIZE];
uint64_t matrixA[MAX_MAXTRIX_ROW][MAX_MAXTRIX_COL];
uint64_t matrixB[MAX_MAXTRIX_ROW][MAX_MAXTRIX_COL];
uint64_t tenMilliSecinTicks;
uint64_t countWriteToBuffer = 0;

// Functions
__attribute__((section(".p2_code")))
static int32_t computeGreatestCommonDivider (int32_t a, int32_t b)
{
  return (b != 0) ? computeGreatestCommonDivider(b, a % b) : a;
}

__attribute__((section(".p2_code")))
static void simulateWriteToDisk (float32_t data_in, float32_t * data_out)
{
  float32_t val = data_in;
  volatile uint64_t i = 0;
  uint32_t count=0;
  for (count = 0; count < WRITE_DISK_ITER; count++)
    {
      if (i++ & 0x1)
        {
          val += (float32_t) 1.0;
        }
      else
        {
          val -= (float32_t) 1.0;
        }
    }
  *data_out = val;
}

__attribute__((section(".p2_code")))
void InitFibbonacciSequenceMatrix (void)
{
  uint64_t current = 0, next = 1;
  uint32_t i = 0, j = 0;
  matrixA[i][0] = current;
  matrixA[i][1] = next;
  for (i = 0; i < MAX_MAXTRIX_ROW; i++)
    {
      if (0 == i) { j = 2; }
      else { j = 0; }
      for (; j < MAX_MAXTRIX_COL; j++)
        {
          matrixA[i][j] = current + next;
          current = next;
          next = matrixA[i][j];
        }
    }
}

__attribute__((section(".p2_code")))
void InitSquareSequenceMatrix (void)
{
  uint64_t k = 0;
  uint32_t i = 0, j = 0;
  for (i = 0; i < MAX_MAXTRIX_ROW; i++)
    {
      for (j = 0; j < MAX_MAXTRIX_COL; j++)
        {
          matrixB[i][j] = k * k;
          k++;
        }
    }
}

__attribute__((section(".p2_code")))
void InitTimeReferencesP2 (void)
{
  tenMilliSecinTicks = perf_ns_to_ticks(10000000);
}

__attribute__((section(".p2_code")))
void StartPerformanceMeasurementsP2 (void)
{
  DECLARE_TIME_MEASURE();
  INITIALIZE_TIME_VARS("SAMPLE_APEX_APP_B");
  INIT_TIME_MEASURE();
}

__attribute__((section(".p2_code")))
void InitializeSampleApexP2Resources(void)
{
  RETURN_CODE_TYPE errCode = NO_ERROR;
  CREATE_SAMPLING_PORT("Target_Src", 8, SOURCE, TARGET_TIME_OUT, &SamplingTargetSrcId, &errCode);
  CREATE_SAMPLING_PORT("InitSyncA_Dst", 8, DESTINATION, 30000000, &SamplingInitSyncADstId, &errCode);
  CREATE_SAMPLING_PORT("InitSyncB_Src", 8, SOURCE, 30000000, &SamplingInitSyncBSrcId, &errCode);
  CREATE_SAMPLING_PORT("InterCRC_Dst", 4, DESTINATION, 30000000, &SamplingInterCRCDstId, &errCode);
  CREATE_SEMAPHORE("mutex", 1, 1, FIFO, &SemaMutexId, &errCode);
  CREATE_SEMAPHORE("empty", MAX_INTERCRC_QUEUE_SIZE, MAX_INTERCRC_QUEUE_SIZE, FIFO, &SemaEmptyId, &errCode);
  CREATE_SEMAPHORE("full", 0, MAX_INTERCRC_QUEUE_SIZE, FIFO, &SemaFullId, &errCode);
}

__attribute__((section(".p2_code")))
void InitializeSampleApexP2Processes(void)
{
  PROCESS_ATTRIBUTE_TYPE process1Attrib;
  PROCESS_ATTRIBUTE_TYPE process2Attrib;
  PROCESS_ATTRIBUTE_TYPE process3Attrib;
  PROCESS_STATUS_TYPE process1Status;
  PROCESS_STATUS_TYPE process2Status;
  PROCESS_STATUS_TYPE process3Status;
  RETURN_CODE_TYPE errCode = NO_ERROR;

  strncpy(process1Attrib.NAME, "Logger", MAX_NAME_LENGTH);
  process1Attrib.BASE_PRIORITY = 150;
  process1Attrib.DEADLINE = SOFT;
  process1Attrib.ENTRY_POINT = (SYSTEM_ADDRESS_TYPE)&ProcessLogger;
  process1Attrib.PERIOD = INFINITE_TIME_VALUE;
  process1Attrib.STACK_SIZE = 4*1024;
  process1Attrib.TIME_CAPACITY = INFINITE_TIME_VALUE;

  strncpy(process2Attrib.NAME, "CRCMonitor", MAX_NAME_LENGTH);
  process2Attrib.BASE_PRIORITY = 100;
  process2Attrib.DEADLINE = SOFT;
  process2Attrib.ENTRY_POINT = (SYSTEM_ADDRESS_TYPE)&ProcessCRCMonitor;
  process2Attrib.PERIOD = 100; // 100ms (Must be a multiple of partition period = 100ms)
  process2Attrib.STACK_SIZE = 4*1024;
  process2Attrib.TIME_CAPACITY = INFINITE_TIME_VALUE;

  strncpy(process3Attrib.NAME, "MatrixMultiplier", MAX_NAME_LENGTH);
  process3Attrib.BASE_PRIORITY = 50;
  process3Attrib.DEADLINE = SOFT;
  process3Attrib.ENTRY_POINT = (SYSTEM_ADDRESS_TYPE)&ProcessMatrixMultiplier;
  process3Attrib.PERIOD = INFINITE_TIME_VALUE;
  process3Attrib.STACK_SIZE = 4*1024;
  process3Attrib.TIME_CAPACITY = INFINITE_TIME_VALUE;

  CREATE_PROCESS(&process1Attrib, &ProcessLoggerId, &errCode);
  START(ProcessLoggerId, &errCode);

  CREATE_PROCESS(&process2Attrib, &ProcessCRCMonitorId, &errCode);
  START(ProcessCRCMonitorId, &errCode);

  CREATE_PROCESS(&process3Attrib, &ProcessMatrixMultiplierId, &errCode);
  START(ProcessMatrixMultiplierId, &errCode);
}

__attribute__((section(".p2_code")))
void InitSyncWithApplicationA(void)
{
  char_t tempMsg[8];
  MESSAGE_SIZE_TYPE rxMsgSize = 0;
  VALIDITY_TYPE     isMsgValid = INVALID;
  RETURN_CODE_TYPE errCode = NO_ERROR;
  while(1)
    {
      READ_SAMPLING_MESSAGE(SamplingInitSyncADstId, (MESSAGE_ADDR_TYPE)tempMsg, &rxMsgSize, &isMsgValid, &errCode);
      if(errCode == NO_ERROR)
      {
        if (!strcmp("A_READY", tempMsg))
          {
            break;
          }
      }
    }
  strncpy(tempMsg, "B_READY", sizeof(tempMsg));
  WRITE_SAMPLING_MESSAGE(SamplingInitSyncBSrcId, (MESSAGE_ADDR_TYPE)tempMsg, sizeof(tempMsg), &errCode);
}

__attribute__((section(".p2_code")))
void ProcessLogger(void)
{
  SEMAPHORE_STATUS_TYPE semaFullStatus;
  uint32_t count;
  float32_t ret;
  SYSTEM_TIME_TYPE timeOut = INFINITE_TIME_VALUE;
  RETURN_CODE_TYPE errCode = NO_ERROR;
  while (1)
    {
      countProcLog++;
      WAIT_SEMAPHORE(SemaFullId, timeOut, &errCode);
      WAIT_SEMAPHORE(SemaMutexId, timeOut, &errCode);

      GET_SEMAPHORE_STATUS(SemaFullId, &semaFullStatus, &errCode);
      count = (uint32_t)semaFullStatus.CURRENT_VALUE;
      uint32_t txTargetMsg = logBuffer[count];

      SIGNAL_SEMAPHORE(SemaMutexId, &errCode);
      SIGNAL_SEMAPHORE(SemaEmptyId, &errCode);

      simulateWriteToDisk((float32_t)txTargetMsg,&ret);

      WRITE_SAMPLING_MESSAGE(SamplingTargetSrcId, (MESSAGE_ADDR_TYPE)&ret, sizeof(ret), &errCode);
    }
}

__attribute__((section(".p2_code")))
void ProcessCRCMonitor(void)
{
  int32_t gcd;
  uint32_t count;
  uint32_t rxCrcDataMsg;
  MESSAGE_SIZE_TYPE rxCrcDataMsgSize = 0;
  SEMAPHORE_STATUS_TYPE semaFullStatus;
  SYSTEM_TIME_TYPE timeOut = INFINITE_TIME_VALUE;
  VALIDITY_TYPE isMsgValid = INVALID;
  RETURN_CODE_TYPE errCode = NO_ERROR;
  while (1)
    {
      countProcCrc++;
      READ_SAMPLING_MESSAGE(SamplingInterCRCDstId, (MESSAGE_ADDR_TYPE) &rxCrcDataMsg, &rxCrcDataMsgSize, &isMsgValid, &errCode);
      if (errCode == NO_ERROR)
        {
          RETURN_CODE_TYPE semaRC = NO_ERROR;
          gcd = computeGreatestCommonDivider((int32_t) rxCrcDataMsg, (int32_t) GCD_VALUE);
          WAIT_SEMAPHORE(SemaEmptyId, timeOut, &semaRC);
          WAIT_SEMAPHORE(SemaMutexId, timeOut, &semaRC);

          GET_SEMAPHORE_STATUS(SemaFullId, &semaFullStatus, &semaRC);
          count = (uint32_t) semaFullStatus.CURRENT_VALUE;
          logBuffer[count] = (uint32_t) gcd;
          countWriteToBuffer++;

          SIGNAL_SEMAPHORE(SemaMutexId, &semaRC);
          SIGNAL_SEMAPHORE(SemaFullId, &semaRC);
        }
      PERIODIC_WAIT(&errCode);
    }
}

__attribute__((section(".p2_code")))
void ProcessMatrixMultiplier(void)
{
  SEMAPHORE_STATUS_TYPE semaFullStatus;
  SYSTEM_TIME_TYPE timeOut = INFINITE_TIME_VALUE;
  uint64_t sum = 0, result = 0;
  uint32_t i = 0, j = 0, count = 0;
  RETURN_CODE_TYPE errCode = NO_ERROR;
  uint64_t last_tick = GET_CURRENT_TICKS();
  RETURN_CODE_TYPE dummy_ret;
  while (1)
    {
      countProcMatrix++;
      sum = 0;
      for (j = 0; j < MAX_MAXTRIX_COL; j++)
        {
          result = matrixA[i][j] * matrixB[i][j];
          sum += result;
        }

      WAIT_SEMAPHORE(SemaEmptyId, timeOut, &errCode);
      WAIT_SEMAPHORE(SemaMutexId, timeOut, &errCode);

      GET_SEMAPHORE_STATUS(SemaFullId, &semaFullStatus, &errCode);
      count = (uint32_t)semaFullStatus.CURRENT_VALUE;
      logBuffer[count] = (uint32_t)sum;

      SIGNAL_SEMAPHORE(SemaMutexId, &errCode);
      SIGNAL_SEMAPHORE(SemaFullId, &errCode);

      if (++i >= MAX_MAXTRIX_ROW)
        {
          DECLARE_TIME_MEASURE();
          INITIALIZE_TIME_VARS("SAMPLE_APEX_APP_B");
          FINISH_TIME_MEASURE();
          VALIDATE_TIME_MEASURE(0);
          PRINT_PERFORMANCE_INFO();

          PERF_PRINT_STRING("Proc. Logger count=");
          PERF_PRINT_UNSIGNED(countProcLog);
          PERF_PRINT_EOL();
          PERF_PRINT_STRING("Proc. CRCMonitor count=");
          PERF_PRINT_UNSIGNED(countProcCrc);
          PERF_PRINT_EOL();
          PERF_PRINT_STRING("Proc. MatrixMultiplier count=");
          PERF_PRINT_UNSIGNED(countProcMatrix);
          PERF_PRINT_EOL();
          break;
        }
      uint64_t current_tick = GET_CURRENT_TICKS();
      if (current_tick != last_tick) {
          last_tick = current_tick;
          TIMED_WAIT(0, &dummy_ret);
      }
    }
  STOP_SELF();
}

__attribute__((section(".p2_code")))
void p2_main(void)
{
  RETURN_CODE_TYPE errCode = NO_ERROR;
  InitFibbonacciSequenceMatrix();
  InitSquareSequenceMatrix();
  InitTimeReferencesP2();
  InitializeSampleApexP2Resources();
  InitializeSampleApexP2Processes();
  InitSyncWithApplicationA();
  StartPerformanceMeasurementsP2();
  SET_PARTITION_MODE(NORMAL, &errCode);
  while (1) {}
}

#endif // P2_BENCHMARK_PERF19_H
