#pragma once
#include "interrupt.h"
#include "io.h"
#include "memory_map.h"
#include "memory.h"

#pragma pack(push, 1)
struct page_descriptor {
    union {
        struct {
            int next_page_id;
            int prev_page_id;
        };
        void* slab;
    };
    unsigned int in_list : 1;
    unsigned int order : 6;
};
#pragma pack(pop)

extern struct page_descriptor* descriptors;

void* get_page(int k);

void* get_page0(int k);

void free_page(void* page_addr, int k);

void init_buddy();

void init_boot();

void* get_mem(size_t mem_size, size_t alignment);


