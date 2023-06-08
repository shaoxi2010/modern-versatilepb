#include <mmu.h>

void mmu_setttbase(register uint32_t i) {
    register uint32_t value;

    /* Invalidates all TLBs.Domain access is selected as
     * client by configuring domain access register,
     * in that case access controlled by permission value
     * set by page table entry
     */
    value = 0;
    asm volatile("mcr p15, 0, %0, c8, c7, 0" ::"r"(value));

    value = 0x55555555;
    asm volatile("mcr p15, 0, %0, c3, c0, 0" ::"r"(value));

    asm volatile("mcr p15, 0, %0, c2, c0, 0" ::"r"(i));
}

void mmu_set_domain(register uint32_t i) {
    asm volatile("mcr p15,0, %0, c3, c0,  0" : : "r"(i));
}

void mmu_enable() {
    asm volatile("mrc p15, 0, r0, c1, c0, 0 \n"
                 "orr r0, r0, #0x1 \n"
                 "mcr p15, 0, r0, c1, c0, 0 \n" ::
                     : "r0");
}

void mmu_disable() {
    asm volatile("mrc p15, 0, r0, c1, c0, 0 \n"
                 "bic r0, r0, #0x1 \n"
                 "mcr p15, 0, r0, c1, c0, 0 \n" ::
                     : "r0");
}

void mmu_enable_icache() {
    asm volatile("mrc p15, 0, r0, c1, c0, 0 \n"
                 "orr r0, r0, #(1<<12) \n"
                 "mcr p15, 0, r0, c1, c0, 0 \n" ::
                     : "r0");
}

void mmu_enable_dcache() {
    asm volatile("mrc p15, 0, r0, c1, c0, 0 \n"
                 "orr r0, r0, #(1<<2) \n"
                 "mcr p15, 0, r0, c1, c0, 0 \n" ::
                     : "r0");
}

void mmu_disable_icache() {
    asm volatile("mrc p15, 0, r0, c1, c0, 0 \n"
                 "bic r0, r0, #(1<<12) \n"
                 "mcr p15, 0, r0, c1, c0, 0 \n" ::
                     : "r0");
}

void mmu_disable_dcache() {
    asm volatile("mrc p15, 0, r0, c1, c0, 0 \n"
                 "bic r0, r0, #(1<<2) \n"
                 "mcr p15, 0, r0, c1, c0, 0 \n" ::
                     : "r0");
}

void mmu_enable_alignfault() {
    asm volatile("mrc p15, 0, r0, c1, c0, 0 \n"
                 "orr r0, r0, #1 \n"
                 "mcr p15, 0, r0, c1, c0, 0 \n" ::
                     : "r0");
}

void mmu_disable_alignfault() {
    asm volatile("mrc p15, 0, r0, c1, c0, 0 \n"
                 "bic r0, r0, #1 \n"
                 "mcr p15, 0, r0, c1, c0, 0 \n" ::
                     : "r0");
}

void hw_mmu_clean_invalidated_cache_index(int index) {
    asm volatile("mcr p15, 0, %0, c7, c14, 2" : : "r"(index));
}

void mmu_clean_invalidated_dcache(uint32_t buffer, uint32_t size) {
    unsigned int ptr;

    ptr = buffer & ~(CACHE_LINE_SIZE - 1);

    while (ptr < buffer + size) {
        asm volatile("mcr p15, 0, %0, c7, c14, 1" : : "r"(ptr));

        ptr += CACHE_LINE_SIZE;
    }
}

void hw_mmu_clean_dcache(uint32_t buffer, uint32_t size) {
    unsigned int ptr;

    ptr = buffer & ~(CACHE_LINE_SIZE - 1);

    while (ptr < buffer + size) {
        asm volatile("mcr p15, 0, %0, c7, c10, 1" : : "r"(ptr));

        ptr += CACHE_LINE_SIZE;
    }
}

void hw_mmu_invalidate_dcache(uint32_t buffer, uint32_t size) {
    unsigned int ptr;

    ptr = buffer & ~(CACHE_LINE_SIZE - 1);

    while (ptr < buffer + size) {
        asm volatile("mcr p15, 0, %0, c7, c6, 1" : : "r"(ptr));

        ptr += CACHE_LINE_SIZE;
    }
}

void mmu_invalidate_tlb() {
    asm volatile("mcr p15, 0, %0, c8, c7, 0" : : "r"(0));
}

void mmu_invalidate_icache() {
    asm volatile("mcr p15, 0, %0, c7, c5, 0" : : "r"(0));
}

void mmu_invalidate_dcache_all() {
    asm volatile("mcr p15, 0, %0, c7, c6, 0" : : "r"(0));
}

static volatile uint32_t _page_table[4 * 1024]
    __attribute__((aligned(16 * 1024)));

void mmu_setmtt(uint32_t vaddrStart, uint32_t vaddrEnd, uint32_t paddrStart,
                uint32_t attr) {
    volatile uint32_t *pTT;
    volatile int nSec;
    int i = 0;
    pTT = (uint32_t *)_page_table + (vaddrStart >> 20);
    nSec = (vaddrEnd >> 20) - (vaddrStart >> 20);
    for (i = 0; i <= nSec; i++) {
        *pTT = attr | (((paddrStart >> 20) + i) << 20);
        pTT++;
    }
}

void hw_mmu_init(struct mem_desc *mdesc, uint32_t size) {
    /* disable I/D cache */
    mmu_disable_dcache();
    mmu_disable_icache();
    mmu_disable();
    mmu_invalidate_tlb();

    /* set page table */
    for (; size > 0; size--) {
        mmu_setmtt(mdesc->vaddr_start, mdesc->vaddr_end, mdesc->paddr_start,
                   mdesc->attr);
        mdesc++;
    }

    /* set MMU table address */
    mmu_setttbase((uint32_t)_page_table);

    /* enables MMU */
    mmu_enable();

    /* enable Instruction Cache */
    mmu_enable_icache();

    /* enable Data Cache */
    mmu_enable_dcache();

    mmu_invalidate_icache();
    mmu_invalidate_dcache_all();
}