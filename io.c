#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include "uart.h"
#include "stdarg.h"
#include "io.h"

int putc_s(char c, char **buffer, size_t *len) {
    if (buffer == NULL) {
        putc(c);
        return 1;
    }
    if (*len == 0) {
        return 0;
    } else {
        --*len;
        (*buffer)[0] = c;
        ++*buffer;
        return 1;
    }
}

int puts_in(char *s, char **buffer, size_t *len) {
    int res = 0;
    for (int i = 0; s[i] != 0; ++i) {
        if (putc_s(s[i], buffer, len)) {
            ++res;
        } else {
            break;
        }
    }
    return res;
}

void puts(char *s) {
    puts_in(s, NULL, NULL);
    putc('\n');
}

int print_uint(uint64_t x, int base, char **buffer, size_t *len) {
    static char buff[30];
    if (x == 0) {
        return puts_in("0", buffer, len);
    }
    int i = 0;
    int res = 0;
    for (; x != 0; ++i) {
        buff[i] = x % base + '0';
        if (buff[i] > 9 + '0') {
            buff[i] = buff[i] - '0' - 10 + 'a';
        }
        x /= base;
    }
    for (int j = i - 1; j >= 0; --j) {
        if (putc_s(buff[j], buffer, len)) {
            ++res;
        } else {
            break;
        }
    }
    return res;
}

int print_int(int64_t x, char **buffer, size_t *len) {
    if (x == LLONG_MIN) {
        return puts_in("-9223372036854775808", buffer, len);
    }
    int res = 0;
    if (x < 0) {
        if (putc_s('-', buffer, len)) {
            ++res;
        }
        x = -x;
    }
    return res + print_uint(x, 10, buffer, len);
}

#define GET_SIZED_INT(sign, size) \
if (cntl == 0 && cnth == 0 && cntz == 0) { \
    x = va_arg(args, sign int); \
} else if (cntl == 1) {\
    x = va_arg(args, sign long);\
} else if (cntl == 2) {\
    x = va_arg(args, sign long long);\
} else if (cnth == 1) {\
    x = (sign short)va_arg(args, sign int);\
} else if (cnth == 2) {\
    x = (sign char)va_arg(args, sign int);\
} else if (cntz == 1) {\
    x = va_arg(args, size);\
}\


int vsnprintf_internal(char **s, size_t *len, const char *fmt, va_list args) {
    if (len != NULL && *len == 0) {
        return 0;
    }
    if (len != NULL) --*len;

    int count = 0;

    for (int i = 0; fmt[i] != 0; ++i) {
        if (len != NULL && *len == 0) {
            break;
        }
        if (fmt[i] == '%') {
            ++i;
            int cntl = 0, cnth = 0, cntz = 0;
            while (fmt[i] == 'l') {
                ++cntl;
                ++i;
            }
            while (fmt[i] == 'h') {
                ++cnth;
                ++i;
            }
            while (fmt[i] == 'z') {
                ++cntz;
                ++i;
            }
            if (fmt[i] == 'd' || fmt[i] == 'i') {
                int64_t x;
                GET_SIZED_INT(signed, ssize_t);
                count += print_int(x, s, len);
                continue;
            } else if (fmt[i] == 'p') {
                void *x = va_arg(args, void*);
                count += puts_in("0x", s, len);
                count += print_uint((uint64_t) x, 16, s, len);
                continue;
            } else if (fmt[i] == 'u') {
                uint64_t x;
                GET_SIZED_INT(unsigned, size_t);
                count += print_uint((uint64_t) x, 10, s, len);
                continue;
            } else if (fmt[i] == 'o') {
                uint64_t x;
                GET_SIZED_INT(unsigned, size_t);
                count += print_uint((uint64_t) x, 8, s, len);
                continue;
            } else if (fmt[i] == 'x') {
                uint64_t x;
                GET_SIZED_INT(unsigned, size_t);
                count += print_uint((uint64_t) x, 16, s, len);
                continue;
            } else if (fmt[i] == 'c') {
                char x = va_arg(args, int);
                count += putc_s(x, s, len);
                continue;
            } else if (fmt[i] == 's') {
                char *x = va_arg(args, char*);
                count += puts_in(x, s, len);
                continue;
            } else {
                count += putc_s(fmt[i], s, len);
            }
        } else {
            count += putc_s(fmt[i], s, len);
        }
    }
    if (s != NULL) {
        *s = 0;
    }
    return count;
}

int vsnprintf(char *s, size_t len, const char *fmt, va_list args) {
    return vsnprintf_internal(&s, &len, fmt, args);
}

int vprintf(const char *fmt, va_list args) {
    return vsnprintf_internal(NULL, NULL, fmt, args);
}

int printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int count = vprintf(fmt, args);
    va_end(args);
    return count;
}

int snprintf(char *s, size_t len, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int count = vsnprintf(s, len, fmt, args);
    va_end(args);
    return count;
}

#undef GET_SIZED_INT
