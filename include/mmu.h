#ifndef __MMU_H__
#define __MMU_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define CACHE_LINE_SIZE					  32

#define IS_ALIGNED(a, b)				  (!(((uint32_t)(a)) & (((uint32_t)(b)) - 1)))

#define MMU_DESCRIPTOR_TEX_0			  0
#define MMU_DESCRIPTOR_TEX_1			  1
#define MMU_DESCRIPTOR_TEX_2			  2
#define MMU_DESCRIPTOR_TEX_MASK			  7

#define MMU_DESCRIPTOR_CACHE_BUFFER_SHIFT 2
#define MMU_DESCRIPTOR_CACHE_BUFFER(x)                                         \
	((x) << MMU_DESCRIPTOR_CACHE_BUFFER_SHIFT)
#define MMU_DESCRIPTOR_NON_CACHEABLE			 MMU_DESCRIPTOR_CACHE_BUFFER(0)
#define MMU_DESCRIPTOR_WRITE_BACK_ALLOCATE		 MMU_DESCRIPTOR_CACHE_BUFFER(1)
#define MMU_DESCRIPTOR_WRITE_THROUGH_NO_ALLOCATE MMU_DESCRIPTOR_CACHE_BUFFER(2)
#define MMU_DESCRIPTOR_WRITE_BACK_NO_ALLOCATE	 MMU_DESCRIPTOR_CACHE_BUFFER(3)

/* user space mmu access permission define begin */
#define MMU_DESCRIPTOR_DOMAIN_MANAGER 0
#define MMU_DESCRIPTOR_DOMAIN_CLIENT  1
#define MMU_DESCRIPTOR_DOMAIN_NA	  2

/* L1 descriptor type */
#define MMU_DESCRIPTOR_L1_TYPE_INVALID	  (0x0 << 0)
#define MMU_DESCRIPTOR_L1_TYPE_PAGE_TABLE (0x1 << 0) ///< 一级条目类型按页分
#define MMU_DESCRIPTOR_L1_TYPE_SECTION	  (0x2 << 0) ///< 1MB 一级条目类型按段分
#define MMU_DESCRIPTOR_L1_TYPE_MASK		  (0x3 << 0)

/* L2 descriptor type */
#define MMU_DESCRIPTOR_L2_TYPE_INVALID (0x0 << 0)
#define MMU_DESCRIPTOR_L2_TYPE_LARGE_PAGE                                      \
	(0x1 << 0) ///< 64KB 二级条目类型按大页分
#define MMU_DESCRIPTOR_L2_TYPE_SMALL_PAGE                                      \
	(0x2 << 0) ///< 4KB 二级条目类型按小页分
#define MMU_DESCRIPTOR_L2_TYPE_SMALL_PAGE_XN                                   \
	(0x3 << 0) ///< 1KB 二级条目类型按极小页分
#define MMU_DESCRIPTOR_L2_TYPE_MASK (0x3 << 0)

#define MMU_DESCRIPTOR_IS_L1_SIZE_ALIGNED(x)                                   \
	IS_ALIGNED(x, MMU_DESCRIPTOR_L1_SMALL_SIZE)
#define MMU_DESCRIPTOR_L1_SMALL_SIZE			   0x100000 // 1M 页表L1大小
#define MMU_DESCRIPTOR_L1_SMALL_MASK			   (MMU_DESCRIPTOR_L1_SMALL_SIZE - 1)
#define MMU_DESCRIPTOR_L1_SMALL_FRAME			   (~MMU_DESCRIPTOR_L1_SMALL_MASK)
#define MMU_DESCRIPTOR_L1_SMALL_SHIFT			   20 // 移动位数
#define MMU_DESCRIPTOR_L1_SECTION_ADDR(x)		   ((x)&MMU_DESCRIPTOR_L1_SMALL_FRAME)
#define MMU_DESCRIPTOR_L1_PAGE_TABLE_ADDR(x)	   ((x) & ~((1 << 10) - 1))
#define MMU_DESCRIPTOR_L1_SMALL_L2_TABLES_PER_PAGE 4
#define MMU_DESCRIPTOR_L1_SMALL_ENTRY_NUMBERS                                  \
	0x4000U ///< 页表必须按16Kb对齐,因为C2寄存器低14位为0
