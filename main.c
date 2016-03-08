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

    struct slabctl** mslab = create_slab_system(1000, 2);
    struct slabctl** mslab10 = create_slab_system(10, 2);

    for (int i = 0; i < 50000; ++i) {
        void *a = allocate_block_in_slab_system(mslab10);
        void *b = allocate_block_in_slab_system(mslab);
        printf("%p %p\n", a, b);
        //free_addr(a);
    }

    init_interrupt(); // инициализируем прерывание
    init_timer();
    //interrupt_on();
    hang();
}
