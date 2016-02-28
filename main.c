#include "interrupt.h"
#include "timer.h"
#include "io.h"
#include "util.h"
#include "memory_map.h"

void main(void) {
    interrupt_off();
    get_memory_map();
    print_mempry_map();
    init_interrupt(); // инициализируем прерывание
    init_timer();
    //interrupt_on();
    hang();
}
