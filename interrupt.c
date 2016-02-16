#include <limits.h>
#include "ioport.h"
#include "uart.h"
#include "stdarg.h"
#include "io.h"
#include "interrupt.h"
#include "memory.h"

#define CRM (0x20) //Command Register and Status Register Master
#define CRS (0xA0) // Command Register and Status Register Slave
#define DRM (0x21) // Interrupt Maske Register and Date Register Master
#define DRS (0xA1) // Interrupt Maske Register and Date Register Slave

void make_idt_entry (struct idt_entry* entry, void* handler) { // инициализатор дискриптора прерывания.
    entry->reserved = 0;
    entry->offset0 = (((uint64_t) handler) & 0xFFFF);   // записываем адрес контроллера прерывания
    entry->offset1 = ((((uint64_t) handler) >> 16) & 0xFFFF);
    entry->offset2 = (((uint64_t) handler) >> 32) & 0xFFFFFFFF;
    entry->segment_selector = KERNEL_CODE;
    entry->flags = 0x8E00; //выставляем тип interrupt gate
}

void init_lpic_slave() { // инициализация раба
    out8(CRS, (1 << 4)|1); // следующие три слова часть этой команды + 4 бит команда инициализации контролерра.
    out8(DRS, 0x28); // устанавливаем ICW
    out8(DRS, (1 << 1)); // номер входа, к которому подключен раб
    out8(DRS, 1); // устанавливаем режим контроллера прерывания.
}

void init_lpic_master() { //инициализация мастера, там все примерно так же.
    out8(CRM, (1 << 4)|1);
    out8(DRM, 0x20);
    out8(DRM, (1 << 2));
    out8(DRM, 1);
}

void init_lpic() { // инициализация Legacy PIC
    init_lpic_master(); // инициализация мастера
    init_lpic_slave();  // инициализация раба
    out8(DRM, 0xfe); // выставляем маску для прирываний, все не нужное ингорируем.
    out8(DRS, 0xff);
}

struct idt_entry idt[256];

extern void* isr_wrapper;

void init_interrupt() { // инициализация прерывания.
    init_lpic(); //  инициализируем Legacy PIC
    for (int i = 0; i < 256; ++i) { // заполняем таблицу
        make_idt_entry(idt + i, &isr_wrapper);
    }
    static struct idt_ptr ptr; // записываем указатель на таблицу
    ptr.base = (uint64_t) idt;
    ptr.size = sizeof(idt) - 1;

    set_idt(&ptr);
}

int cnt = 0;

void interrupt_handler(void) { // обрабатывем прерывание.
    printf("current time is %d\n", cnt);
    ++cnt;
}