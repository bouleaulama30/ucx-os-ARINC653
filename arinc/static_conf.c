#include "ucx.h"

const int routing_table_size = 2;

const ERROR_ACTION_TYPE hm_table_partition_1[4][4] = {
    { PROCESS_RESTART, PROCESS_STOP, PARTITION_RESTART, PROCESS_STOP },
    { PROCESS_RESTART, PROCESS_STOP, PARTITION_RESTART, PROCESS_STOP },
    { PROCESS_RESTART, PROCESS_STOP, PARTITION_RESTART, PROCESS_STOP },
    { IGNORE, PROCESS_STOP, PROCESS_RESTART, PROCESS_STOP }
};

const ERROR_ACTION_TYPE hm_table_partition_2[4][4] = {
    { PROCESS_RESTART, PROCESS_STOP, PARTITION_RESTART, PROCESS_STOP },
    { PROCESS_RESTART, PROCESS_STOP, PARTITION_RESTART, PROCESS_STOP },
    { PROCESS_RESTART, PROCESS_STOP, PARTITION_RESTART, PROCESS_STOP },
    { PROCESS_REPLENISH, PROCESS_STOP, PROCESS_RESTART, PROCESS_STOP }
};

const ERROR_ACTION_TYPE hm_table_module[1][4] = {
    { PROCESS_REPLENISH, PROCESS_STOP, PROCESS_RESTART, PROCESS_STOP }
};

__attribute__((section(".p1_code")))
void p1_main_process(struct pcb_s *partition)
{
    RETURN_CODE_TYPE return_code;
    PROCESS_ID_TYPE process_id;
    SAMPLING_PORT_ID_TYPE sp_tx_id, sp_rx_id;

    CREATE_SAMPLING_PORT("P1_OUT_ALT", sizeof(uint32_t), SOURCE, 200, &sp_tx_id, &return_code);
    CREATE_SAMPLING_PORT("P1_IN_ALT", sizeof(uint32_t), DESTINATION, 200, &sp_rx_id, &return_code);

    CREATE_PROCESS(&P1_PROCESS_1_CONFIG, &process_id, &return_code);
    START(process_id, &return_code);

    SET_PARTITION_MODE(NORMAL, &return_code);
}

__attribute__((section(".p2_code")))
void p2_main_process(struct pcb_s *partition)
{
    RETURN_CODE_TYPE return_code;
    PROCESS_ID_TYPE process_id;

    CREATE_PROCESS(&P2_PROCESS_1_CONFIG, &process_id, &return_code);
    START(process_id, &return_code);

    SET_PARTITION_MODE(NORMAL, &return_code);
}
