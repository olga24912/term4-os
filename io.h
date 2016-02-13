#pragma once

void puts(char *s);

__attribute__((format (printf, 1, 2)))
void printf(const char *fmt, ...);
