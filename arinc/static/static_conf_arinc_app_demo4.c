#include "ucx.h"

const int routing_table_size = 4;

const ERROR_ACTION_TYPE hm_table_partition_1[4][4] = {
    // Colonnes : DEADLINE_MISSED, APPLICATION_ERROR, NUMERIC_ERROR, DEFAULT
    
    // Ligne 0 : État IDLE
    {  PROCESS_RESTART, PROCESS_STOP, PARTITION_RESTART, PROCESS_STOP },

    // Ligne 1 : État COLD_START
    {  PROCESS_RESTART, PROCESS_STOP, PARTITION_RESTART, PROCESS_STOP },

    // Ligne 2 : État WARM_START
    {  PROCESS_RESTART, PROCESS_STOP, PARTITION_RESTART, PROCESS_STOP },
    
    // Ligne 3 : État NORMAL
    { IGNORE, PROCESS_STOP, PROCESS_RESTART, PROCESS_STOP }
};

const ERROR_ACTION_TYPE hm_table_partition_2[4][4] = {
    // Colonnes : DEADLINE_MISSED, APPLICATION_ERROR, NUMERIC_ERROR, DEFAULT
    
    // Ligne 0 : État IDLE
    {  PROCESS_RESTART, PROCESS_STOP, PARTITION_RESTART, PROCESS_STOP },

    // Ligne 1 : État COLD_START
    {  PROCESS_RESTART, PROCESS_STOP, PARTITION_RESTART, PROCESS_STOP },

    // Ligne 2 : État WARM_START
    {  PROCESS_RESTART, PROCESS_STOP, PARTITION_RESTART, PROCESS_STOP },
    
    // Ligne 3 : État NORMAL
    { PROCESS_REPLENISH, PROCESS_STOP, PROCESS_RESTART, PROCESS_STOP }
};

const ERROR_ACTION_TYPE hm_table_module[1][4] = {
    // Colonnes : DEADLINE_MISSED, APPLICATION_ERROR, NUMERIC_ERROR, DEFAULT
    
    // // Ligne 0 : État IDLE
    // {  PROCESS_RESTART, PROCESS_STOP, PARTITION_RESTART, PROCESS_STOP },

    // // Ligne 1 : État COLD_START
    // {  PROCESS_RESTART, PROCESS_STOP, PARTITION_RESTART, PROCESS_STOP },

    // // Ligne 2 : État WARM_START
    // {  PROCESS_RESTART, PROCESS_STOP, PARTITION_RESTART, PROCESS_STOP },
    
    // Ligne 3 : État NORMAL
    { PROCESS_REPLENISH, PROCESS_STOP, PROCESS_RESTART, PROCESS_STOP }
};

__attribute__((section(".p1_code")))
void p1_main_process(struct pcb_s *partition){
    RETURN_CODE_TYPE return_code0;
    RETURN_CODE_TYPE return_code1;
    PROCESS_ID_TYPE process_id_0;
    PROCESS_ID_TYPE process_id_1;


    CREATE_PROCESS(&P1_PROCESS_1_CONFIG, &process_id_0, &return_code0);
    printf("CREATE PROCESS %d and Error code is %d\n", process_id_0, return_code0);      
    CREATE_PROCESS(&P1_PROCESS_2_CONFIG, &process_id_1, &return_code0);
    printf("CREATE PROCESS %d and Error code is %d\n", process_id_1, return_code0);      
   
    START(process_id_0, &return_code0);  
    START(process_id_1, &return_code1);  

    SAMPLING_PORT_ID_TYPE port_id;
    CREATE_SAMPLING_PORT(system_port_table[0].port_name, system_port_table[0].messageSizeBytes, system_port_table[0].port_direction, system_port_table[0].refreshPeriodMs,
     &port_id, &return_code0);
    printf("return code sampling port %d, port id %d\n", return_code0, port_id); 
    
    CREATE_QUEUING_PORT(system_port_table[2].port_name, system_port_table[2].messageSizeBytes, system_port_table[2].max_nb_message, system_port_table[2].port_direction, system_port_table[2].QUEUING_DISCIPLINE,
     &port_id, &return_code1);
    printf("return code queuing port %d, port id %d\n", return_code1, port_id);

    SET_PARTITION_MODE(NORMAL, &return_code0);
}

__attribute__((section(".p2_code")))
void p2_main_process(struct pcb_s *partition){
    RETURN_CODE_TYPE return_code0;
    RETURN_CODE_TYPE return_code1;
    PROCESS_ID_TYPE process_id_0;
    PROCESS_ID_TYPE process_id_1;

    CREATE_PROCESS(&P2_PROCESS_1_CONFIG, &process_id_0, &return_code1);
    CREATE_PROCESS(&P2_PROCESS_2_CONFIG, &process_id_1, &return_code0);
    printf("CREATE PROCESS %d and Error code is %d\n", process_id_0, return_code0);
    printf("CREATE PROCESS %d and Error code is %d\n", process_id_1, return_code1);

    START(process_id_0, &return_code1);  
    START(process_id_1, &return_code1);  
    
    SAMPLING_PORT_ID_TYPE port_id;
    CREATE_SAMPLING_PORT(system_port_table[1].port_name, system_port_table[1].messageSizeBytes, system_port_table[1].port_direction, system_port_table[1].refreshPeriodMs,
    &port_id, &return_code0);
    printf("return code sampling port %d, port id %d\n", return_code0, port_id); 

    CREATE_QUEUING_PORT(system_port_table[3].port_name, system_port_table[3].messageSizeBytes, system_port_table[3].max_nb_message, system_port_table[3].port_direction, system_port_table[3].QUEUING_DISCIPLINE,
     &port_id, &return_code1);
    printf("return code queuing port %d, port id %d\n", return_code1, port_id);

    SET_PARTITION_MODE(NORMAL, &return_code0);
}