; Copyright 2026 AiTOS authors.
; SPDX-License-Identifier: Apache-2.0
;
; Kernel architecture entry: BSS zeroing, call to kmain, and IDT stub handlers
; for vectors 0x00–0x2f. Each stub saves registers, calls irq_dispatch(vector),
; restores state, and returns via iretq.

[bits 64]		; long mode: 64-bit instructions throughout

extern irq_dispatch	; C handler: void irq_dispatch(unsigned vector)

%define BOOT_SOURCE_LEGACY	0
%define BOOT_SOURCE_MULTIBOOT2	1
%define MULTIBOOT2_BOOTLOADER_MAGIC 0x36d76289
%define KERNEL_BIN_BASE_ADDR        0x100000
%define EARLY_STACK_TOP             0x7fe000

; --- Save all general-purpose registers on the interrupt stack ---
; Pushes in reverse order so POP_REGS restores correctly. Saves full GPR set
; so irq_dispatch and callees see an intact CPU state on return.
%macro PUSH_REGS 0
    push r15			; save R15
    push r14			; save R14
    push r13			; save R13
    push r12			; save R12
    push r11			; save R11
    push r10			; save R10
    push r9			; save R9
    push r8			; save R8
    push rbp			; save frame pointer
    push rbx			; save RBX (callee-saved in SysV ABI)
    push rdx			; save RDX
    push rcx			; save RCX
    push rax			; save RAX
%endmacro

; --- Restore GPRs in reverse push order ---
%macro POP_REGS 0
    pop rax			; restore RAX
    pop rcx			; restore RCX
    pop rdx			; restore RDX
    pop rbx			; restore RBX
    pop rbp			; restore RBP
    pop r8			; restore R8
    pop r9			; restore R9
    pop r10			; restore R10
    pop r11			; restore R11
    pop r12			; restore R12
    pop r13			; restore R13
    pop r14			; restore R14
    pop r15			; restore R15
%endmacro

; --- ISR stub for exceptions/interrupts without CPU-pushed error code ---
; Pushes dummy error code 0 so stack layout matches error-code vectors.
%macro ISR_NOERR 1
global intr%1_entry
intr%1_entry:
    push 0			; dummy error code (CPU did not push one)
    PUSH_REGS			; save all GPRs
    mov rdi, %1			; RDI = vector number (SysV first arg)
    sub rsp, 8			; align stack to 16 bytes before call
    call irq_dispatch		; dispatch to C IRQ layer
    add rsp, 8			; undo alignment padding
    POP_REGS			; restore GPRs
    add rsp, 8			; drop dummy error code
    iretq			; return from interrupt (restores RIP, CS, RFLAGS)
%endmacro

; --- ISR stub for exceptions that push an error code on the stack ---
; CPU already pushed error code; do not push dummy.
%macro ISR_ERR 1
global intr%1_entry
intr%1_entry:
    PUSH_REGS			; save all GPRs (error code remains below frame)
    mov rdi, %1			; RDI = vector number
    sub rsp, 8			; 16-byte stack alignment for call
    call irq_dispatch		; C handler may inspect error code on stack
    add rsp, 8			; undo alignment padding
    POP_REGS			; restore GPRs
    add rsp, 8			; drop CPU-pushed error code
    iretq			; return to interrupted context
%endmacro

section .data
global boot_code_selector
boot_code_selector:
    dw 0				; filled from CS at entry (GRUB=0x10, legacy=0x18)

section .data
align 8
global boot_handoff_source
global boot_handoff_mbi_phys
boot_handoff_source:
    dq 0
boot_handoff_mbi_phys:
    dq 0
align 16
global boot_stack_top
boot_stack:
    times 16384 db 0
boot_stack_top:

section .head.text	; early kernel text linked at high-half entry
global _start
extern kmain		; kernel C entry after boot
extern boot_early_init	; populate boot_info from legacy or Multiboot2 handoff
extern __bss_start	; start of uninitialized data (linker symbol)
extern __bss_end	; end of BSS (exclusive)

