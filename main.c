#include "interrupt.h"
#include "timer.h"
#include "io.h"
#include "util.h"
#include "memory_map.h"
#include "buddy_allocator.h"

void main(void) {
    interrupt_off();
    get_memory_map();
    print_mempry_map();
    init_buddy();
    for (int i = 0; i < 10; ++i) {
        void *a = get_page(0);
        printf("%p\n", a);
        free_page(a, 0);
    }
    init_interrupt(); // инициализируем прерывание
    init_timer();
    //interrupt_on();
    hang();
}
