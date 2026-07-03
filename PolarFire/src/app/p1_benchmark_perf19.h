#ifndef P1_BENCHMARK_PERF19_H
#define P1_BENCHMARK_PERF19_H

#include "performance_lib.h"
typedef double float64_t;

// Parameters from TestParameterP1.h
#define MAX_INTERCRC_QUEUE_SIZE 8
#define DATA_POOL_ROW_SIZE 1500
#define DATA_POOL_COL_SIZE MAX_INTERCRC_QUEUE_SIZE
#define MATH_COMPUTATION_ITER 88878
#define TARGET_TIME_OUT 1000000

// CalCRC32.h
#define LOOKUP_TABLE_SIZE 256

// Declarations
void InitializeLookupTable(void);
uint32_t CalcCRC32(uint8_t *buffer, uint32_t bufferSize);
void InitializeSampleApexP1Resources(void);
void InitializeSampleApexP1Processes(void);
void InitSyncWithApplicationB(void);
void InitializeSampleApexP1DataPool(void);
void InitTimeReferencesP1(void);
void StartPerformanceMeasurementsP1(void);
void ProcessMonitoring(void);
void ProcessInnerLoop(void);

// Variables
PROCESS_ID_TYPE ProcessMonitoringId = 0;
PROCESS_ID_TYPE ProcessInnerLoopId = 0;

SAMPLING_PORT_ID_TYPE SamplingCommandSrcId = 0;
SAMPLING_PORT_ID_TYPE SamplingCommandDstId = 0;
SAMPLING_PORT_ID_TYPE SamplingTargetDstId = 0;
SAMPLING_PORT_ID_TYPE SamplingInitSyncASrcId = 0;
SAMPLING_PORT_ID_TYPE SamplingInitSyncBDstId = 0;
SAMPLING_PORT_ID_TYPE SamplingInterCRCSrcId = 0;

uint32_t DataPool[DATA_POOL_ROW_SIZE][DATA_POOL_COL_SIZE];
uint64_t twoMilliSecinTicks;
static uint32_t countProcInner = 0;
static uint32_t countProcMonCrc = 0;
static uint32_t sCRCLookupTable[LOOKUP_TABLE_SIZE];

// Functions
#define G	0x04C11DB7
__attribute__((section(".p1_code")))
static uint32_t calLookUpElem(uint32_t index) 
{
	uint32_t generator = G;
	uint32_t leading_bit;
	uint32_t shift_reg = (index << 24);
	for (int32_t k = 7; k >= 0; k--) {
		leading_bit = shift_reg & 0x80000000;
		shift_reg = shift_reg << 1;
		if (0 != leading_bit) {
			shift_reg = shift_reg ^ generator;
		}
	}
	return shift_reg;
}

__attribute__((section(".p1_code")))
void InitializeLookupTable(void)
{
	for (int32_t i = 0; i < LOOKUP_TABLE_SIZE; i++) {
		sCRCLookupTable[i] = calLookUpElem((uint32_t)i);
	}	
}

#define Init 0xFFFFFFFF
#define XorOut 0xFFFFFFFF
__attribute__((section(".p1_code")))
uint32_t CalcCRC32(uint8_t *buffer, uint32_t bufferSize) {
	uint32_t i;
	uint32_t k;
	uint32_t crc = Init;
	for (i = 0; i < bufferSize; i++) {
		k = ((crc >> 24) ^ (int32_t) buffer[i]) & 0xFF;
		crc = (crc << 8) ^ sCRCLookupTable[k];
	}
	return (crc ^ XorOut);
}