#define MMU_DESCRIPTOR_L1_SMALL_DOMAIN_MASK                                    \
	(~(0x0f << 5)) /* 4k page section domain mask */
#define MMU_DESCRIPTOR_L1_SMALL_DOMAIN_CLIENT                                  \
	(MMU_DESCRIPTOR_DOMAIN_CLIENT << 5)

#define MMU_DESCRIPTOR_L1_PAGETABLE_NON_SECURE (1 << 3)
#define MMU_DESCRIPTOR_L1_SECTION_NON_SECURE   (1 << 19)
#define MMU_DESCRIPTOR_L1_SECTION_SHAREABLE	   (1 << 16)
#define MMU_DESCRIPTOR_L1_SECTION_NON_GLOBAL   (1 << 17)
#define MMU_DESCRIPTOR_L1_SECTION_XN		   (1 << 4)

/* TEX CB */
#define MMU_DESCRIPTOR_L1_TEX_SHIFT 12 /* type extension field shift */
#define MMU_DESCRIPTOR_L1_TEX(x)                                               \
	((x) << MMU_DESCRIPTOR_L1_TEX_SHIFT) /* type extension */
#define MMU_DESCRIPTOR_L1_TYPE_STRONGLY_ORDERED                                \
	(MMU_DESCRIPTOR_L1_TEX(MMU_DESCRIPTOR_TEX_0) | MMU_DESCRIPTOR_NON_CACHEABLE)
#define MMU_DESCRIPTOR_L1_TYPE_NORMAL_NOCACHE                                  \
	(MMU_DESCRIPTOR_L1_TEX(MMU_DESCRIPTOR_TEX_1) | MMU_DESCRIPTOR_NON_CACHEABLE)
#define MMU_DESCRIPTOR_L1_TYPE_DEVICE_SHARED                                   \
	(MMU_DESCRIPTOR_L1_TEX(MMU_DESCRIPTOR_TEX_0) |                             \
	 MMU_DESCRIPTOR_WRITE_BACK_ALLOCATE)
#define MMU_DESCRIPTOR_L1_TYPE_DEVICE_NON_SHARED                               \
	(MMU_DESCRIPTOR_L1_TEX(MMU_DESCRIPTOR_TEX_2) | MMU_DESCRIPTOR_NON_CACHEABLE)
#define MMU_DESCRIPTOR_L1_TYPE_NORMAL_WRITE_BACK_ALLOCATE                      \
	(MMU_DESCRIPTOR_L1_TEX(MMU_DESCRIPTOR_TEX_1) |                             \
	 MMU_DESCRIPTOR_WRITE_BACK_NO_ALLOCATE)
#define MMU_DESCRIPTOR_L1_TEX_TYPE_MASK                                        \
	(MMU_DESCRIPTOR_L1_TEX(MMU_DESCRIPTOR_TEX_MASK) |                          \
	 MMU_DESCRIPTOR_WRITE_BACK_NO_ALLOCATE)

#define MMU_DESCRIPTOR_L1_AP2_SHIFT	 15
#define MMU_DESCRIPTOR_L1_AP2(x)	 ((x) << MMU_DESCRIPTOR_L1_AP2_SHIFT)
#define MMU_DESCRIPTOR_L1_AP2_0		 (MMU_DESCRIPTOR_L1_AP2(0))
#define MMU_DESCRIPTOR_L1_AP2_1		 (MMU_DESCRIPTOR_L1_AP2(1))
#define MMU_DESCRIPTOR_L1_AP01_SHIFT 10
#define MMU_DESCRIPTOR_L1_AP01(x)	 ((x) << MMU_DESCRIPTOR_L1_AP01_SHIFT)
#define MMU_DESCRIPTOR_L1_AP01_0	 (MMU_DESCRIPTOR_L1_AP01(0))
#define MMU_DESCRIPTOR_L1_AP01_1	 (MMU_DESCRIPTOR_L1_AP01(1))
#define MMU_DESCRIPTOR_L1_AP01_3	 (MMU_DESCRIPTOR_L1_AP01(3))
#define MMU_DESCRIPTOR_L1_AP_P_NA_U_NA                                         \
	(MMU_DESCRIPTOR_L1_AP2_0 | MMU_DESCRIPTOR_L1_AP01_0)
