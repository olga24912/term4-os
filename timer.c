#include "ioport.h"
#include "io.h"
#include "timer.h"
#include "interrupt.h"

#define CONTROL_PORT 0x43
#define DATA_PORT 0x40

void init_timer() {
    out8(CONTROL_PORT, (2 << 1)|(3 << 4)); //устанавливаем таймер прерывания
    //76543210
    //00110100
    //[3:1] "--- лежит 2,выбрали номер режима работы.
    //[5:4] "--- записано 3, то есть сначала хотим записать наименее значимый бит, потом наиболее.
    out8(DATA_PORT, 200);
    out8(DATA_PORT, 10);
}

static int cnt = 0;

void timer_interrupt_handler(void) { // обрабатывем прерывание.
    printf("current time is %d\n", cnt);
    ++cnt;
}