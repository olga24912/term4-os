#include <limits.h>
#include "ioport.h"
#include "uart.h"
#include "stdarg.h"
#include "io.h"
#include "interrupt.h"
#include "memory.h"

#define CRM (0x20)
#define CRS (0xA0)
#define DRM (0x21)
#define DRS (0xA1)

void make_idt_entry (struct idt_entry* entry, void* handler) {
    entry->reserved = 0;
    entry->offset0 = (((uint64_t)handler) & 0xFFFF);
    entry->offset1 = (((uint64_t)handler) >> 16) & 0xFFFF;
    entry->offset2 = (((uint64_t)handler) >> 32) & 0xFFFFFFFF;
    entry->segment_selector = KERNEL_CODE;
    entry->flags = 0x8E00;
}

void init_lpic_slave() {
    out8(CRS, (1 << 4)|1);
    out8(DRS, 0x28);
    out8(DRS, (1 << 1));
    out8(DRS, 1);
}

void init_lpic_master() {
    out8(CRM, (1 << 4)|1);
    out8(DRM, 0x20);
    out8(DRM, (1 << 2));
    out8(DRM, 1);
}

void init_lpic() {
    init_lpic_master();
    init_lpic_slave();
    out8(DRM, 0xfe);
    out8(DRS, 0xff);
}


struct idt_entry idt[256];

extern void* isr_wrapper;

void init_interrupt() {
    init_lpic();
    for (int i = 0; i < 256; ++i) {
        make_idt_entry(idt + i, isr_wrapper);
    }
    static struct idt_ptr ptr;
    ptr.base = (uint64_t) idt;
    ptr.size = sizeof(idt) - 1;

    set_idt(&ptr);
}

void interrupt_handler(void) {
    puts("timer interrupt");
}