#include "interrupt.h"
#include "timer.h"
#include "io.h"
#include "util.h"
#include "memory_map.h"
#include "buddy_allocator.h"
#include "paging.h"
#include "threads.h"
#include "lock.h"
#include "test_thread.h"


void main(void) {
    start_critical_section();
    get_memory_map();
    init_buddy();
    map_init();

    init_threads();

    //test_finish();
    test_switch_and_arg();
    //test_lock();
    //test_join();

    init_interrupt(); // инициализируем прерывание
    init_timer();
    end_critical_section();

    //test_timer_interrupt();
    //test_slab();

    hang();
}
