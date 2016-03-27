#include "lock.h"
#include "util.h"
#include "threads.h"

void lock(struct spinlock *lock)
{
    const uint16_t ticket = __sync_fetch_and_add(&lock->users, 1);

    while (lock->ticket != ticket) {
        barrier();
        yield();
    }
    __sync_synchronize(); /* we don't use cmpxchg explicitly */
}

void unlock(struct spinlock *lock)
{
    __sync_synchronize();
    __sync_add_and_fetch(&lock->ticket, 1);
}
