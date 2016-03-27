#pragma once

#include <stdint.h>

struct spinlock {
    uint16_t users;
    uint16_t ticket;
};

void lock(struct spinlock *lock);

void unlock(struct spinlock *lock);

void end_critical_section();

void start_critical_section();