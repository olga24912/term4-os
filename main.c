#include "interrupt.h"
#include "timer.h"
#include "io.h"
#include "util.h"
#include "memory_map.h"
#include "buddy_allocator.h"
#include "paging.h"
#include "threads.h"
#include "lock.h"
#include "assert.h"

void* fun(void *arg) {
    while (true) {
        printf("Hello world1\n");
        run_thread(*(pid_t *)arg);
    }
    return 0;
}

void* fun2(void *arg) {
    for (int i = 0; i < 100; ++i) {
        printf("Hello world179\n");
        run_thread(*(pid_t *)arg);
    }
    run_thread(1);
    return 0;
}

void test_switch_and_arg() {
    pid_t t2 = 3;
    pid_t t1 = create_thread(fun, &t2);
    create_thread(fun2, &t1);

    printf("%d %d\n", t1, t2);
    yield();
    printf("I am return to main thread\n");
}


void* fun_finish(void *arg __attribute__((unused))) {
    printf("Function with one printf\n");
    return 0;
}

void* test_finish() {
    pid_t t1 = create_thread(fun_finish, 0);

    printf("%d\n", t1);
    yield();
    printf("I am return to main thread\n");
    return 0;
}


struct spinlock spinlock1;
struct spinlock spinlock2;

int val = 0;

void* fun_lock1(void *arg __attribute__((unused))) {
    printf("I am starting working in lock1\n");
    lock(&spinlock1);
    printf("I am in first lock %d\n", val);
    yield();
    lock(&spinlock2);
    printf("I am in second lock %d\n", val);
    val = 1;
    unlock(&spinlock2);
    printf("I am out of second lock %d\n", val);
    unlock(&spinlock1);
    printf("I am out of first lock %d\n", val);
    return 0;
}

void* fun_lock2(void *arg __attribute__((unused))) {
    lock(&spinlock1);
    printf("I am in first lock %d\n", val);
    lock(&spinlock2);
    printf("I am in second lock %d\n", val);
    val = 2;
    unlock(&spinlock2);
    printf("I am out of second lock %d\n", val);
    unlock(&spinlock1);
    printf("I am out of first lock %d\n", val);
    return 0;
}

void test_lock() {
    create_thread(fun_lock1, 0);
    create_thread(fun_lock2, 0);

    yield();
    yield();
    printf("I am return to main thread\n");
}

void* fun_join(void *arg __attribute__((unused))) {
    for (int i = 0; i < 9; ++i) {
        yield();
        printf("yield number %d\n", i + 1);
    }
    return fun_join;
}

void test_join() {
    pid_t t1 = create_thread(fun_join, 0);

    void* ret;
    thread_join(t1, &ret);

    printf("%p\n", ret);
    assert(ret == fun_join);
}

void main(void) {
    start_critical_section();
    get_memory_map();
    init_buddy();
    map_init();

    init_threads();

    test_finish();
    //test_switch_and_arg();
    //test_lock();
    //test_join();

    init_interrupt(); // инициализируем прерывание
    init_timer();
    //end_critical_section();
    hang();
}
