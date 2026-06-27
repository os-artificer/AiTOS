ENTRY(_start)

KERNEL_VMA = 0x70000;
KERNEL_PHYS = 0x70000;

PHDRS
{
	text PT_LOAD FLAGS(5);	/* PF_R | PF_X */
	data PT_LOAD FLAGS(6);	/* PF_R | PF_W */
}

SECTIONS
{
	. = KERNEL_VMA;

	.text : AT(KERNEL_PHYS) {
		*(.head.text)
		*(.text .text.*)
		*(.init.text)
	} :text

	.rodata : AT(KERNEL_PHYS + (ADDR(.rodata) - KERNEL_VMA)) {
		*(.rodata .rodata.*)
	} :text

	.data : AT(KERNEL_PHYS + (ADDR(.data) - KERNEL_VMA)) {
		*(.data .data.*)
		*(.init.data)
	} :data

	.bss : AT(KERNEL_PHYS + (ADDR(.bss) - KERNEL_VMA)) {
		__bss_start = .;
		*(.bss .bss.* COMMON)
		__bss_end = .;
	} :data

	/DISCARD/ : {
		*(.note.*)
		*(.eh_frame)
	}
}
