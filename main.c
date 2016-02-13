#include "ioport.h"
#include "uart.h"

void puts(char* s) {
    for (int i = 0; s[i] != 0; ++i) {
        putc(s[i]);
    }
    putc('\n');
}

void main(void) {
    puts("hello"); 
    while (1);
}
