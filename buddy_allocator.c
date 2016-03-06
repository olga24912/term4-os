#include "buddy_allocator.h"
#include "util.h"

#define MAX_ORDER 64

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
    while (head[lv] == -1 && lv < MAX_ORDER) {
        ++lv;
    }
    if (lv == MAX_ORDER) {
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

void init_buddy() {
    get_memory_map();

    size_t head_size = MAX_ORDER * sizeof(head[0]);
    size_t descriptors_size = ((memory_map[memory_map_size - 1].base_addr + memory_map[memory_map_size - 1].length))/PAGE_SIZE;

    int init_head = 0;
    int init_descriptor = 0;
    for (size_t i = 0; i < memory_map_size; ++i) {
        if (memory_map[i].type == 1) {
            if (init_head == 0 && memory_map[i].length >= head_size) {
                head = va(memory_map[i].base_addr);
                memory_map[i].base_addr += head_size;
                memory_map[i].length -= head_size;
                init_head = 1;
            }
            if (init_descriptor == 0 && memory_map[i].length >= descriptors_size) {
                descriptors = va(memory_map[i].base_addr);
                memory_map[i].base_addr += descriptors_size;
                memory_map[i].length -= descriptors_size;
                init_descriptor = 1;
            }
        }
    }

    for (size_t i = 0; i < MAX_ORDER; ++i) {
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
