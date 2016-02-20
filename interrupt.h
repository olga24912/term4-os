#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include <stdint.h>
#include "ioport.h"
struct idt_ptr {
	uint16_t size;
	uint64_t base;
} __attribute__((packed));

struct idt_entry { //дискриптор обработки прерывания.
	uint16_t offset0;
    uint16_t segment_selector;
    uint16_t flags;
    uint16_t offset1;
    uint32_t offset2;
    uint32_t reserved;
} __attribute__((packed));

struct interrupt_handler_args {
	uint64_t rax;
	uint64_t rcx;
	uint64_t rdx;
	uint64_t rsi;
	uint64_t rdi;
	uint64_t r8;
	uint64_t r9;
	uint64_t r10;
	uint64_t r11;
	uint64_t interrupt_id;
	uint64_t error_code;
};

static inline void set_idt(const struct idt_ptr *ptr) {
    __asm__ volatile ("lidt (%0)" : : "a"(ptr));
}

static inline void interrupt_off() {
	__asm__ volatile ("cli"); // запрещаем прерывание
}

static inline void interrupt_on() {
	__asm__ volatile ("sti"); //разрешаем прерываться
}

static inline void send_EOI() {
	/*послали подтверждение прерывания EOI*/
	out8(0x20, 0x20);
}

void init_interrupt();

#endif /*__INTERRUPT_H__*/
