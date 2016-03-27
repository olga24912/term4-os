.code64
.section .text
.globl start_thread
.globl switch_threads

start_thread:
        callq check_thread_finished
        callq end_critical_section   //section was open in yield

        popq %rax         // get thread function address
        popq %rdi         // get thread function args

        callq *%rax       // call real thread function

        movq %rax, %rdi
        callq finish_current_thread
        loop: jmp loop

// int switch_threads(void **old_sp, void *new_sp); // C signature return previous thread
switch_threads:
        pushq %rbp        // save volatile registers
        pushq %rbx
        pushq %r12
        pushq %r13
        pushq %r14
        pushq %r15

        movq %rsp, (%rdi) // save SP
        movq %rsi, %rsp   // restore SP

        popq %r15         // restore volatile register
        popq %r14
        popq %r13
        popq %r12
        popq %rbx
        popq %rbp

        ret