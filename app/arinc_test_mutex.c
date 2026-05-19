#include <ucx.h>
#include "arinc_test_support.h"

static arinc_test_suite_result_t mutex_p1_suite;

static void mutex_expect_status(arinc_test_suite_result_t *suite,
                                const char *state_case,
                                const char *lock_case,
                                const char *waiting_case,
                                const MUTEX_STATUS_TYPE *status,
                                MUTEX_STATE_TYPE expected_state,
                                LOCK_COUNT_TYPE expected_lock,
                                WAITING_RANGE_TYPE expected_waiting)
{
    arinc_test_suite_check(suite, state_case, arinc_test_check_int(expected_state, status->MUTEX_STATE));
    arinc_test_suite_check(suite, lock_case, arinc_test_check_int(expected_lock, status->LOCK_COUNT));
    arinc_test_suite_check(suite, waiting_case, arinc_test_check_int(expected_waiting, status->WAITING_PROCESSES));
}

__attribute__((section(".p1_code")))
static void test_get_mutex_id_p1(void)
{
    RETURN_CODE_TYPE return_code;
    MUTEX_ID_TYPE mutex_id;
    MUTEX_ID_TYPE invalid_mutex_id;

    arinc_test_suite_begin(&mutex_p1_suite, "mutex_get_id");

    GET_MUTEX_ID("Mutex1", &mutex_id, &return_code);
    arinc_test_suite_check(&mutex_p1_suite, "get_id_valid_rc", arinc_test_check_int(NO_ERROR, return_code));
    arinc_test_suite_check(&mutex_p1_suite, "get_id_valid_id", arinc_test_check_int(1, mutex_id != NULL_MUTEX_ID));

    GET_MUTEX_ID("InvalidMutex", &invalid_mutex_id, &return_code);
    arinc_test_suite_check(&mutex_p1_suite, "get_id_invalid_rc", arinc_test_check_int(INVALID_CONFIG, return_code));

    arinc_test_suite_end(&mutex_p1_suite);
}

__attribute__((section(".p1_code")))
static void test_get_mutex_status_p1(MUTEX_ID_TYPE mutex_id)
{
    RETURN_CODE_TYPE return_code;
    MUTEX_STATUS_TYPE mutex_status;

    arinc_test_suite_begin(&mutex_p1_suite, "mutex_status");
    GET_MUTEX_STATUS(mutex_id, &mutex_status, &return_code);
    arinc_test_suite_check(&mutex_p1_suite, "status_rc", arinc_test_check_int(NO_ERROR, return_code));
    mutex_expect_status(&mutex_p1_suite,
                        "status_state",
                        "status_lock_count",
                        "status_waiting",
                        &mutex_status,
                        AVAILABLE,
                        0,
                        0);
    arinc_test_suite_end(&mutex_p1_suite);
}

__attribute__((section(".p1_code")))
static void test_acquire_mutex_p1(MUTEX_ID_TYPE mutex_id)
{
    RETURN_CODE_TYPE return_code;
    MUTEX_STATUS_TYPE status;

    arinc_test_suite_begin(&mutex_p1_suite, "mutex_acquire");

    /* first acquire should succeed */
    ACQUIRE_MUTEX(mutex_id, 0, &return_code);
    arinc_test_suite_check(&mutex_p1_suite, "acquire_first_rc", arinc_test_check_int(NO_ERROR, return_code));

    GET_MUTEX_STATUS(mutex_id, &status, &return_code);
    arinc_test_suite_check(&mutex_p1_suite, "status_after_first_acquire_rc", arinc_test_check_int(NO_ERROR, return_code));
    mutex_expect_status(&mutex_p1_suite,
                        "status_after_first_acquire_state",
                        "status_after_first_acquire_lock",
                        "status_after_first_acquire_waiting",
                        &status,
                        OWNED,
                        1,
                        0);

    /* recursive acquire by same process should increment lock count */
    ACQUIRE_MUTEX(mutex_id, 0, &return_code);
    arinc_test_suite_check(&mutex_p1_suite, "acquire_second_rc", arinc_test_check_int(NO_ERROR, return_code));

    GET_MUTEX_STATUS(mutex_id, &status, &return_code);
    arinc_test_suite_check(&mutex_p1_suite, "status_after_second_acquire_rc", arinc_test_check_int(NO_ERROR, return_code));
    mutex_expect_status(&mutex_p1_suite,
                        "status_after_second_acquire_state",
                        "status_after_second_acquire_lock",
                        "status_after_second_acquire_waiting",
                        &status,
                        OWNED,
                        2,
                        0);

    /* release once should decrement lock count */
    RELEASE_MUTEX(mutex_id, &return_code);
    arinc_test_suite_check(&mutex_p1_suite, "release_after_second_rc", arinc_test_check_int(NO_ERROR, return_code));

    GET_MUTEX_STATUS(mutex_id, &status, &return_code);
    arinc_test_suite_check(&mutex_p1_suite, "status_after_release_one_rc", arinc_test_check_int(NO_ERROR, return_code));
    mutex_expect_status(&mutex_p1_suite,
                        "status_after_release_one_state",
                        "status_after_release_one_lock",
                        "status_after_release_one_waiting",
                        &status,
                        OWNED,
                        1,
                        0);

    /* release final time to free mutex */
    RELEASE_MUTEX(mutex_id, &return_code);
    arinc_test_suite_check(&mutex_p1_suite, "release_final_rc", arinc_test_check_int(NO_ERROR, return_code));

    GET_MUTEX_STATUS(mutex_id, &status, &return_code);
    arinc_test_suite_check(&mutex_p1_suite, "status_after_final_release_rc", arinc_test_check_int(NO_ERROR, return_code));
    mutex_expect_status(&mutex_p1_suite,
                        "status_after_final_release_state",
                        "status_after_final_release_lock",
                        "status_after_final_release_waiting",
                        &status,
                        AVAILABLE,
                        0,
                        0);

    arinc_test_suite_end(&mutex_p1_suite);
}

