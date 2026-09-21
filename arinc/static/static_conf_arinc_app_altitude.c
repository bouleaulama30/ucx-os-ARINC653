#include "ucx.h"

const int routing_table_size = sizeof(system_port_table) / sizeof(system_port_table[0]);

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
    { PROCESS_REPLENISH, PROCESS_STOP, PROCESS_RESTART, PROCESS_STOP }
};

__attribute__((section(".p1_code")))
void p1_main_process(struct pcb_s *partition){
    RETURN_CODE_TYPE return_code0;
    PROCESS_ID_TYPE process_id_0;
    SAMPLING_PORT_ID_TYPE port_p1_out;

    // Creation du Sampling Port P1_OUT_ALT (SOURCE)
    CREATE_SAMPLING_PORT("P1_OUT_ALT", 64, SOURCE, 200, &port_p1_out, &return_code0);
    printf("[P1 Init] CREATE_SAMPLING_PORT P1_OUT_ALT rc=%d (port_id=%d)\n", return_code0, port_p1_out);

    // Creation et demarrage du Processus 1 (Capture IPC & Ecriture Sampling Port)
    CREATE_PROCESS(&P1_PROCESS_1_CONFIG, &process_id_0, &return_code0);
    printf("[P1 Init] CREATE_PROCESS %s (id=%d, rc=%d)\n", P1_PROCESS_1_CONFIG.NAME, process_id_0, return_code0);      

    START(process_id_0, &return_code0);  

    SET_PARTITION_MODE(NORMAL, &return_code0);
}

__attribute__((section(".p2_code")))
void p2_main_process(struct pcb_s *partition){
    RETURN_CODE_TYPE return_code0;
    PROCESS_ID_TYPE process_id_0;
    SAMPLING_PORT_ID_TYPE port_p2_in;

    // Creation du Sampling Port P2_IN_ALT (DESTINATION)
    CREATE_SAMPLING_PORT("P2_IN_ALT", 64, DESTINATION, 200, &port_p2_in, &return_code0);
    printf("[P2 Init] CREATE_SAMPLING_PORT P2_IN_ALT rc=%d (port_id=%d)\n", return_code0, port_p2_in);

    // Creation et demarrage du Processus 1 (Lecture Sampling Port & Log UART)
    CREATE_PROCESS(&P2_PROCESS_1_CONFIG, &process_id_0, &return_code0);
    printf("[P2 Init] CREATE_PROCESS %s (id=%d, rc=%d)\n", P2_PROCESS_1_CONFIG.NAME, process_id_0, return_code0);

    START(process_id_0, &return_code0);  
    
    SET_PARTITION_MODE(NORMAL, &return_code0);
}
