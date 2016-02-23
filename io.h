#pragma once

#include <stdarg.h>
#include <sys/types.h>

void puts(char *s);

__attribute__((format (printf, 1, 2)))
int printf(const char *fmt, ...);

int vprintf(const char *fmt, va_list args);

__attribute__((format (printf, 3, 4)))
int snprintf(char *s, size_t len, const char *fmt, ...);

int vsnprintf(char *s, size_t len, const char *fmt, va_list args);
