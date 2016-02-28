#include <stddef.h>
#include "memory_map.h"
#include "io.h"
#include "util.h"

#define MMAP_SIZE 32

extern const uint32_t mboot_info;

extern char text_phys_begin[];
extern char bss_phys_end[];

size_t memory_map_size = 0;
struct memory_map_entry memory_map[MMAP_SIZE];

static struct memory_map_entry kernel;

static void cut_last_elem() {
    if (memory_map[memory_map_size - 1].base_addr >= kernel.base_addr) {
        if (memory_map[memory_map_size - 1].length + memory_map[memory_map_size - 1].base_addr <= kernel.base_addr + kernel.length) {
            --memory_map_size;
            return;
        }
        if (memory_map[memory_map_size - 1].base_addr >= kernel.base_addr + kernel.length) {
            return;
        }
        uint64_t end = memory_map[memory_map_size - 1].base_addr + memory_map[memory_map_size - 1].length;
        memory_map[memory_map_size - 1].base_addr = kernel.base_addr + kernel.length;
        memory_map[memory_map_size - 1].length = end - memory_map[memory_map_size - 1].base_addr;
    } else {
        if (kernel.base_addr >= memory_map[memory_map_size - 1].base_addr + memory_map[memory_map_size - 1].length) {
            return;
        }
        if (memory_map[memory_map_size - 1].length + memory_map[memory_map_size - 1].base_addr >= kernel.base_addr + kernel.length) {
            uint64_t end = memory_map[memory_map_size - 1].base_addr + memory_map[memory_map_size - 1].length;
            memory_map[memory_map_size - 1].length = kernel.base_addr - memory_map[memory_map_size - 1].base_addr;

            memory_map[memory_map_size].base_addr = kernel.base_addr + kernel.length;
            memory_map[memory_map_size].length = end - memory_map[memory_map_size].base_addr;
            memory_map[memory_map_size].type = memory_map[memory_map_size - 1].type;
            ++memory_map_size;
            return;
        }
        memory_map[memory_map_size - 1].length = kernel.base_addr - memory_map[memory_map_size].base_addr;
    }

}

static void sort_mmap() {
    for (size_t i = 0; i < memory_map_size; ++i) {
        for (size_t j = i + 1; j < memory_map_size; ++j) {
            if (memory_map[i].base_addr >= memory_map[j].base_addr) {
                struct memory_map_entry k = memory_map[i];
                memory_map[i] = memory_map[j];
                memory_map[j] = k;
            }
        }
    }
}

void get_memory_map(){
    kernel.base_addr = (uint64_t)text_phys_begin;
    kernel.length = (uint64_t)bss_phys_end - kernel.base_addr;
    kernel.type = 0;

    /*
     * https://www.gnu.org/software/grub/manual/multiboot/multiboot.html
     * 3.3 Boot information format
    */
    uint32_t flags = *(uint32_t *) (uint64_t) mboot_info;
    if (!((flags >> 6) & 1)) { //if flags[6] is not set then mmap_length and mmap_addr not present
        puts("NO MMAP PROVIDED");
        hang();
    }

    uint32_t mmap_length = *(uint32_t *) (uint64_t) (mboot_info + 44);
    uint32_t mmap_addr = *(uint32_t *) (uint64_t) (mboot_info + 48);

    //Went in MEMORY MAP
    uint32_t cnt_skip = 0;
    while (cnt_skip < mmap_length) {
        uint32_t size = *(uint32_t *)(uint64_t) (mmap_addr + cnt_skip);
        cnt_skip += 4;
        memory_map[memory_map_size].base_addr =*(uint64_t*)(uint64_t)(mmap_addr + cnt_skip);
        memory_map[memory_map_size].length = *(uint64_t*)(uint64_t)(mmap_addr + cnt_skip + 8);
        memory_map[memory_map_size].type = *(uint32_t*)(uint64_t)(mmap_addr + cnt_skip + 16);
        cnt_skip += size;
        ++memory_map_size;

        cut_last_elem();

    }

    memory_map[memory_map_size] = kernel;
    ++memory_map_size;

    sort_mmap();
}

void print_mempry_map() {
    for (size_t i = 0; i < memory_map_size; ++i) {
        printf("%llx - %llx, %s\n", (unsigned long long) memory_map[i].base_addr,
               (unsigned long long int) (memory_map[i].base_addr + memory_map[i].length - 1),
               memory_map[i].type == 1 ? "Available" : memory_map[i].type == 0 ? "Kernel" : "Reserved");
    }
}



