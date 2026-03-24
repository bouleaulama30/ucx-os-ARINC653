
/*----------------------------------------------------------------*/
/*                                                                */
/* TIME constant and type definitions and management services     */
/*                                                                */
/*----------------------------------------------------------------*/

#include "arinc_apex_types.h"

#ifndef APEX_TIME
#define APEX_TIME

#define MAX_SYSTEM_TIME 0x7fffffffffffffffULL

static inline BOOLEAN_TYPE time_overflow(uint64_t time){
    if(time > MAX_SYSTEM_TIME)
        return true;
    return false;
}


extern void TIMED_WAIT (
       /*in */ SYSTEM_TIME_TYPE         DELAY_TIME,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void PERIODIC_WAIT (
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void GET_TIME (
       /*out*/ SYSTEM_TIME_TYPE         *SYSTEM_TIME,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

extern void REPLENISH (
       /*in */ SYSTEM_TIME_TYPE         BUDGET_TIME,
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE );

SYSTEM_TIME_TYPE  arinc_time_find_first_release_point(struct pcb_s *current_partition);
void arinc_time_update_partition(struct pcb_s *partition);
#endif