; --- Kernel entry: set stack, save handoff, zero BSS, boot_early_init, kmain ---
_start:
    cli				; interrupts off until IDT is installed in C
    mov r13, rbx		; GRUB → EBX = MBI phys (save before any RBX clobber)
    cmp eax, MULTIBOOT2_BOOTLOADER_MAGIC
    je .mb2_handoff
    cmp eax, MULTIBOOT2_BOOTLOADER_MAGIC - 1
    je .mb2_handoff
    cmp eax, KERNEL_BIN_BASE_ADDR
    je .legacy_handoff
    jmp .legacy_handoff
.legacy_handoff:
    mov r12, BOOT_SOURCE_LEGACY
    jmp .check_mbi
.mb2_handoff:
    mov r12, BOOT_SOURCE_MULTIBOOT2
.check_mbi:
    test r13, r13
    jz .save_handoff
    cmp r13d, 0x10000
    jb .save_handoff
    cmp r13d, 0x08000000
    ja .save_handoff
    mov r12, BOOT_SOURCE_MULTIBOOT2
.save_handoff:
    mov ax, cs
    mov word [boot_code_selector], ax
    mov qword [boot_handoff_source], r12
    mov qword [boot_handoff_mbi_phys], r13
.bss_clear:
    mov rdi, __bss_start	; RDI = destination for BSS zero
    mov rcx, __bss_end		; RCX = end address
    sub rcx, rdi		; RCX = byte count to clear
    xor eax, eax		; fill value 0
    cld				; rep stosb increments RDI
    rep stosb			; zero entire BSS range
    mov rsp, boot_stack_top
    and rsp, ~0xf		; 16-byte align RSP for C calls
    mov rdi, r12
    mov rsi, r13
    call boot_early_init
    call kmain
.start_hang:
    hlt				; halt CPU if kmain returns
    jmp .start_hang		; loop forever on resume from halt

; --- CPU exception and IRQ vectors 0x00–0x2f (IDT stub generation) ---
ISR_NOERR 0x00		; divide error
ISR_NOERR 0x01		; debug exception
ISR_NOERR 0x02		; non-maskable interrupt
ISR_NOERR 0x03		; breakpoint
ISR_NOERR 0x04		; overflow
ISR_NOERR 0x05		; bound range exceeded
ISR_NOERR 0x06		; invalid opcode
ISR_NOERR 0x07		; device not available
ISR_ERR   0x08		; double fault (error code)
ISR_NOERR 0x09		; coprocessor segment overrun
ISR_ERR   0x0a		; invalid TSS (error code)
ISR_ERR   0x0b		; segment not present (error code)
ISR_ERR   0x0c		; stack-segment fault (error code)
ISR_ERR   0x0d		; general protection fault (error code)
ISR_ERR   0x0e		; page fault (error code)
ISR_NOERR 0x0f		; reserved
ISR_NOERR 0x10		; x87 floating-point exception
ISR_ERR   0x11		; alignment check (error code)
ISR_NOERR 0x12		; machine check
ISR_NOERR 0x13		; SIMD floating-point exception
ISR_NOERR 0x14		; virtualization exception
ISR_NOERR 0x15		; reserved
ISR_NOERR 0x16		; reserved
ISR_NOERR 0x17		; reserved
ISR_NOERR 0x18		; reserved
ISR_NOERR 0x19		; reserved
ISR_NOERR 0x1a		; reserved
ISR_NOERR 0x1b		; reserved
ISR_NOERR 0x1c		; reserved
ISR_NOERR 0x1d		; reserved
ISR_ERR   0x1e		; reserved (error code on some CPUs)
ISR_NOERR 0x1f		; reserved
ISR_NOERR 0x20		; IRQ0: timer (PIC master)
ISR_NOERR 0x21		; IRQ1: keyboard
ISR_NOERR 0x22		; IRQ2: cascade
ISR_NOERR 0x23		; IRQ3: COM2
ISR_NOERR 0x24		; IRQ4: COM1
ISR_NOERR 0x25		; IRQ5: LPT2
ISR_NOERR 0x26		; IRQ6: floppy
ISR_NOERR 0x27		; IRQ7: LPT1
ISR_NOERR 0x28		; IRQ8: RTC
ISR_NOERR 0x29		; IRQ9: free
ISR_NOERR 0x2a		; IRQ10: free
ISR_NOERR 0x2b		; IRQ11: free
ISR_NOERR 0x2c		; IRQ12: PS/2 mouse
ISR_NOERR 0x2d		; IRQ13: FPU
ISR_NOERR 0x2e		; IRQ14: primary ATA
ISR_NOERR 0x2f		; IRQ15: secondary ATA

