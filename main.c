#include "interrupt.h"
#include "timer.h"
#include "io.h"
#include "util.h"
#include "memory_map.h"
#include "buddy_allocator.h"
#include "paging.h"

#define assert(X) if (!(X)) _assert(#X, __FILE__, __LINE__, __func__)

void _assert(const char* s, const char* fl, int ln, const char* fn) {
    printf("fail %s file: %s line:%d function:%s\n", s, fl, ln, fn);
    hang();
}

pte_t* pml4;

void force_pte(pte_t* pmle) {
    if (!pte_present(*pmle)) {
        void *npg = get_page0(0);

        assert(npg != 0);
        assert((pa(npg) & ((1 << 12) - 1)) == 0);

        *pmle = pa(npg) | PTE_PRESENT | PTE_WRITE;
    }
}

void map_adr(virt_t vad, phys_t pad, int big_page) {
    if (big_page) {
        assert((vad & ((1 << (12 + 9)) - 1)) == 0);
        assert((pad & ((1 << (12 + 9)) - 1)) == 0);
    } else {
        assert((vad & ((1 << (12)) - 1)) == 0);
        assert((pad & ((1 << (12)) - 1)) == 0);
    }

    pte_t *pml4e = pml4 + pml4_i(vad);

    force_pte(pml4e);

    pte_t *pdpte = ((pte_t *) va(pte_phys(*pml4e) << 12)) + pml3_i(vad);

    force_pte(pdpte);

    pte_t *pde = ((pte_t *) va(pte_phys(*pdpte) << 12)) + pml2_i(vad);
    if (big_page) {
        assert(pte_present(*pde) == false);
        *pde = pad | PTE_PRESENT | PTE_WRITE | PTE_LARGE;

        flush_tlb_addr(vad);
        return;
    }

    force_pte(pde);

    pte_t *pte = ((pte_t *) va(pte_phys(*pde) << 12)) + pml1_i(vad);
    assert(pte_present(*pte) == false);
    *pte = pad | PTE_PRESENT | PTE_WRITE;

    flush_tlb_addr(vad);
}


void map_init() {
    for (phys_t i = 0; i < memory_map[memory_map_size - 1].base_addr + memory_map[memory_map_size - 1].length; i += (1 << 21)) {
        map_adr(i + HIGH_BASE, i, 1);
        if (i + KERNEL_BASE > HIGH_BASE) {
            map_adr(i + KERNEL_BASE, i, 1);
        }
    }

    puts("init page mapping");

    store_pml4(pa(pml4));
}


void main(void) {
    interrupt_off();
    get_memory_map();
    print_mempry_map();
    init_buddy();

    pml4 = get_page0(0);

    map_init();

    init_interrupt(); // инициализируем прерывание
    init_timer();
    interrupt_on();
    hang();
}
