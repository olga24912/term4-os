#include "paging.h"
#include "io.h"
#include "memory_map.h"
#include "buddy_allocator.h"
#include "assert.h"

pte_t *pml4;

void force_pte(pte_t *pmle, int flags) {
    if (!pte_present(*pmle)) {
        void *npg;
        if (flags & USE_BOOT_ALLOCATE) {
            npg = get_mem(PAGE_SIZE, PAGE_SIZE);
        } else {
            npg = get_page0(0);
        }

        assert(npg != 0);
        assert((pa(npg) & ((1 << 12) - 1)) == 0);

        *pmle = pa(npg) | PTE_PRESENT | PTE_WRITE;
    }
}

void map_adr(virt_t vad, phys_t pad, int flags) {
    if (flags & USE_BIG_PAGE) {
        assert((vad & ((1 << (12 + 9)) - 1)) == 0);
        assert((pad & ((1 << (12 + 9)) - 1)) == 0);
    } else {
        assert((vad & ((1 << (12)) - 1)) == 0);
        assert((pad & ((1 << (12)) - 1)) == 0);
    }

    pte_t *pml4e = pml4 + pml4_i(vad);

    force_pte(pml4e, flags);

    pte_t *pdpte = ((pte_t *) va(pte_phys(*pml4e) << 12)) + pml3_i(vad);

    force_pte(pdpte, flags);

    pte_t *pde = ((pte_t *) va(pte_phys(*pdpte) << 12)) + pml2_i(vad);
    if (flags & USE_BIG_PAGE) {
        assert(pte_present(*pde) == false);
        *pde = pad | PTE_PRESENT | PTE_WRITE | PTE_LARGE;

        flush_tlb_addr(vad);
        return;
    }

    force_pte(pde, flags);

    pte_t *pte = ((pte_t *) va(pte_phys(*pde) << 12)) + pml1_i(vad);
    assert(pte_present(*pte) == false);
    *pte = pad | PTE_PRESENT | PTE_WRITE;

    if (!(flags & NOT_FLUSH_TLB)) {
        flush_tlb_addr(vad);
    }
}

phys_t get_phys_adr(virt_t vad) {
    phys_t pad;
    pte_t *pml4e = pml4 + pml4_i(vad);
    pte_t *pdpte = ((pte_t *) va(pte_phys(*pml4e) << 12)) + pml3_i(vad);
    pte_t *pde = ((pte_t *) va(pte_phys(*pdpte) << 12)) + pml2_i(vad);
    if (pte_large(*pde)) {
        pad = ((*pde & (~((1 << 21) - 1)))) | (vad & ((1 << 21) - 1));
        return pad;
    }

    pte_t *pte = ((pte_t *) va(pte_phys(*pde) << 12)) + pml1_i(vad);
    pad = ((*pte & (~((1 << 12) - 1)))) | (vad & ((1 << 12) - 1));;
    return pad;
}


void map_init() {
    pml4 = get_mem(PAGE_SIZE, PAGE_SIZE);

    for (phys_t i = 0;
         i < memory_map[memory_map_size - 1].base_addr + memory_map[memory_map_size - 1].length;
         i += (1 << 21)) {
        map_adr(i + HIGH_BASE, i, USE_BIG_PAGE | USE_BOOT_ALLOCATE | NOT_FLUSH_TLB);
        if (i + KERNEL_BASE > HIGH_BASE) {
            map_adr(i + KERNEL_BASE, i, USE_BIG_PAGE | USE_BOOT_ALLOCATE | NOT_FLUSH_TLB);
        }
    }

    puts("init page mapping");

    store_pml4(pa(pml4));
}
