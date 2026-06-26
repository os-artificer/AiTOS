ENTRY(_start)

KERNEL_VMA = 0x70000;
KERNEL_PHYS = 0x70000;

SECTIONS
{
	. = KERNEL_VMA;

	.text : AT(KERNEL_PHYS) {
		*(.head.text)
		*(.text .text.*)
	}

	.rodata : AT(KERNEL_PHYS + (ADDR(.rodata) - KERNEL_VMA)) {
		*(.rodata .rodata.*)
	}

	.data : AT(KERNEL_PHYS + (ADDR(.data) - KERNEL_VMA)) {
		*(.data .data.*)
	}

	.bss : AT(KERNEL_PHYS + (ADDR(.bss) - KERNEL_VMA)) {
		__bss_start = .;
		*(.bss .bss.* COMMON)
		__bss_end = .;
	}

	/DISCARD/ : {
		*(.note.*)
		*(.eh_frame)
	}
}
