#include "interrupt.h"
#include "timer.h"
#include "io.h"
#include "util.h"
#include "memory_map.h"
#include "buddy_allocator.h"
#include "paging.h"
#include "SLAB_allocator.h"

void main(void) {
    interrupt_off();
    get_memory_map();
    print_mempry_map();
    init_buddy();
    map_init();

    struct slabctl* mslab = allocate_slab(1000, 2);

    for (int i = 0; i < 10; ++i) {
        void *a = allocate_block(mslab);
        printf("%p\n", a);
        //free_addr(a);
    }

    init_interrupt(); // инициализируем прерывание
    init_timer();
    //interrupt_on();
    hang();
}
