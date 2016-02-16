#include "ioport.h"
#include "io.h"
#include "interrupt.h"

#define CONTROL_PORT 0x43
#define DATA_PORT 0x40


void main(void) {
    __asm__ volatile ("cli"); // запрещаем прерывание
    init_interrupt(); // инициализируем прерывание
    out8(CONTROL_PORT, (2 << 1)|(3 << 4)); //устанавливаем таймер прерывания
    out8(DATA_PORT, 10);
    out8(DATA_PORT, 0);
    __asm__ volatile ("sti"); //разрешаем прерываться

    while (1);
}
