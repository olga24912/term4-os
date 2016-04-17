#include "interrupt.h"
#include "timer.h"
#include "io.h"
#include "util.h"
#include "memory_map.h"
#include "buddy_allocator.h"
#include "paging.h"
#include "threads.h"
#include "lock.h"
#include "test_thread.h"
#include "file_system.h"
#include "initramfs.h"

void main(void) {
    start_critical_section();
    get_memory_map();
    init_buddy();
    map_init();
    print_mempry_map();
    init_malloc_small();

    init_threads();
    init_interrupt(); // инициализируем прерывание
    init_timer();

    init_file_system();

    initramfs_to_fs();

    /*int id = open("/file", O_CREAT|O_WRONLY|O_TRUNC);
    open("/file2", O_CREAT|O_WRONLY|O_TRUNC);
    open("/file3", O_WRONLY|O_TRUNC);

    mkdir("/dir1");
    mkdir("/dir1/dir2");

    open("/dir1/dir2/file2", O_CREAT|O_WRONLY|O_TRUNC);
    open("/dir1/file179", O_CREAT|O_WRONLY|O_TRUNC);

    for (int i = 0; i < 50; ++i) {
        write(id, "a", 1);
    }

    close(id);

    id = open("/file", O_RDONLY);

    printf("%d\n", id);
    char s[3];
    int size = 0;
    while ((size = read(id, s, 3)) > 0) {
        for (int i= 0; i < size; ++i) {
            printf("%c", s[i]);
        }
    }
    printf("\n");*/

    print_file_system();

    end_critical_section();

    hang();
}
