#include "buddy_allocator.h"
#include "util.h"

int max_order;

int* head;
struct page_descriptor* descriptors;

int pair_id(int id, int lv) {
    return id ^ (1 << lv);
}

void del_page(int id, int k) {
    if (descriptors[id].prev_page_id == id) {
        head[k] = -1;
    } else {
        if (head[k] == id) {
            head[k] = descriptors[id].next_page_id;
        }
        descriptors[descriptors[id].next_page_id].prev_page_id = descriptors[id].prev_page_id;
        descriptors[descriptors[id].prev_page_id].next_page_id = descriptors[id].next_page_id;
    }
    descriptors[id].in_list = 0;
}

void add_page(int id, int k) {
    descriptors[id].in_list = 1;
    descriptors[id].order = k;

    if (head[k] == -1) {
        head[k] = id;
        descriptors[id].next_page_id = id;
        descriptors[id].prev_page_id = id;
    } else {
        descriptors[id].next_page_id = head[k];
        descriptors[id].prev_page_id = descriptors[head[k]].prev_page_id;
        descriptors[descriptors[head[k]].prev_page_id].next_page_id = id;
        descriptors[head[k]].prev_page_id = id;
    }
}

void* get_page(int k) {
    int lv = k;
    while (head[lv] == -1 && lv < max_order) {
        ++lv;
    }
    if (lv == max_order) {
        return 0;
    }
    for (; lv > k; --lv) {
        int num_p = head[lv];
        del_page(num_p, lv);
        add_page(num_p, lv - 1);
        add_page(pair_id(num_p, lv - 1), lv - 1);
    }
    int val = head[k];
    del_page(head[k], k);
    return va(val * (uint64_t)PAGE_SIZE);
}

void free_page(void* page_addr, int k) {
    int id = pa(page_addr)/PAGE_SIZE;
    while (1) {
        int pid = pair_id(id, k);
        if (descriptors[pid].in_list == 1 && descriptors[pid].order == k) {
            del_page(pid, k);
            id = min(pid, id);
            ++k;
        } else {
            add_page(id, k);
            break;
        }
    }
}


size_t boot_size = 1e6;
void* boot_mem;

void init_boot() {
    for (size_t i = 0; i < memory_map_size; ++i) {
        if (memory_map[i].type == 1) {
            if (memory_map[i].length >= boot_size) {
                boot_mem = va(memory_map[i].base_addr);
                memory_map[i].base_addr += boot_size;
                memory_map[i].length -= boot_size;
            }
        }
    }
}

void* get_mem(size_t mem_size, size_t alignment) {
    char* res = boot_mem;
    if (alignment != 0) {
        res = (char *) ((((uint64_t)res + 1) / alignment) * alignment);
    }
    boot_size -= ((uint64_t)res - (uint64_t)boot_mem) + mem_size;
    boot_mem = res + boot_size;

    for (size_t i = 0; i < mem_size; ++i) {
        res[i] = 0;
    }

    return res;
}

void init_buddy() {
    get_memory_map();
    size_t descriptors_size = ((memory_map[memory_map_size - 1].base_addr + memory_map[memory_map_size - 1].length))/PAGE_SIZE;

    max_order = 1;
    while ((1ll<<max_order) <= (int)descriptors_size) {
        ++max_order;
    }

    size_t head_size = max_order * sizeof(head[0]);

    boot_size += head_size + descriptors_size;

    init_boot();

    head = get_mem(head_size, 0);
    descriptors = get_mem(descriptors_size, 0);

    for (size_t i = 0; i < (size_t)max_order; ++i) {
        head[i] = -1;
    }

    for (size_t i = 0; i < memory_map_size; ++i) {
        if (memory_map[i].type == 1) {
            while (memory_map[i].length >= PAGE_SIZE) {
                uint64_t start_addr = ((memory_map[i].base_addr + 1) / PAGE_SIZE) * PAGE_SIZE;
                if (start_addr + PAGE_SIZE > memory_map[i].base_addr + memory_map[i].length) {
                    memory_map[i].length = PAGE_SIZE - 1;
                    continue;
                }
                descriptors[start_addr/PAGE_SIZE].in_list = 1;
                descriptors[start_addr/PAGE_SIZE].order = 0;
                free_page(va(start_addr), 0);
                memory_map[i].base_addr = start_addr + PAGE_SIZE;
                memory_map[i].length -= PAGE_SIZE;
            }
        }
    }

    get_memory_map();
}

void* get_page0(int k) {
    uint64_t* adr = get_page(k);
    for (size_t i = 0; i < (1 << k)*PAGE_SIZE/sizeof(adr[0]); ++i) {
        adr[i] = 0;
    }
    return adr;
}
