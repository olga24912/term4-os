#include "lock.h"
#include "util.h"
#include "threads.h"
#include "io.h"
#include "interrupt.h"

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

volatile int critical_section_depth = 0;

void start_critical_section() {
    //printf("critical section start %d, current thread: %d \n", critical_section_depth, get_current_thread());
    interrupt_off();
    __sync_fetch_and_add(&critical_section_depth, 1);
}

void end_critical_section() {
    //printf("critical section end %d\n", critical_section_depth);
    if (__sync_fetch_and_add(&critical_section_depth, -1) == 1) {
        interrupt_on();
    }
}
