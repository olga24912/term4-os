#include "interrupt.h"
#include "timer.h"
#include "io.h"
#include "util.h"
#include "memory_map.h"
#include "buddy_allocator.h"
#include "paging.h"

#define CNT_PAGES 4

struct slabctl {
    uint16_t block_size;
    uint16_t alignment;
    uint16_t head;
    uint16_t cnt_ref;
};

void* allocate_slab(unsigned int size, unsigned int al);

void* allocate_block(struct slabctl* slab);

void free_addr(void* addr);