section .rodata		; read-only handler pointer table for IDT setup
global intr_entry_table
intr_entry_table:
    dq intr0x00_entry		; vector 0x00 handler address
    dq intr0x01_entry		; vector 0x01 handler address
    dq intr0x02_entry		; vector 0x02 handler address
    dq intr0x03_entry		; vector 0x03 handler address
    dq intr0x04_entry		; vector 0x04 handler address
    dq intr0x05_entry		; vector 0x05 handler address
    dq intr0x06_entry		; vector 0x06 handler address
    dq intr0x07_entry		; vector 0x07 handler address
    dq intr0x08_entry		; vector 0x08 handler address
    dq intr0x09_entry		; vector 0x09 handler address
    dq intr0x0a_entry		; vector 0x0a handler address
    dq intr0x0b_entry		; vector 0x0b handler address
    dq intr0x0c_entry		; vector 0x0c handler address
    dq intr0x0d_entry		; vector 0x0d handler address
    dq intr0x0e_entry		; vector 0x0e handler address
    dq intr0x0f_entry		; vector 0x0f handler address
    dq intr0x10_entry		; vector 0x10 handler address
    dq intr0x11_entry		; vector 0x11 handler address
    dq intr0x12_entry		; vector 0x12 handler address
    dq intr0x13_entry		; vector 0x13 handler address
    dq intr0x14_entry		; vector 0x14 handler address
    dq intr0x15_entry		; vector 0x15 handler address
    dq intr0x16_entry		; vector 0x16 handler address
    dq intr0x17_entry		; vector 0x17 handler address
    dq intr0x18_entry		; vector 0x18 handler address
    dq intr0x19_entry		; vector 0x19 handler address
    dq intr0x1a_entry		; vector 0x1a handler address
    dq intr0x1b_entry		; vector 0x1b handler address
    dq intr0x1c_entry		; vector 0x1c handler address
    dq intr0x1d_entry		; vector 0x1d handler address
    dq intr0x1e_entry		; vector 0x1e handler address
    dq intr0x1f_entry		; vector 0x1f handler address
    dq intr0x20_entry		; vector 0x20 handler address
    dq intr0x21_entry		; vector 0x21 handler address
    dq intr0x22_entry		; vector 0x22 handler address
    dq intr0x23_entry		; vector 0x23 handler address
    dq intr0x24_entry		; vector 0x24 handler address
    dq intr0x25_entry		; vector 0x25 handler address
    dq intr0x26_entry		; vector 0x26 handler address
    dq intr0x27_entry		; vector 0x27 handler address
    dq intr0x28_entry		; vector 0x28 handler address
    dq intr0x29_entry		; vector 0x29 handler address
    dq intr0x2a_entry		; vector 0x2a handler address
    dq intr0x2b_entry		; vector 0x2b handler address
    dq intr0x2c_entry		; vector 0x2c handler address
    dq intr0x2d_entry		; vector 0x2d handler address
    dq intr0x2e_entry		; vector 0x2e handler address
    dq intr0x2f_entry		; vector 0x2f handler address
