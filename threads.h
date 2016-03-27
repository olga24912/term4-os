#pragma once

#include <sys/types.h>

void init_threads();

pid_t create_thread(void* (*fptr)(void *), void *arg);

void run_thread(pid_t tid);

void yield();

pid_t get_current_thread();

void thread_join(pid_t thread, void** retval);