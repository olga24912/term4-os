#pragma once

#include <stdint.h>

struct spinlock {
    uint16_t users;
    uint16_t ticket;
};

void lock(struct spinlock *lock);

void unlock(struct spinlock *lock);
