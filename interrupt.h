#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include <stdint.h>

struct idt_ptr {
	uint16_t size;
	uint64_t base;
} __attribute__((packed));

struct idt_entry {
	uint16_t offset0;
    uint16_t segment_selector;
    uint16_t flags;
    uint16_t offset1;
    uint32_t offset2;
    uint32_t reserved;
} __attribute__((packed));

static inline void set_idt(const struct idt_ptr *ptr) {
    __asm__ volatile ("lidt (%0)" : : "a"(ptr));
}

void init_interrupt();

#endif /*__INTERRUPT_H__*/
