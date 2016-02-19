#include "interrupt.h"
#include "timer.h"

#define CONTROL_PORT 0x43
#define DATA_PORT 0x40


void main(void) {
    interrupt_off();
    init_interrupt(); // инициализируем прерывание
    init_timer();
    interrupt_on();
    while (1);
}
