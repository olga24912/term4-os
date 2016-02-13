#include "ioport.h"

#define UART_PORT(x) (0x3f8 + (x))
#define LCR UART_PORT(3)
#define LSR UART_PORT(5)
#define DATA UART_PORT(0)

void init_uart() {
    out8(LCR, 3);   
}

void putc(char c) {
    while (!((in8(LSR) >> 5) & 1)) {}
    out8(DATA, c);
}

