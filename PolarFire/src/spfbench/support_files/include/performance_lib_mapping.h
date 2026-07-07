/* __________________________________________________________________________
 * DEVELOPMENT HISTORY:
 * --------------------
 *
 * $Author:: $: Felipe Magalhaes
 * $Rev:: $: Revision of last commit
 * $Date:: $: 11/12/2018
 * $Comments:: $: Initial version of the file.
 *  __________________________________________________________________________
 */
/* __________________________________________________________________________
 * MODULE DESCRIPTION:
 * -------------------
 * Filename : performance_lib_mapping.h
 * Original Author : Felipe Magalhaes
 *
 * TODO
 *
 * ASSUMPTIONS, CONSTRAINTS OR LIMITATIONS:
 * ----------------------------------------
 * None.
 *
 * REFERENCES:
 * -----------
 * None.
 * __________________________________________________________________________
 */
/*INCLUDE LIBS*/
/*This should be adapted to reflect target OS*/

#ifndef UCX_H 
#define UCX_H
#include <ucx.h>
#endif


/*PLATFORM DEPENDENT DEFINITIONS*/
/*These are the definitions and declarations specific for the target OS*/
#if defined(__riscv) && (__riscv_xlen == 64)
#define CLOCK_FREQ 600000000UL                       // CLINT timer on PolarFire runs at 1 MHz
#else
#define CLOCK_FREQ 10000000    					//the system clock can be hardcoded or rely on a function (GET_CLOCK_FREQUENCY(), for instance)
#endif

//functions definitions
void init_tick_counter();                          //function to initialize system timer
uint64_t PerfGetTimeTicks(void);                   //function to read system timer

#define START_TICK_COUNTER() init_tick_counter();   //mapping for the function
#define GET_CURRENT_TICKS() PerfGetTimeTicks()  	//mapping for the function

//base print function (i.e. putchar(c))
#ifdef PERFORMANCE_PRINT
  #define PRINT_OUTPUT_CHAR(x) printf("%c", x)
#else
  #define PRINT_OUTPUT_CHAR(x) (void)x
#endif 

/* __________________________________________________________________________
* END OF FILE:
* -------------
* ___________________________________________________________________________
*/
