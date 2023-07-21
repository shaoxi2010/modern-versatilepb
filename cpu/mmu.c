#include "mmu.h"
#include "compiler.h"
#include <assert.h>
#include <malloc.h>
#include <sys/param.h>

void mmu_setttbase(uint32_t i)
{
	uint32_t value;

	/* Invalidates all TLBs.Domain access is selected as
	 * client by configuring domain access ,
	 * in that case access controlled by permission value
	 * set by page table entry
	 */
	value = 0;
	asm volatile("mcr p15, 0, %0, c8, c7, 0" ::"r"(value));

	value = 0x55555555;
	asm volatile("mcr p15, 0, %0, c3, c0, 0" ::"r"(value));

	asm volatile("mcr p15, 0, %0, c2, c0, 0" ::"r"(i));
}

void mmu_set_domain(uint32_t i)
{
	asm volatile("mcr p15,0, %0, c3, c0,  0" : : "r"(i));
}

void mmu_enable()
{
	asm volatile("mrc p15, 0, r0, c1, c0, 0 \n"
				 "orr r0, r0, #0x1 \n"
				 "mcr p15, 0, r0, c1, c0, 0 \n" ::
					 : "r0");
}

void mmu_disable()
{
	asm volatile("mrc p15, 0, r0, c1, c0, 0 \n"
				 "bic r0, r0, #0x1 \n"
				 "mcr p15, 0, r0, c1, c0, 0 \n" ::
					 : "r0");
}

void mmu_enable_icache()
{
	asm volatile("mrc p15, 0, r0, c1, c0, 0 \n"
				 "orr r0, r0, #(1<<12) \n"
				 "mcr p15, 0, r0, c1, c0, 0 \n" ::
					 : "r0");
}

void mmu_enable_dcache()
{
	asm volatile("mrc p15, 0, r0, c1, c0, 0 \n"
				 "orr r0, r0, #(1<<2) \n"
				 "mcr p15, 0, r0, c1, c0, 0 \n" ::
					 : "r0");
}

void mmu_disable_icache()
{
	asm volatile("mrc p15, 0, r0, c1, c0, 0 \n"
				 "bic r0, r0, #(1<<12) \n"
				 "mcr p15, 0, r0, c1, c0, 0 \n" ::
					 : "r0");
}

void mmu_disable_dcache()
{
	asm volatile("mrc p15, 0, r0, c1, c0, 0 \n"
				 "bic r0, r0, #(1<<2) \n"
				 "mcr p15, 0, r0, c1, c0, 0 \n" ::
					 : "r0");
}

void mmu_enable_alignfault()
{
	asm volatile("mrc p15, 0, r0, c1, c0, 0 \n"
				 "orr r0, r0, #1 \n"
				 "mcr p15, 0, r0, c1, c0, 0 \n" ::
					 : "r0");
}

void mmu_disable_alignfault()
{
	asm volatile("mrc p15, 0, r0, c1, c0, 0 \n"
				 "bic r0, r0, #1 \n"
				 "mcr p15, 0, r0, c1, c0, 0 \n" ::
					 : "r0");
}

void hw_mmu_clean_invalidated_cache_index(int index)
{
	asm volatile("mcr p15, 0, %0, c7, c14, 2" : : "r"(index));
}

void mmu_clean_invalidated_dcache(uint32_t buffer, uint32_t size)
{
	unsigned int ptr;

	ptr = buffer & ~(CACHE_LINE_SIZE - 1);

	while (ptr < buffer + size) {
		asm volatile("mcr p15, 0, %0, c7, c14, 1" : : "r"(ptr));

		ptr += CACHE_LINE_SIZE;
	}
}

void hw_mmu_clean_dcache(uint32_t buffer, uint32_t size)
{
	unsigned int ptr;

	ptr = buffer & ~(CACHE_LINE_SIZE - 1);

	while (ptr < buffer + size) {
		asm volatile("mcr p15, 0, %0, c7, c10, 1" : : "r"(ptr));

		ptr += CACHE_LINE_SIZE;
	}
}

void hw_mmu_invalidate_dcache(uint32_t buffer, uint32_t size)
{
	unsigned int ptr;

	ptr = buffer & ~(CACHE_LINE_SIZE - 1);

	while (ptr < buffer + size) {
		asm volatile("mcr p15, 0, %0, c7, c6, 1" : : "r"(ptr));

		ptr += CACHE_LINE_SIZE;
	}
}

void mmu_invalidate_tlb()
{
	asm volatile("mcr p15, 0, %0, c8, c7, 0" : : "r"(0));
}

void mmu_invalidate_icache()
{
	asm volatile("mcr p15, 0, %0, c7, c5, 0" : : "r"(0));
}

void mmu_invalidate_dcache_all()
{
	asm volatile("mcr p15, 0, %0, c7, c6, 0" : : "r"(0));
}

/* 4G line */
ALIGNED(MMU_DESCRIPTOR_L1_SMALL_ENTRY_NUMBERS)
static volatile uint8_t _page_table[MMU_DESCRIPTOR_L1_SMALL_ENTRY_NUMBERS] = {};

