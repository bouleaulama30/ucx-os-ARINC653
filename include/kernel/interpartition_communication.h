#include "../arinc/arinc_apex_types.h"
#include "../arinc/arinc_interpartition_communication.h"

#ifndef INTERPARTITION
#define INTERPARTITION

struct krnl_sampling_channel {
    char *name;
    uint8_t *buffer;
    MESSAGE_SIZE_TYPE max_message_size;
    MESSAGE_SIZE_TYPE current_message_size;
    SYSTEM_TIME_TYPE last_update_time;
};

#endif