.code64
.section .text
.globl isr_wrapper
.align 16

isr_wrapper: /*контроллер прерывания*/
    push %rax /*положили регистры на стек*/
    push %rcx
    push %rdx
    push %r8
    push %r9
    push %r10
    push %r11

    call interrupt_handler /*вызвали функцию обработки прерывания*/

    pop %r11 /*сняли регистры со стека*/
    pop %r10
    pop %r9
    pop %r8
    pop %rdx
    pop %rcx
    pop %rax


	movw $0x20, %dx /*послали подтверждение прерывания EOI*/
	movb $0x20, %al
	outb %al, %dx

	sti /*разрешили еще прерываться.*/
    iretq
