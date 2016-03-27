#pragma once

__attribute__((noreturn))
static inline void hang () {
    while(1) {
        __asm__ volatile ("hlt");
    };
}

static inline int min(int a, int b) {
    return a < b ? a : b;
}

static inline int max(int a, int b) {
    return a > b ? a : b;
}

static inline uint64_t align_up(uint64_t val, uint64_t al) {
    if (val % al) {
        val += al - val%al;
    }
    return val;
}

static inline void barrier() {
    __asm__ volatile ("" : : : "memory");
}