__attribute__((section(".p1_code")))
static void test_release_mutex_p1(MUTEX_ID_TYPE mutex_id)
{
    RETURN_CODE_TYPE return_code;
    MUTEX_ID_TYPE got_mutex;

    arinc_test_suite_begin(&mutex_p1_suite, "mutex_release");

    /* releasing when not owned should return INVALID_MODE */
    RELEASE_MUTEX(mutex_id, &return_code);
    arinc_test_suite_check(&mutex_p1_suite, "release_not_owned_rc", arinc_test_check_int(INVALID_MODE, return_code));

    /* acquire then release */
    ACQUIRE_MUTEX(mutex_id, 0, &return_code);
    arinc_test_suite_check(&mutex_p1_suite, "acquire_rc", arinc_test_check_int(NO_ERROR, return_code));

    GET_PROCESS_MUTEX_STATE(MAIN_PROCESS_ID, &got_mutex, &return_code);
    arinc_test_suite_check(&mutex_p1_suite, "process_mutex_state_after_acquire_rc", arinc_test_check_int(NO_ERROR, return_code));
    arinc_test_suite_check(&mutex_p1_suite, "process_mutex_state_after_acquire_id", arinc_test_check_int(mutex_id, got_mutex));

    RELEASE_MUTEX(mutex_id, &return_code);
    arinc_test_suite_check(&mutex_p1_suite, "release_owned_rc", arinc_test_check_int(NO_ERROR, return_code));

    GET_MUTEX_STATUS(mutex_id, (MUTEX_STATUS_TYPE *)&(MUTEX_STATUS_TYPE){0}, &return_code);
    /* re-check available via GET_MUTEX_STATUS */
    MUTEX_STATUS_TYPE status;
    GET_MUTEX_STATUS(mutex_id, &status, &return_code);
    arinc_test_suite_check(&mutex_p1_suite, "status_after_release_rc", arinc_test_check_int(NO_ERROR, return_code));
    mutex_expect_status(&mutex_p1_suite,
                        "status_after_release_state",
                        "status_after_release_lock",
                        "status_after_release_waiting",
                        &status,
                        AVAILABLE,
                        0,
                        0);

    arinc_test_suite_end(&mutex_p1_suite);
}

__attribute__((section(".p1_code")))
static void test_reset_mutex_p1(MUTEX_ID_TYPE mutex_id)
{
    RETURN_CODE_TYPE return_code;
    MUTEX_STATUS_TYPE status;
    PROCESS_ID_TYPE my_id;

    arinc_test_suite_begin(&mutex_p1_suite, "mutex_reset");

    /* acquire then reset using our own process id */
    ACQUIRE_MUTEX(mutex_id, 0, &return_code);
    arinc_test_suite_check(&mutex_p1_suite, "acquire_for_reset_rc", arinc_test_check_int(NO_ERROR, return_code));

    GET_MY_ID(&my_id, &return_code);
    arinc_test_suite_check(&mutex_p1_suite, "get_my_id_rc", arinc_test_check_int(NO_ERROR, return_code));

    RESET_MUTEX(mutex_id, my_id, &return_code);
    arinc_test_suite_check(&mutex_p1_suite, "reset_rc", arinc_test_check_int(NO_ERROR, return_code));

    GET_MUTEX_STATUS(mutex_id, &status, &return_code);
    arinc_test_suite_check(&mutex_p1_suite, "status_after_reset_rc", arinc_test_check_int(NO_ERROR, return_code));
    mutex_expect_status(&mutex_p1_suite,
                        "status_after_reset_state",
                        "status_after_reset_lock",
                        "status_after_reset_waiting",
                        &status,
                        AVAILABLE,
                        0,
                        0);

    arinc_test_suite_end(&mutex_p1_suite);
}