__attribute__((section(".p1_code")))
void InitializeSampleApexP1Resources(void)
{
  RETURN_CODE_TYPE errCode = NO_ERROR;
  CREATE_SAMPLING_PORT("Command_Src", 8, SOURCE, 30000000, &SamplingCommandSrcId, &errCode);
  CREATE_SAMPLING_PORT("Command_Dst", 8, DESTINATION, 30000000, &SamplingCommandDstId, &errCode);
  CREATE_SAMPLING_PORT("Target_Dst", 8, DESTINATION, TARGET_TIME_OUT, &SamplingTargetDstId, &errCode);
  CREATE_SAMPLING_PORT("InitSyncA_Src", 8, SOURCE, 30000000, &SamplingInitSyncASrcId, &errCode);
  CREATE_SAMPLING_PORT("InitSyncB_Dst", 8, DESTINATION, 30000000, &SamplingInitSyncBDstId, &errCode);
  CREATE_SAMPLING_PORT("InterCRC_Src", 4, SOURCE, 30000000, &SamplingInterCRCSrcId, &errCode);
}

__attribute__((section(".p1_code")))
void InitializeSampleApexP1Processes(void)
{
  PROCESS_ATTRIBUTE_TYPE process1Attrib;
  PROCESS_ATTRIBUTE_TYPE process2Attrib;
  PROCESS_STATUS_TYPE process1Status;
  PROCESS_STATUS_TYPE process2Status;
  RETURN_CODE_TYPE errCode = NO_ERROR;

  strncpy(process1Attrib.NAME, "Monitoring", MAX_NAME_LENGTH);
  process1Attrib.BASE_PRIORITY = 50;
  process1Attrib.DEADLINE = SOFT;
  process1Attrib.ENTRY_POINT = (SYSTEM_ADDRESS_TYPE)&ProcessMonitoring;
  process1Attrib.PERIOD = INFINITE_TIME_VALUE;
  process1Attrib.STACK_SIZE = 4*1024;
  process1Attrib.TIME_CAPACITY = INFINITE_TIME_VALUE;

  strncpy(process2Attrib.NAME, "InnerLoop", MAX_NAME_LENGTH);
  process2Attrib.BASE_PRIORITY = 100;
  process2Attrib.DEADLINE = SOFT;
  process2Attrib.ENTRY_POINT = (SYSTEM_ADDRESS_TYPE)&ProcessInnerLoop;
  process2Attrib.PERIOD = 100; // 100ms (Must be a multiple of partition period = 100ms)
  process2Attrib.STACK_SIZE = 4*1024;
  process2Attrib.TIME_CAPACITY = 25; // 25ms (OS expects milliseconds)

  CREATE_PROCESS(&process1Attrib, &ProcessMonitoringId, &errCode);
  START(ProcessMonitoringId, &errCode);

  CREATE_PROCESS(&process2Attrib, &ProcessInnerLoopId, &errCode);
  START(ProcessInnerLoopId, &errCode);
}

__attribute__((section(".p1_code")))
void InitSyncWithApplicationB(void)
{
  char_t tempMsg[8];
  MESSAGE_SIZE_TYPE rxMsgSize = 0;
  VALIDITY_TYPE isMsgValid = INVALID;
  RETURN_CODE_TYPE errCode = NO_ERROR;

  strncpy(tempMsg, "A_READY", sizeof(tempMsg));
  WRITE_SAMPLING_MESSAGE(SamplingInitSyncASrcId, (MESSAGE_ADDR_TYPE)tempMsg, sizeof(tempMsg), &errCode);

  while (1)
    {
      READ_SAMPLING_MESSAGE(SamplingInitSyncBDstId, (MESSAGE_ADDR_TYPE)tempMsg, &rxMsgSize, &isMsgValid, &errCode);
      if(errCode == NO_ERROR)
      {
          if (!strcmp("B_READY", tempMsg))
          {
            break;
          }
      }
    }
}

__attribute__((section(".p1_code")))
void InitializeSampleApexP1DataPool(void)
{
  uint32_t i, j;
  for (i = 0; i < DATA_POOL_ROW_SIZE; i++)
  {
    for(j = 0; j < DATA_POOL_COL_SIZE; j++)
    {
      DataPool[i][j] = i*j;
    }    
  }
  InitializeLookupTable();
}

