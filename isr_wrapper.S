.code64
.section .text
.globl isr_wrapper
.align 16

isr_wrapper:
    push %rax
    push %rcx
    push %rdx
    push %r8
    push %r9
    push %r10
    push %r11

    call interrupt_handler

    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rdx
    pop %rcx
    pop %rax


	movw $0x20, %dx
	movb $0x20, %al
	outb %al, %dx
	
	sti
    iretq
