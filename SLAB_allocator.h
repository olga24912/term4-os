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
    struct slabctl* next;
    void* slab_list_head;
};

struct slabctl** create_slab_system (unsigned int size, unsigned int al);

void* allocate_block_in_slab_system (struct slabctl** slab_sys);

void free_block(void *addr);