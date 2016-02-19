#!/usr/bin/python3
print("/*this file is automatically generated by gen_isr_wrapper.py*/")
print(".code64")
print(".section .text")
for i in range(0, 256):
    print(".globl _isr" + str(i))
print(".align 16")
print()
print()
print()


have_error = [8, 10, 11, 12, 13, 14] #intel 6.15 interrupt with exseption error code

for i in range(0, 256):
    print("_isr" + str(i) + ":")
    if i not in have_error:
        print("    push $0")
    print("    push $" + str(i))
    print("    jmp isr_wrapper")
    print()

#        If the interrupt handler code is simply a stub that forwards to C code,
#        you don't need to save all of the registers.
#        You can assume that the C compiler will generate code that
#        will be preserving rbx, rbp, rsi, rdi, and r12 thru r15.
#        You should only need to save and restore rax, rcx, rdx, and r8 thru r11
#
#        http://stackoverflow.com/questions/6837392/how-to-save-the-registers-on-x86-64-for-an-interrupt-service-routine*/

reg = ["rax", "rcx", "rdx", "r8", "r9", "r10", "r11"]

#interrupt controler
print("isr_wrapper:")
for cur_reg in reg[::-1]:
    print("    push %" + cur_reg) #push registers on stack

print()
print("    call interrupt_handler")
print()

for cur_reg in reg:
    print("    pop %" + cur_reg) #pop registers on stack

print()
print("    add $16, %rsp") #pop from stack error_code and id_interrupt
print("    iretq")