/* arm9 没有控制运行属性的值也没有APX和TEX */
#define MMU_DESCRIPTOR_L1_SMALL (MMU_DESCRIPTOR_L1_TYPE_PAGE_TABLE)
#define MMU_DESCRIPTOR_L2_4K_NCNB                                              \
	(MMU_DESCRIPTOR_L2_TYPE_SMALL_PAGE |                                       \
	 MMU_DESCRIPTOR_L2_TYPE_STRONGLY_ORDERED | MMU_DESCRIPTOR_L2_AP_P_RW_U_RW)
#define MMU_DESCRIPTOR_L2_4K_CB                                                \
	(MMU_DESCRIPTOR_L2_TYPE_SMALL_PAGE |                                       \
	 MMU_DESCRIPTOR_L2_TYPE_NORMAL_WRITE_BACK_ALLOCATE |                       \
	 MMU_DESCRIPTOR_L2_AP_P_RW_U_RW)
#define MMU_DESCRIPTOR_L1_1M_NCNB                                              \
	(MMU_DESCRIPTOR_L1_TYPE_SECTION | MMU_DESCRIPTOR_L1_AP_P_RW_U_RW |         \
	 MMU_DESCRIPTOR_L1_TYPE_STRONGLY_ORDERED)
#define MMU_DESCRIPTOR_L1_1M_CB                                                \
	(MMU_DESCRIPTOR_L1_TYPE_SECTION | MMU_DESCRIPTOR_L1_AP_P_RW_U_RW |         \
	 MMU_DESCRIPTOR_L1_TYPE_NORMAL_WRITE_BACK_ALLOCATE)

static uint32_t mapsection(uint32_t vaddr, uint32_t paddr, uint32_t count,
						   uint32_t cached)
{
	uint32_t *l1_entry = (uint32_t *)_page_table;
	uint32_t offset = vaddr >> 20;

	if (cached)
		l1_entry[offset] =
			MMU_DESCRIPTOR_L1_SECTION_ADDR(paddr) | MMU_DESCRIPTOR_L1_1M_CB;
	else
		l1_entry[offset] =
			MMU_DESCRIPTOR_L1_SECTION_ADDR(paddr) | MMU_DESCRIPTOR_L1_1M_NCNB;

	return MMU_DESCRIPTOR_L2_NUMBERS_PER_L1;
}

static uint32_t mapl2(uint32_t *l2_entry, uint32_t vaddr, uint32_t paddr,
					  uint32_t count, uint32_t cached)
{
	uint32_t offset = 0;
	uint32_t len = 0;
	vaddr = vaddr & MMU_DESCRIPTOR_L1_SMALL_MASK;
	offset = vaddr >> 12;
	len = MIN(MMU_DESCRIPTOR_L2_NUMBERS_PER_L1 - offset, count);
	for (int i = 0; i < len; i++) {
		if (cached)
			l2_entry[offset + i] = MMU_DESCRIPTOR_L2_SMALL_PAGE_ADDR(paddr) |
								   MMU_DESCRIPTOR_L2_4K_CB;
		else
			l2_entry[offset + i] = MMU_DESCRIPTOR_L2_SMALL_PAGE_ADDR(paddr) |
								   MMU_DESCRIPTOR_L2_4K_NCNB;
		paddr += MMU_DESCRIPTOR_L2_SMALL_SIZE;
	}

	return count;
}

int mmu_map(uint32_t vaddr, uint32_t paddr, uint32_t count, uint32_t cached)
{
	uint32_t saveCounts = 0;
	while (count > 0) {
		if (MMU_DESCRIPTOR_IS_L1_SIZE_ALIGNED(vaddr) &&
			MMU_DESCRIPTOR_IS_L1_SIZE_ALIGNED(paddr) &&
			count >= MMU_DESCRIPTOR_L2_NUMBERS_PER_L1) {
			saveCounts = mapsection(vaddr, paddr, count, cached);
		} else {
			uint32_t *l2_entry = (uint32_t *)memalign(1024, 1024);
			assert(l2_entry);
			saveCounts = mapl2(l2_entry, vaddr, paddr, count, cached);
			uint32_t *l1_entry = (uint32_t *)_page_table;
			uint32_t offset = vaddr >> 20;
			l1_entry[offset] =
				MMU_DESCRIPTOR_L1_PAGE_TABLE_ADDR((uint32_t)l2_entry) |
				MMU_DESCRIPTOR_L1_SMALL;
		}
		vaddr += MMU_DESCRIPTOR_L2_SMALL_SIZE * saveCounts;
		paddr += MMU_DESCRIPTOR_L2_SMALL_SIZE * saveCounts;
		count -= saveCounts;
	}
	return 0;
}

void hw_mmu_init(struct mem_desc *mdesc, uint32_t size)
{
	/* disable I/D cache */
	mmu_disable_dcache();
	mmu_disable_icache();
	mmu_disable();
	mmu_invalidate_tlb();

	/* set page table */
	for (; size > 0; size--) {
		uint32_t count = (mdesc->vaddr_end - mdesc->vaddr_start + 1) /
						 MMU_DESCRIPTOR_L2_SMALL_SIZE;
		mmu_map(mdesc->vaddr_start, mdesc->paddr_start, count, mdesc->cached);
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