#define MMU_DESCRIPTOR_L1_AP_P_RW_U_RW                                         \
	(MMU_DESCRIPTOR_L1_AP2_0 | MMU_DESCRIPTOR_L1_AP01_3)
#define MMU_DESCRIPTOR_L1_AP_P_RW_U_NA                                         \
	(MMU_DESCRIPTOR_L1_AP2_0 | MMU_DESCRIPTOR_L1_AP01_1)
#define MMU_DESCRIPTOR_L1_AP_P_RO_U_RO                                         \
	(MMU_DESCRIPTOR_L1_AP2_1 | MMU_DESCRIPTOR_L1_AP01_3)
#define MMU_DESCRIPTOR_L1_AP_P_RO_U_NA                                         \
	(MMU_DESCRIPTOR_L1_AP2_1 | MMU_DESCRIPTOR_L1_AP01_1)
#define MMU_DESCRIPTOR_L1_AP_MASK                                              \
	(MMU_DESCRIPTOR_L1_AP2_1 | MMU_DESCRIPTOR_L1_AP01_3)

#define MMU_DESCRIPTOR_L2_SMALL_SIZE  0x1000 // L2 小页大小 4K
#define MMU_DESCRIPTOR_L2_SMALL_MASK  (MMU_DESCRIPTOR_L2_SMALL_SIZE - 1)
#define MMU_DESCRIPTOR_L2_SMALL_FRAME (~MMU_DESCRIPTOR_L2_SMALL_MASK)
#define MMU_DESCRIPTOR_L2_SMALL_SHIFT 12 // 小页偏移 12位
#define MMU_DESCRIPTOR_L2_NUMBERS_PER_L1                                       \
	(MMU_DESCRIPTOR_L1_SMALL_SIZE >> MMU_DESCRIPTOR_L2_SMALL_SHIFT)
#define MMU_DESCRIPTOR_IS_L2_SIZE_ALIGNED(x)                                   \
	IS_ALIGNED(x, MMU_DESCRIPTOR_L2_SMALL_SIZE)
#define MMU_DESCRIPTOR_L2_TEX_SHIFT 6 /* type extension field shift */
#define MMU_DESCRIPTOR_L2_TEX(x)                                               \
	((x) << MMU_DESCRIPTOR_L2_TEX_SHIFT) /* type extension */
#define MMU_DESCRIPTOR_L2_TYPE_STRONGLY_ORDERED                                \
	(MMU_DESCRIPTOR_L2_TEX(MMU_DESCRIPTOR_TEX_0) | MMU_DESCRIPTOR_NON_CACHEABLE)
#define MMU_DESCRIPTOR_L2_TYPE_NORMAL_NOCACHE                                  \
	(MMU_DESCRIPTOR_L2_TEX(MMU_DESCRIPTOR_TEX_1) | MMU_DESCRIPTOR_NON_CACHEABLE)
#define MMU_DESCRIPTOR_L2_TYPE_DEVICE_SHARED                                   \
	(MMU_DESCRIPTOR_L2_TEX(MMU_DESCRIPTOR_TEX_0) |                             \
	 MMU_DESCRIPTOR_WRITE_BACK_ALLOCATE)
#define MMU_DESCRIPTOR_L2_TYPE_DEVICE_NON_SHARED                               \
	(MMU_DESCRIPTOR_L2_TEX(MMU_DESCRIPTOR_TEX_2) | MMU_DESCRIPTOR_NON_CACHEABLE)
#define MMU_DESCRIPTOR_L2_TYPE_NORMAL_WRITE_BACK_ALLOCATE                      \
	(MMU_DESCRIPTOR_L2_TEX(MMU_DESCRIPTOR_TEX_1) |                             \
	 MMU_DESCRIPTOR_WRITE_BACK_NO_ALLOCATE)
