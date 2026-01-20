/*  This is a C language specification of the APEX interface,        	    */
/*  derived from Section 3 of ARINC 653 Part 1.                             */

/*  The declarations of the services given below are taken from the         */
/*  standard, as are the enum types and the names of the others types.      */
/*  Unless specified as implementation dependent, the values specified in   */
/*  this appendix should be implemented as defined.                         */

/*  This C language specification follows the same structure (package) as   */
/*  the Ada specification and the parameters within a structure are defined */
/*  in the same order. The objective is to have the same definitions        */
/*  and representations of the interface in both the Ada and C languages.   */
/*  The two specifications are aligned in the same order, and define the    */
/*  same items.                                                             */

/* Parameter ordering within a struct as well as ID types and sizes are     */
/* implementation dependent. For example, an ID returned by the O/S may be  */
/* based on an index or may be based on a reference to a data structure.    */
/* The APEX_NATIVE_INTEGER and APEX_NATIVE_UNSIGNED types are intended      */
/* to support references to a data structure that scale with the            */
/* processor's native address size.                                         */


/* The APEX specifications will be available via "ARINC653.h" header file.  */


/*----------------------------------------------------------------*/
/*                                                                */
/* Global constant and type definitions                           */
/*                                                                */
/*----------------------------------------------------------------*/

#ifndef APEX_TYPES
#define APEX_TYPES

/*---------------------------*/
/* Domain limits             */
/*---------------------------*/

/*                       Implementation Dependent                          */
/* These values define the domain limits and are implementation dependent. */
/* These values represent typical minimum values supported by an O/S,      */
/* unless otherwise limited (e.g., message size limited by transport).     */
/* For partition scope, the limits are per partition.                      */

#define  SYSTEM_LIMIT_NUMBER_OF_PARTITIONS      32      /* module scope */

#define  SYSTEM_LIMIT_NUMBER_OF_MESSAGES        512     /* module scope */

#define  SYSTEM_LIMIT_MESSAGE_SIZE              8192    /* module scope */

#define  SYSTEM_LIMIT_NUMBER_OF_PROCESSES       128     /* partition scope */

#define  SYSTEM_LIMIT_NUMBER_OF_SAMPLING_PORTS  512     /* partition scope */

#define  SYSTEM_LIMIT_NUMBER_OF_QUEUING_PORTS   512     /* partition scope */

#define  SYSTEM_LIMIT_NUMBER_OF_BUFFERS         256     /* partition scope */

#define  SYSTEM_LIMIT_NUMBER_OF_BLACKBOARDS     256     /* partition scope */

#define  SYSTEM_LIMIT_NUMBER_OF_SEMAPHORES      256     /* partition scope */

#define  SYSTEM_LIMIT_NUMBER_OF_EVENTS          256     /* partition scope */

#define  SYSTEM_LIMIT_NUMBER_OF_MUTEXES         256     /* partition scope */

/*----------------------*/
/* Base APEX types      */
/*----------------------*/

/*                       Implementation Portable                         */
/*  The sizes of these base types are identical for all systems and the  */
/*  actual types must match the types used by the implementation of the  */
/*  underlying Operating System to result in the defined sizes.          */

typedef  unsigned char   APEX_BYTE;             /* 8-bit unsigned  */

typedef  int             APEX_INTEGER;          /* 32-bit signed   */

typedef  unsigned        APEX_UNSIGNED;         /* 32-bit unsigned */

typedef  long long     	 APEX_LONG_INTEGER;     /* 64-bit signed   */


/*  The following types are either 32-bit or 64-bit and will match */ 
/*  the processor's native address size.                           */
typedef  long            APEX_NATIVE_INTEGER; 
typedef  unsigned long   APEX_NATIVE_UNSIGNED; 


/*----------------------*/
/* General APEX types   */
/*----------------------*/

typedef
   enum {
        NO_ERROR       = 0,   /* request valid and operation performed      */
        NO_ACTION      = 1,   /* status of system unaffected by request     */
        NOT_AVAILABLE  = 2,   /* resource required by request unavailable   */
        INVALID_PARAM  = 3,   /* invalid parameter specified in request     */
        INVALID_CONFIG = 4,   /* parameter incompatible with configuration  */
        INVALID_MODE   = 5,   /* request incompatible with current mode     */
        TIMED_OUT      = 6    /* time-out tied up with request has expired  */
   } RETURN_CODE_TYPE;

#define  MAX_NAME_LENGTH          32

typedef  char            NAME_TYPE[MAX_NAME_LENGTH];

/* the size of address pointers must match the address range supported by   */
/* the processor (e.g., 32-bits or 64-bits as supported or as configured    */
/* for a processor).                                                        */
typedef  void            (* SYSTEM_ADDRESS_TYPE);

typedef  APEX_BYTE *     MESSAGE_ADDR_TYPE;

typedef  APEX_INTEGER    MESSAGE_SIZE_TYPE;

typedef  APEX_INTEGER    MESSAGE_RANGE_TYPE;

typedef  enum { SOURCE = 0, DESTINATION = 1 } PORT_DIRECTION_TYPE;

typedef  enum { FIFO = 0, PRIORITY = 1 } QUEUING_DISCIPLINE_TYPE;

typedef  APEX_LONG_INTEGER   SYSTEM_TIME_TYPE;
                           /* 64-bit signed integer with a 1 nanosecond LSB */

#define  INFINITE_TIME_VALUE    -1

typedef  APEX_INTEGER           PROCESSOR_CORE_ID_TYPE; 
#define  CORE_AFFINITY_NO_PREFERENCE       <implementation dependent> 

#endif