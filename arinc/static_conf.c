#include "ucx.h"

const int routing_table_size = 4;


__attribute__((section(".p1_code")))
void p1_main_process(struct pcb_s *partition){
    RETURN_CODE_TYPE return_code0;
    RETURN_CODE_TYPE return_code1;
    PROCESS_ID_TYPE process_id_0;
    PROCESS_ID_TYPE process_id_1;
    PROCESS_ID_TYPE process_id_2;

    CREATE_PROCESS(&P1_PROCESS_1_CONFIG, &process_id_0, &return_code0);
    CREATE_PROCESS(&P1_PROCESS_2_CONFIG, &process_id_1, &return_code1);
    CREATE_PROCESS(&P1_PROCESS_3_CONFIG, &process_id_2, &return_code0);

    // printf("CREATE PROCESS %d and Error code is %d\n", process_id_0, return_code0);
    // printf("CREATE PROCESS %d and Error code is %d\n", process_id_1, return_code1);      

    // DELAYED_START(process_id_0, 18, &return_code0);  
    // DELAYED_START(process_id_1, 25, &return_code0);  
    // printf("return code delayed start %d\n", return_code0); 
    START(process_id_0, &return_code0);  
    START(process_id_1, &return_code1);  

    START(process_id_0, &return_code0);  
    START(process_id_2, &return_code0);  


    SAMPLING_PORT_ID_TYPE port_id;
    CREATE_SAMPLING_PORT(system_port_table[0].port_name, system_port_table[0].messageSizeBytes, system_port_table[0].port_direction, system_port_table[0].refreshPeriodMs,
     &port_id, &return_code0);
    printf("return code sampling port %d, port id %d\n", return_code0, port_id); 
    
    CREATE_QUEUING_PORT(system_port_table[2].port_name, system_port_table[2].messageSizeBytes, system_port_table[2].max_nb_message, system_port_table[2].port_direction, system_port_table[2].QUEUING_DISCIPLINE,
     &port_id, &return_code1);
    printf("return code queuing port %d, port id %d\n", return_code1, port_id);

    CREATE_BLACKBOARD(blackboard_configs->blackboard_name , blackboard_configs->max_message_size, &port_id, &return_code0);
    printf("return code blackboard %d, blackboard id %d\n", return_code0, port_id);

    CREATE_BUFFER(buffer_configs[0].buffer_name, buffer_configs[0].max_message_size, buffer_configs[0].max_nb_message, buffer_configs[0].queuing_discipline, &port_id, &return_code1);
    printf("return code buffer %s %d, buffer id %d\n", buffer_configs[0].buffer_name, return_code1, port_id);

    CREATE_BUFFER(buffer_configs[1].buffer_name, buffer_configs[1].max_message_size, buffer_configs[1].max_nb_message, buffer_configs[1].queuing_discipline, &port_id, &return_code1);
    printf("return code buffer %s %d, buffer id %d\n", buffer_configs[1].buffer_name, return_code1, port_id);
    
    CREATE_SEMAPHORE(semaphore_configs->semaphore_name, semaphore_configs->current_value, semaphore_configs->maximum_value, semaphore_configs->queuing_discipline, &port_id, &return_code0);
    printf("return code semaphore %d, semaphore id %d\n", return_code0, port_id);

    CREATE_EVENT(event_configs->event_name, &port_id, &return_code1);
    printf("return code event %d, event id %d\n", return_code1, port_id);

    CREATE_MUTEX(mutex_configs[0].mutex_name, mutex_configs[0].mutex_priority, mutex_configs[0].queuing_discipline, &port_id, &return_code0);
    printf("return code mutex %d, mutex id %d\n", return_code0, port_id);
    
    SET_PARTITION_MODE(NORMAL, &return_code0);
}

__attribute__((section(".p2_code")))
void p2_main_process(struct pcb_s *partition){
    RETURN_CODE_TYPE return_code0;
    RETURN_CODE_TYPE return_code1;
    PROCESS_ID_TYPE process_id_0;
    PROCESS_ID_TYPE process_id_1;

    CREATE_PROCESS(&P2_PROCESS_1_CONFIG, &process_id_1, &return_code1);

    printf("CREATE PROCESS %d and Error code is %d\n", process_id_0, return_code0);
    printf("CREATE PROCESS %d and Error code is %d\n", process_id_1, return_code1);

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