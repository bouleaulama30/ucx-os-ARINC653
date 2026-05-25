ENTRY(_entry)

MEMORY
{
    /* Keep this within the DDR example window already validated in your setup */
    ddr (rwx) : ORIGIN = 0x80000000, LENGTH = 0x00100000

    /* définition de la p1 */
	p1_code (rx)  : ORIGIN = 0x81000000, LENGTH = 1M
	p1_data (rw)  : ORIGIN = 0x82000000, LENGTH = 64K
	
	/* définition de la p2 */
	p2_code (rx)  : ORIGIN = 0x83000000, LENGTH = 1M
	p2_data (rw)  : ORIGIN = 0x84000000, LENGTH = 64K
}

/* Initial boot stack reserved at top of DDR */
STACK_SIZE = 0x00010000;   /* 64 KiB */

_stack_start = ORIGIN(ddr) + LENGTH(ddr);
_stack_end   = _stack_start - STACK_SIZE;
_stack       = _stack_start;

SECTIONS
{
    . = ORIGIN(ddr);

    .text :
    {
        . = ALIGN(8);
        _text = .;
        KEEP(*(.text.prologue))
        KEEP(*(.init))
        *(.text.init)
        *(.text)
        *(.text.*)
        . = ALIGN(8);
        _etext = .;
    } > ddr

    .rodata :
    {
        . = ALIGN(8);
        _rodata = .;
        *(.rdata)
        *(.rodata)
        *(.rodata.*)
        *(.srodata)
        *(.srodata.*)
        . = ALIGN(8);
        _erodata = .;
    } > ddr

    .data :
    {
        . = ALIGN(8);
        _sdata = .;
        *(.data)
        *(.data.*)

        /* Small data base, matching UCX RV64 style */
        _gp = . + 0x800;

        *(.sdata)
        *(.sdata.*)
        . = ALIGN(8);
        _edata = .;
    } > ddr

    /* Debug-loaded image: load address equals run address */
    _sidata = LOADADDR(.data);

    .bss (NOLOAD) :
    {
        . = ALIGN(8);
        _sbss = .;
        *(.sbss)
        *(.sbss.*)
        *(.bss)
        *(.bss.*)
        *(COMMON)
        . = ALIGN(8);
        _ebss = .;
    } > ddr

    _end = .;

    _kernel_end = ORIGIN(ddr) + LENGTH(ddr);

    .p1_code : {
        _p1_code_start = .;  

        *(.p1_code)
		_p1_code_end = .;
    } > p1_code

	.p1_data : {
        _p1_data_start = .;
    } > p1_data
	
    _p1_data_end = ORIGIN(p1_data) + LENGTH(p1_data) ;

	.p2_code : {
        _p2_code_start = .;  

        *(.p2_code)
		_p2_code_end = .;
    } > p2_code

	.p2_data : {
        _p2_data_start = .;
    } > p2_data

    _p2_data_end = ORIGIN(p2_data) + LENGTH(p2_data) ;


    /* Optional aliases used by some UCX code paths */
    __bss_end = _ebss;
    __stack   = _stack;

    /* Heap directly below reserved boot stack */
    _heap_start = _ebss;
    _heap_end   = _stack_end;
    _heap_size  = _heap_end - _heap_start;

    /DISCARD/ :
    {
        *(.comment*)
        *(.note*)
        *(.eh_frame*)
        *(.eh_frame_hdr*)
    }
}