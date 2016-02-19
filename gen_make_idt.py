#!/usr/bin/python3
print("/*this file is automatically generated by gen_make_idt.py*/")
print("#pragma once")
print('#include "interrupt.h"')
print()
for i in range(256):
    print("extern void* _isr" + str(i) + ";")
print()
print("void make_idt(struct idt_entry* idt) {")
for i in range(256): #заполняем таблицу
    print("    make_idt_entry(idt + " + str(i) +", &_isr" + str(i) + ");")
print("}")