__attribute__((section(".p1_code")))
void p1_process1(void)
{
    RETURN_CODE_TYPE return_code;
    MUTEX_ID_TYPE mutex_id;

    test_get_mutex_id_p1();

    GET_MUTEX_ID("Mutex1", &mutex_id, &return_code);
    if (return_code != NO_ERROR) {
        STOP_SELF();
    }

    test_get_mutex_status_p1(mutex_id);
    test_acquire_mutex_p1(mutex_id);
    test_release_mutex_p1(mutex_id);
    test_reset_mutex_p1(mutex_id);

    STOP_SELF();
}

__attribute__((section(".p1_code")))
void p1_process2(void)
{
    STOP_SELF();
}

__attribute__((section(".p2_code")))
void p2_process1(void)
{
    STOP_SELF();
}

__attribute__((section(".p2_code")))
void p2_process2(void)
{
    STOP_SELF();
}

int app_main(void)
{
    size_t p1_data_size = _p1_data_end - _p1_data_start;
    size_t p1_code_size = _p1_code_end - _p1_code_start;
    size_t p2_data_size = _p2_data_end - _p2_data_start;
    size_t p2_code_size = _p2_code_end - _p2_code_start;

    partition_init(DEFAULT_PARTITION_CONFIG.period,
                   DEFAULT_PARTITION_CONFIG.duration,
                   DEFAULT_PARTITION_CONFIG.identifier,
                   DEFAULT_PARTITION_CONFIG.num_assigned_cores,
                   DEFAULT_PARTITION_CONFIG.name,
                   DEFAULT_PARTITION_CONFIG.region_name_code_mem,
                   (void *)_p1_code_start,
                   (size_t)p1_code_size,
                   DEFAULT_PARTITION_CONFIG.access_code_mem,
                   DEFAULT_PARTITION_CONFIG.region_name_data_mem,
                   (void *)_p1_data_start,
                   p1_data_size,
                   DEFAULT_PARTITION_CONFIG.access_data_mem,
                   p1_main_process,
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
                   DEFAULT_PARTITION_CONFIG.max_errors);

    partition_init(P2_CONFIG.period,
                   P2_CONFIG.duration,
                   P2_CONFIG.identifier,
                   P2_CONFIG.num_assigned_cores,
                   P2_CONFIG.name,
                   P2_CONFIG.region_name_code_mem,
                   (void *)_p2_code_start,
                   (size_t)p2_code_size,
                   P2_CONFIG.access_code_mem,
                   P2_CONFIG.region_name_data_mem,
                   (void *)_p2_data_start,
                   p2_data_size,
                   P2_CONFIG.access_data_mem,
                   p2_main_process,
                   P2_CONFIG.is_system_partition,
                   P2_CONFIG.sampling_ports,
                   P2_CONFIG.max_sampling_ports,
                   P2_CONFIG.sampling_port_count,
                   P2_CONFIG.max_sampling_port_data_size,
                   P2_CONFIG.queuing_ports,
                   P2_CONFIG.max_queuing_ports,
                   P2_CONFIG.queuing_port_count,
                   P2_CONFIG.max_queuing_port_data_size,
                   P2_CONFIG.blackboards,
                   P2_CONFIG.max_blackboards,
                   P2_CONFIG.blackboard_count,
                   P2_CONFIG.max_blackboard_data_size,
                   P2_CONFIG.blackboards_data,
                   P2_CONFIG.blackboards_size_data,
                   P2_CONFIG.buffers,
                   P2_CONFIG.max_buffers,
                   P2_CONFIG.buffer_count,
                   P2_CONFIG.max_buffer_data_size,
                   P2_CONFIG.buffers_data,
                   P2_CONFIG.buffers_size_data,
                   P2_CONFIG.semaphores,
                   P2_CONFIG.max_semaphores,
                   P2_CONFIG.semaphore_count,
                   P2_CONFIG.semaphores_counter,
                   P2_CONFIG.events,
                   P2_CONFIG.max_events,
                   P2_CONFIG.event_count,
                   P2_CONFIG.mutexes,
                   P2_CONFIG.max_mutexes,
                   P2_CONFIG.mutex_count,
                   P2_CONFIG.error_list,
                   P2_CONFIG.error_list_cb,
                   P2_CONFIG.partition_hm_table,
                   P2_CONFIG.max_errors);

    return 1;
}
