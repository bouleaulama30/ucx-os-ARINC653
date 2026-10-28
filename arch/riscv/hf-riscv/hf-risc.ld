MEMORY
{
	ram (rwx)     : ORIGIN = 0x40000000, LENGTH = 256K
}

/* Entry Point */
ENTRY(_entry)

/* Defines beginning and ending of stack. */
_stack_size  = 4K;
_stack = _stack_start;
_stack_start = ORIGIN(ram) + LENGTH(ram);
_stack_end   = _stack_start - _stack_size;

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

	.p1_code : {
		. = ALIGN(4);
		_p1_code_start = .;  
		*(.p1_code)
		*(.p1_code.*)
		. = ALIGN(4);
		_p1_code_end = .;
	} > ram

	.p2_code : {
		. = ALIGN(4);
		_p2_code_start = .;  
		*(.p2_code)
		*(.p2_code.*)
		. = ALIGN(4);
		_p2_code_end = .;
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
	
	.bss (NOLOAD) : {
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

	.p1_data (NOLOAD) : {
		. = ALIGN(4);
		_p1_data_start = .;
		*(.p1_data)
		*(.p1_data.*)
		. += 16K;
		. = ALIGN(4);
		_p1_data_end = .;
	} > ram

	.p2_data (NOLOAD) : {
		. = ALIGN(4);
		_p2_data_start = .;
		*(.p2_data)
		*(.p2_data.*)
		. += 8K;
		. = ALIGN(4);
		_p2_data_end = .;
	} > ram

	_end = .;
	_kernel_end = _p1_code_start;

	/* Defines beginning and ending of heap. */
	_heap_start = _end;
	_heap_end   = _stack_end;
	_heap_size  = _stack_end - _end;
}
