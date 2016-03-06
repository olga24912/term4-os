#pragma once
#include "io.h"
#include "util.h"

#define assert(X) if (!(X)) _assert(#X, __FILE__, __LINE__, __func__)

static inline void _assert(const char* s, const char* fl, int ln, const char* fn) {
    printf("fail %s file: %s line:%d function:%s\n", s, fl, ln, fn);
    hang();
}
