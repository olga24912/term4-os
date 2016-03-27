#include "interrupt.h"
#include "timer.h"
#include "io.h"
#include "util.h"
#include "memory_map.h"
#include "buddy_allocator.h"
#include "paging.h"
#include "threads.h"

void fun(void *arg) {
    while (true) {
        printf("Hello world1\n");
        run_thread(*(pid_t *)arg);
    }
}

void fun2(void *arg) {
    for (int i = 0; i < 100; ++i) {
        printf("Hello world179\n");
        run_thread(*(pid_t *)arg);
    }
    run_thread(1);
}

void test_switch() {
    pid_t t2 = 3;
    pid_t t1 = create_thread(fun, &t2);
    create_thread(fun2, &t1);

    printf("%d %d\n", t1, t2);
    //run_thread(t1);
    yield();
    printf("I am return to main thread\n");
}


void fun_finish(void *arg __attribute__((unused))) {
    printf("Function with one printf\n");
}

void test_finish() {
    pid_t t1 = create_thread(fun_finish, 0);

    printf("%d\n", t1);
    yield();
    printf("I am return to main thread\n");

}

void main(void) {
    interrupt_off();
    get_memory_map();
    init_buddy();
    map_init();

    init_threads();

    test_finish();
    //test_switch();

    init_interrupt(); // инициализируем прерывание
    init_timer();
    //interrupt_on();
    hang();
}