#define MMU_DESCRIPTOR_L2_TEX_TYPE_MASK                                        \
	(MMU_DESCRIPTOR_L2_TEX(MMU_DESCRIPTOR_TEX_MASK) |                          \
	 MMU_DESCRIPTOR_WRITE_BACK_NO_ALLOCATE)
#define MMU_DESCRIPTOR_L2_AP2_SHIFT	 9
#define MMU_DESCRIPTOR_L2_AP2(x)	 ((x) << MMU_DESCRIPTOR_L2_AP2_SHIFT)
#define MMU_DESCRIPTOR_L2_AP2_0		 (MMU_DESCRIPTOR_L2_AP2(0))
#define MMU_DESCRIPTOR_L2_AP2_1		 (MMU_DESCRIPTOR_L2_AP2(1))
#define MMU_DESCRIPTOR_L2_AP01_SHIFT 4
#define MMU_DESCRIPTOR_L2_AP01(x)	 ((x) << MMU_DESCRIPTOR_L2_AP01_SHIFT)
#define MMU_DESCRIPTOR_L2_AP01_0	 (MMU_DESCRIPTOR_L2_AP01(0))
#define MMU_DESCRIPTOR_L2_AP01_1	 (MMU_DESCRIPTOR_L2_AP01(1))
#define MMU_DESCRIPTOR_L2_AP01_3	 (MMU_DESCRIPTOR_L2_AP01(3))
#define MMU_DESCRIPTOR_L2_AP_P_NA_U_NA                                         \
	(MMU_DESCRIPTOR_L2_AP2_0 | MMU_DESCRIPTOR_L2_AP01_0)
#define MMU_DESCRIPTOR_L2_AP_P_RW_U_RW                                         \
	(MMU_DESCRIPTOR_L2_AP2_0 | MMU_DESCRIPTOR_L2_AP01_3)
#define MMU_DESCRIPTOR_L2_AP_P_RW_U_NA                                         \
	(MMU_DESCRIPTOR_L2_AP2_0 | MMU_DESCRIPTOR_L2_AP01_1)
#define MMU_DESCRIPTOR_L2_AP_P_RO_U_RO                                         \
	(MMU_DESCRIPTOR_L2_AP2_1 | MMU_DESCRIPTOR_L2_AP01_3)
#define MMU_DESCRIPTOR_L2_AP_P_RO_U_NA                                         \
	(MMU_DESCRIPTOR_L2_AP2_1 | MMU_DESCRIPTOR_L2_AP01_1)
#define MMU_DESCRIPTOR_L2_AP_MASK                                              \
	(MMU_DESCRIPTOR_L2_AP2_1 | MMU_DESCRIPTOR_L2_AP01_3)

#define MMU_DESCRIPTOR_L2_SHAREABLE				(1 << 10)
#define MMU_DESCRIPTOR_L2_NON_GLOBAL			(1 << 11)
#define MMU_DESCRIPTOR_L2_SMALL_PAGE_ADDR(x)	((x)&MMU_DESCRIPTOR_L2_SMALL_FRAME)

#define MMU_DESCRIPTOR_TTBCR_PD0				(1 << 4)
#define MMU_DESCRIPTOR_TTBR_WRITE_BACK_ALLOCATE 1
#define MMU_DESCRIPTOR_TTBR_RGN(x)				(((x)&0x3) << 3)
#define MMU_DESCRIPTOR_TTBR_IRGN(x)                                            \
	((((x)&0x1) << 6) | ((((x) >> 1) & 0x1) << 0))
#define MMU_DESCRIPTOR_TTBR_S	(1 << 1)
#define MMU_DESCRIPTOR_TTBR_NOS (1 << 5)


struct mem_desc {
	uint32_t vaddr_start;
	uint32_t vaddr_end;
	uint32_t paddr_start;
    uint32_t cached;
};

void hw_mmu_init(struct mem_desc *mdesc, uint32_t size);
void hw_mmu_clean_invalidated_dcache(uint32_t buffer, uint32_t size);
void hw_mmu_clean_dcache(uint32_t buffer, uint32_t size);
void hw_mmu_invalidate_dcache(uint32_t buffer, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif