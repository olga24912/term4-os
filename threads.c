#include "interrupt.h"
#include "threads.h"
#include "util.h"
#include "buddy_allocator.h"
#include "assert.h"
#include "lock.h"

#define MAX_THREADS (1 << 16)
typedef enum {NOT_STARTED = 0, RUNNING, FINISHED, WAIT_JOIN} thread_state;

struct thread {
    void* stack_pointer;
    void* stack_start;
    int cnt_log_page;
    void * ret_val;

    thread_state state;
};

static volatile pid_t current_thread = 1;
static volatile pid_t previous_thread = 1;

struct threads_pool {
    volatile pid_t first_free;
    volatile struct thread threads[MAX_THREADS];
    volatile pid_t next[MAX_THREADS];
    volatile pid_t prev[MAX_THREADS];
};

static struct threads_pool tp;

void init_threads() {
    tp.first_free = 2;
    for (int i = 2; i < MAX_THREADS - 1; ++i) {
        tp.next[i] = i + 1;
    }
    tp.next[1] = tp.prev[1] = 1;

    tp.threads[1].state = RUNNING;
    tp.threads[0].state = NOT_STARTED;
}

volatile struct thread *get_free_thread() {
    pid_t first = tp.first_free;
    tp.first_free = tp.next[first];

    tp.next[first] = tp.next[1];
    tp.prev[first] = 1;
    tp.next[1] = first;
    tp.prev[tp.next[1]] = first;

    return &tp.threads[first];
}

pid_t create_thread(void* (*fptr)(void *), void *arg) {
    start_critical_section();
    volatile struct thread *new_thread = get_free_thread();
    new_thread->cnt_log_page = 10;
    new_thread->stack_start = get_page0(new_thread->cnt_log_page);

    new_thread->stack_pointer = (uint8_t *)new_thread->stack_start + PAGE_SIZE * (1 << (new_thread->cnt_log_page));

    struct init_thread_data  {
        uint64_t r15, r14, r13, r12, rbx, rbp;
        void* start_thread_addr;
        void* fun_addr;
        void* arg;
    };

    new_thread->stack_pointer = (uint8_t *)new_thread->stack_pointer - sizeof(struct init_thread_data);

    struct init_thread_data* init_val = new_thread->stack_pointer;

    init_val->r12 = 0;
    init_val->r13 = 0;
    init_val->r14 = 0;
    init_val->r15 = 0;
    init_val->rbx = 0;
    init_val->rbp = 0;

    extern void *start_thread;
    init_val->start_thread_addr = &start_thread;

    init_val->fun_addr = fptr;
    init_val->arg = arg;

    new_thread->state = RUNNING;
    end_critical_section();
    return (pid_t)(new_thread - tp.threads);
}

void switch_threads(void **old_sp, void *new_sp);


void check_thread_finished() {
    //printf("check thread fin %d\n", previous_thread);
    volatile struct thread* thread = tp.threads + previous_thread;

    if (thread->state == FINISHED && previous_thread != current_thread) {
        free_page(thread->stack_start, thread->cnt_log_page);
        thread->state = WAIT_JOIN;
    }
}

void run_thread(pid_t tid) {
    if (current_thread == tid) {
        return;
    }
    struct thread *thread = (struct thread*)tp.threads + tid;

    int ot = current_thread;
    current_thread = tid;
    previous_thread = ot;

    struct thread *othread = (struct thread*)tp.threads + ot;

    //printf("before switch from %d to %d\n", ot, tid);
    switch_threads(&othread->stack_pointer, thread->stack_pointer);

    //printf("after switch from %d to %d, pr_t %d\n", ot, tid, previous_thread);

    check_thread_finished();
}

void finish_current_thread(void* val) {
    start_critical_section();
    int ct = get_current_thread();

    printf("thread finish %d\n", ct);

    volatile struct thread* current_t = tp.threads + ct;
    current_t->state = FINISHED;
    current_t->ret_val = val;


    tp.prev[tp.next[ct]] = tp.prev[ct];
    tp.next[tp.prev[ct]] = tp.next[ct];

    end_critical_section();
    yield();
    assert(0);
}

void yield() {
    start_critical_section();
    for (pid_t i = tp.next[current_thread];; i = tp.next[current_thread]) {
        if (i == 0 || tp.threads[i].state != RUNNING) continue;
        run_thread(i);
        break;
    }
    end_critical_section();
}


void thread_join(pid_t thread, void** retval) {
    while (tp.threads[thread].state != WAIT_JOIN) {
        yield();
        barrier();
    }
    if (retval) {
        *retval = tp.threads[thread].ret_val;
    }
    tp.threads[thread].state = NOT_STARTED;

    start_critical_section();
    tp.next[thread] = tp.first_free;
    tp.first_free = thread;
    end_critical_section();
}

pid_t get_current_thread() {
    return current_thread;
}