__attribute__((section(".p1_code")))
static void mathComputation (float64_t target, float64_t * command)
{
  float64_t val = target;
  volatile uint64_t i = 0;
  uint32_t count=0;
  for (count = 0; count < MATH_COMPUTATION_ITER; count++)
    {
      if (i++ & 0x1)
        {
          val *= (float64_t) 1.0;
        }
      else
        {
          val /= (float64_t) 1.0;
        }
    }
  *command = val;
}

__attribute__((section(".p1_code")))
void InitTimeReferencesP1 (void)
{
  twoMilliSecinTicks = perf_ns_to_ticks(2000000);
}

__attribute__((section(".p1_code")))
void StartPerformanceMeasurementsP1 (void)
{
  DECLARE_TIME_MEASURE();
  INITIALIZE_TIME_VARS("SAMPLE_APEX_APP_A");
  INIT_TIME_MEASURE();
}

__attribute__((section(".p1_code")))
void ProcessMonitoring(void)
{
  uint32_t crcResultBytes;
  RETURN_CODE_TYPE errCode = NO_ERROR;
  uint32_t i = 0, j = 0;
  uint64_t last_tick = GET_CURRENT_TICKS();
  RETURN_CODE_TYPE dummy_ret;
  while (1)
    {
      countProcMonCrc++;
      crcResultBytes = CalcCRC32((uint8_t*) DataPool[i], (uint32_t)DATA_POOL_COL_SIZE);
      WRITE_SAMPLING_MESSAGE(SamplingInterCRCSrcId, (MESSAGE_ADDR_TYPE)&crcResultBytes, sizeof(crcResultBytes), &errCode);
      if (++i >= DATA_POOL_ROW_SIZE)
        {
          if (j > 40)
            {
              DECLARE_TIME_MEASURE();
              INITIALIZE_TIME_VARS("SAMPLE_APEX_APP_A");
              FINISH_TIME_MEASURE();
              VALIDATE_TIME_MEASURE(0);
              PRINT_PERFORMANCE_INFO();

              PERF_PRINT_STRING("Proc. Inner Loop count=");
              PERF_PRINT_UNSIGNED(countProcInner);
              PERF_PRINT_EOL();
              PERF_PRINT_STRING("Proc. Monitoring count=");
              PERF_PRINT_UNSIGNED(countProcMonCrc);
              PERF_PRINT_EOL();
              break;
            }
          else
            {
              j++;
              i = 0;
            }
        }
      uint64_t current_tick = GET_CURRENT_TICKS();
      if (current_tick != last_tick) {
          last_tick = current_tick;
          TIMED_WAIT(0, &dummy_ret);
      }
    }
  STOP_SELF();
}

__attribute__((section(".p1_code")))
void ProcessInnerLoop(void)
{
  float64_t rxTargetMsg=0;
  float64_t txCommandMsg;
  MESSAGE_SIZE_TYPE rxTargetMsgSize = 0;
  VALIDITY_TYPE isMsgValid = INVALID;
  RETURN_CODE_TYPE errCode = NO_ERROR;
  uint64_t urxTargetMsg;
  while (1)
    {
      countProcInner++;
      rxTargetMsg = 0;
      READ_SAMPLING_MESSAGE(SamplingTargetDstId, (MESSAGE_ADDR_TYPE) &urxTargetMsg, &rxTargetMsgSize, &isMsgValid, &errCode);
      mathComputation((float64_t) rxTargetMsg, &txCommandMsg);
      WRITE_SAMPLING_MESSAGE(SamplingCommandSrcId, (MESSAGE_ADDR_TYPE) &txCommandMsg, sizeof(txCommandMsg), &errCode);
      PERIODIC_WAIT(&errCode);
    }
}

__attribute__((section(".p1_code")))
void MAIN_FUNCTION()
{
  RETURN_CODE_TYPE errCode = NO_ERROR;
  InitializeSampleApexP1DataPool();
  InitTimeReferencesP1();
  InitializeSampleApexP1Resources();
  InitializeSampleApexP1Processes();
  InitSyncWithApplicationB();
  StartPerformanceMeasurementsP1();
  SET_PARTITION_MODE(NORMAL, &errCode);
  while (1) {}
}

#endif // P1_BENCHMARK_PERF19_H
