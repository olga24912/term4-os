#include "ioport.h"

#define UART_PORT(x) (0x3f8 + (x)) // получение доступа к конкретному порту порта ввода/вывода.
#define LCR UART_PORT(3) // Line Control Register
#define LSR UART_PORT(5) // Line Status Register
#define DATA UART_PORT(0) // регистр данных

void init_uart() {
    out8(LCR, 3);// выставляем 8 бит данных в формате кадра
}

void putc(char c) {
    while (!((in8(LSR) >> 5) & 1)) {} //ждем, когда можно будет передать следующий бит, то есть когда передали старый
    out8(DATA, c); //передаем очередной символ.
}

