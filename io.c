#include <limits.h>
#include <stdint.h>
#include "uart.h"
#include "stdarg.h"

void puts_in(char *s) {
    for (int i = 0; s[i] != 0; ++i) {
        putc(s[i]);
    }
}

void puts(char *s) {
    puts_in(s);
    putc('\n');
}

void print_int(int64_t x) {
    if (x == LLONG_MIN) {
        puts_in("-9223372036854775808");
        return;
    }
    if (x < 0) {
        putc('-');
        x = -x;
    }
    static char buff[30];
    int i = 0;
    for (; x != 0; ++i) {
        buff[i] = x % 10 + '0';
        x /= 10;
    }
    for (int j = i - 1; j >= 0; --j) {
        putc(buff[j]);
    }
}

__attribute__((format (printf, 1, 2)))
void printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    for (int i = 0; fmt[i] != 0; ++i) {
        if (fmt[i] == '%') {
            ++i;
            if (fmt[i] == 'd') {
                int x = va_arg(args, int);
                print_int(x);
                continue;
            }
        } else {
            putc(fmt[i]);
        }
    }

    va_end(args);
}
