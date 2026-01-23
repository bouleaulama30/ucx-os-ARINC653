MEMORY
{
	ram (rwx)     : ORIGIN = 0x80000000, LENGTH = 16M

	/* définition de la p1 */
	p1 (rwx)  : ORIGIN = 0x81000000, LENGTH = 1M
}

/* Entry Point */
ENTRY(_entry)

/* Defines beginning and ending of stack. */
_stack_size  = 4M;
_stack = _stack_start;
_stack_start = ORIGIN(ram) + LENGTH(ram);
_stack_end   = _stack_start - _stack_size;

/* Defines beginning and ending of heap. */
_heap_start = _ebss;
_heap_end   = _stack_end;
_heap_size  = _stack_end - _ebss;

SECTIONS
{
	.text : {
		_text = .;
		*(.text.prologue);
		*(.text.init)
		*(.text)
		*(.text.*)
		_etext = .;
	} > ram
	
	.rodata : {
		. = ALIGN(4);
		_rodata = .;
		*(.rdata)
		*(.rodata)
		*(.rodata*)
		. = ALIGN(4);
		_erodata = .;
	} > ram

	.data : {
		. = ALIGN(4);
		_data = .;
		*(.data)
		*(.data*)
		_gp = . + 0x800;
		*(.sdata)
		*(.sdata.*)
		. = ALIGN(4);
		_edata = .;
	} > ram
	
	.bss : {
		. = ALIGN(4);
		_sbss = .;
		*(.sbss)
		*(.sbss.*)
		*(.bss)
		*(.bss*)
		*(COMMON)
		. = ALIGN(4);
		_ebss = .;
	} > ram
	
	_end = .;

    .p1_code : {
        _p1_code_start = .;  

        *(.p1_code)

    } > p1

	.p1_stack : {
        _p1_stack_end = .;
        . += 256K;
        _p1_stack_start = .;
    } > p1

    _p1_code_size = . - _p1_code_start; 
}
