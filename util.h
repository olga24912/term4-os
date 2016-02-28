#pragma once

__attribute__((noreturn))
static inline void hang () {
    while(1);
}