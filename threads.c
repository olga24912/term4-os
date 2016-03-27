#include "interrupt.h"
#include "threads.h"
#include "util.h"
#include "buddy_allocator.h"
#include "assert.h"


typedef enum {NOT_STARTED = 0, RUNNING, FINISHED} thread_state;

struct thread {
    void* stack_pointer;
    void* stack_start;
    int cnt_log_page;

    thread_state state;
};

static volatile pid_t current_thread = 1;

struct threads_pool {
    volatile pid_t size;
    volatile struct thread threads[(1 << 16)];
};

static struct threads_pool tp;

void init_threads() {
    tp.size = 2;
    tp.threads[1].state = RUNNING;
    tp.threads[0].state = NOT_STARTED;
}

volatile struct thread *get_free_thread() {
    return &tp.threads[__sync_fetch_and_add(&tp.size, 1)];
}

pid_t create_thread(void (*fptr)(void *), void *arg) {
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
    return (pid_t)(new_thread - tp.threads);
}

pid_t switch_threads(void **old_sp, void *new_sp);


void check_thread_finished(pid_t previous_thread) {
    volatile struct thread* thread = tp.threads + previous_thread;

    if (thread->state == FINISHED && previous_thread != current_thread) {
        free_page(thread->stack_start, thread->cnt_log_page);
        thread->state = NOT_STARTED;
    }
}

void run_thread(pid_t tid) {
    if (current_thread == tid) {
        return;
    }
    struct thread *thread = (struct thread*)tp.threads + tid;

    int ot = current_thread;
    current_thread = tid;

    struct thread *othread = (struct thread*)tp.threads + ot;

    printf("switch from %d to %d\n", ot, tid);
    pid_t pr_t = switch_threads(&othread->stack_pointer, thread->stack_pointer);

    check_thread_finished(pr_t);
}

void finish_thread() {
    volatile struct thread* current_t = tp.threads + get_current_thread();
    current_t->state = FINISHED;
    yield();
    assert(0);
}

void yield() {
    interrupt_off();
    for (pid_t i = (current_thread + 1u)%tp.size;; i = (i + 1u)%tp.size) {
        if (i == 0 || tp.threads[i].state != RUNNING) continue;
        run_thread(i);
        break;
    }
    interrupt_on();
}


pid_t get_current_thread() {
    return current_thread;
}