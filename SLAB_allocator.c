#include <stddef.h>
#include "SLAB_allocator.h"
#include "assert.h"

#define MALLOC_SMALL_MAX_SIZE 256

static struct spinlock slab_lock;

struct bufctl {
    void* buf_adr;
    struct bufctl* next_ctl;
    struct slab* slab_ctl;
};

void* heads;

void* get_page_adr(void* adr) {
    return (void*)((uint64_t)adr&(~(PAGE_SIZE - 1)));
}

uint64_t get_buffsize(struct slabctl* slab) {
    return (uint64_t)max(2, (int)align_up(slab->block_size, slab->alignment));
}

void* small_slab_get_buffer_addr(struct slabctl* slab, int id) {
    uint64_t start_page_adr = align_up((uint64_t)get_page_adr(slab), slab->alignment);
    uint64_t buffsize = get_buffsize(slab);
    return (void*)(start_page_adr + buffsize*id);
}

void* big_slab_get_buffer_addr(struct slabctl* slab, int id, int cnt_page) {
    uint64_t start_page_adr = align_up((uint64_t)get_page_adr(slab)-(cnt_page - 1)*PAGE_SIZE, slab->alignment);
    uint64_t buffsize = get_buffsize(slab);
    return (void*)(start_page_adr + buffsize*id);
}

size_t small_slab_cnt(struct slabctl* slab) {
    uint64_t start_page_adr = align_up((uint64_t)get_page_adr(slab), slab->alignment);
    uint64_t buffsize = get_buffsize(slab);
    return ((uint64_t)slab - start_page_adr)/buffsize;
}

size_t big_slab_cnt(struct slabctl* slab, int cnt_page) {
    uint64_t start_page_adr = align_up((uint64_t)get_page_adr(slab)-(cnt_page - 1)*PAGE_SIZE, slab->alignment);
    uint64_t buffsize = get_buffsize(slab);
    return ((uint64_t)slab - start_page_adr)/buffsize;
}


int get_big_id(struct slabctl* slab, void* addr, int cnt_page) {
    uint64_t start_page_adr = align_up((uint64_t)get_page_adr(slab)-(cnt_page - 1)*PAGE_SIZE, slab->alignment);
    uint64_t buffsize = get_buffsize(slab);
    return (int)(((uint64_t)addr - start_page_adr)/buffsize);
}

int get_small_id (struct slabctl* slab, void* addr) {
    uint64_t start_page_adr = align_up((uint64_t)get_page_adr(slab), slab->alignment);
    uint64_t buffsize = get_buffsize(slab);
    return (int)(((uint64_t)addr - start_page_adr)/buffsize);
}

void* allocate_slab_small(unsigned int size, unsigned int al) {
    void* buf = get_page(0);
    struct slabctl* slab_control = (struct slabctl*)((char*)buf + PAGE_SIZE - sizeof(struct slabctl));
    slab_control->alignment = al;
    slab_control->block_size = size;
    slab_control->cnt_ref = small_slab_cnt(slab_control);
    slab_control->head=0;

    descriptors[get_phys_adr((virt_t)((char*)buf))/PAGE_SIZE].slab = slab_control;

    size_t cnt = small_slab_cnt(slab_control);

    for (uint16_t i = 0; i < cnt; ++i) {
        uint16_t * curbuff = small_slab_get_buffer_addr(slab_control, i);
        *curbuff = i + (uint16_t)1;
    }
    return slab_control;
}

void* allocate_slab_big(unsigned int size, unsigned int al) {
    void* buf = get_page(2);

    struct slabctl* slab_control = (struct slabctl*)((char*)buf + CNT_PAGES * PAGE_SIZE - sizeof(struct slabctl));
    for (int i = 0; i < CNT_PAGES; ++i) {
        descriptors[get_phys_adr((virt_t)((char*)buf + i*PAGE_SIZE))/PAGE_SIZE].slab = slab_control;
    }

    slab_control->alignment = al;
    slab_control->block_size = size;
    slab_control->cnt_ref = big_slab_cnt(slab_control, CNT_PAGES);
    slab_control->head=0;

    size_t cnt = big_slab_cnt(slab_control, CNT_PAGES);
    for (uint16_t i = 0; i < cnt; ++i) {
        uint16_t * curbuff = big_slab_get_buffer_addr(slab_control, i, CNT_PAGES);
        *curbuff = i + (uint16_t)1;
    }
    return slab_control;
}

void* allocate_slab(unsigned int size, unsigned int al) {
    if (al == 0) {
        al = 1;
    }
    void* ret;
    if (size*8 <= PAGE_SIZE) {
        ret = allocate_slab_small(size, al);
    } else {
        ret = allocate_slab_big(size, al);
    }
    return ret;
}

int is_big_slab(struct slabctl* slab) {
    return slab->block_size * 8 > PAGE_SIZE;
}

void* allocate_block(struct slabctl* slab) {
    if (is_big_slab(slab)) {
        void* res = big_slab_get_buffer_addr(slab, slab->head, CNT_PAGES);
        slab->head = *((uint16_t*)res);
        slab->cnt_ref--;
        return res;
    }
    void* res = small_slab_get_buffer_addr(slab, slab->head);
    slab->head = *((uint16_t*)res);
    slab->cnt_ref--;
    return res;
}

void free_block(void *addr) {
    void* pg_addr = get_page_adr(addr);
    struct slabctl* sl = descriptors[get_phys_adr((virt_t)pg_addr)/PAGE_SIZE].slab;
    start_critical_section();
    int id;
    if (is_big_slab(sl)) {
        id = get_big_id(sl, addr, CNT_PAGES);
    } else {
        id = get_small_id(sl, addr);
    }
    *((uint16_t*)addr) = sl->head;
    sl->head = id;
    sl->cnt_ref++;
    if (sl->cnt_ref == 1) {
        sl->next = *((struct slabctl**)sl->slab_list_head);
        *((struct slabctl**)sl->slab_list_head) = sl;
    }
    end_critical_section();
}

void* allocate_block_in_slab_system (struct slabctl** slab_sys) {
    struct slabctl** head = slab_sys;
    lock(&(*head)->lock);
    void* ret = allocate_block(*head);
    if ((*head)->cnt_ref == 0) {
        if ((*head)->next == (*head)) {
            *head = allocate_slab((*head)->block_size, (*head)->alignment);
            (*head)->slab_list_head = head;
            (*head)->next = *head;
        } else {
            (*head) = (*head)->next;
        }
    }
    unlock(&(*head)->lock);
    return ret;
}

struct slabctl** create_slab_system (unsigned int size, unsigned int al) {
    lock(&slab_lock);
    if (heads == NULL) {
        heads = get_page(0);
    }
    struct slabctl** head = heads;
    heads = (void*)((uint64_t) heads + sizeof(struct slabctl**));
    *head = (struct slabctl*)allocate_slab(size, al);
    (*head)->slab_list_head = head;
    (*head)->next = *head;
    unlock(&slab_lock);
    return head;
}

struct slabctl** small_slabs[MALLOC_SMALL_MAX_SIZE];

void init_malloc_small() {
    for (unsigned i = 1; i < MALLOC_SMALL_MAX_SIZE; ++i) {
        small_slabs[i] = create_slab_system(i, 1);
    }
}

void* malloc_small (unsigned int size) {
    assert(size > 0);
    assert(size < MALLOC_SMALL_MAX_SIZE);
    return allocate_block_in_slab_system(small_slabs[size]);
}