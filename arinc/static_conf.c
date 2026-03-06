#include "ucx.h"

__attribute__((section(".p1_code")))
void p1_main_process(struct pcb_s *partition){
    RETURN_CODE_TYPE return_code0;
    RETURN_CODE_TYPE return_code1;
    PROCESS_ID_TYPE process_id_0;
    PROCESS_ID_TYPE process_id_1;

    CREATE_PROCESS(&DEFAULT_PROCESS_CONFIG, &process_id_0, &return_code0);
    CREATE_PROCESS(&PROCESS_1_CONFIG, &process_id_1, &return_code1);

    printf("CREATE PROCESS %d and Error code is %d\n", process_id_0, return_code0);
    printf("CREATE PROCESS %d and Error code is %d\n", process_id_1, return_code1);      

    START(process_id_0, &return_code0);  
    START(process_id_1, &return_code1);  

    SET_PARTITION_MODE(NORMAL, &return_code0);
}

__attribute__((section(".p2_code")))
void p2_main_process(struct pcb_s *partition){
    RETURN_CODE_TYPE return_code0;
    RETURN_CODE_TYPE return_code1;
    PROCESS_ID_TYPE process_id_0;
    PROCESS_ID_TYPE process_id_1;

    CREATE_PROCESS(&PROCESS_2_CONFIG, &process_id_0, &return_code0);
    CREATE_PROCESS(&PROCESS_3_CONFIG, &process_id_1, &return_code1);

    printf("CREATE PROCESS %d and Error code is %d\n", process_id_0, return_code0);
    printf("CREATE PROCESS %d and Error code is %d\n", process_id_1, return_code1);

    START(process_id_0, &return_code0);  
    START(process_id_1, &return_code1);  


    SET_PARTITION_MODE(NORMAL, &return_code0);
}