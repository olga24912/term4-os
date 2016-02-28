#pragma once
#include <stdint.h>

struct memory_map_entry {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
};

void get_memory_map();
void print_mempry_map();

extern size_t memory_map_size;
extern struct memory_map_entry memory_map[];