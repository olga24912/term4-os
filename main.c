#include <limits.h>
#include "ioport.h"
#include "uart.h"
#include "stdarg.h"
#include "io.h"
#include "interrupt.h"

#define CONTROL_PORT 0x43
#define DATA_PORT 0x40


void main(void) {
    __asm__ volatile ("cli");
    init_interrupt();
    puts("init fin");
    puts("init fin");
    //out8(CONTROL_PORT, (2 << 1)|(3 << 4));
    //out8(DATA_PORT, 255);
    //out8(DATA_PORT, 110);
    puts("cd_port");
    __asm__ volatile ("sti");
    puts("main");
    printf("%d\n", in8(0x21));

    while (1);
}
