/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-02-08     RT-Thread    the first version
 */

#ifndef __MMU_H__
#define __MMU_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define CACHE_LINE_SIZE 32

#define DESC_SEC (0x2 | (1 << 4))
#define CB (3 << 2)		// cache_on, write_back
#define CNB (2 << 2)	// cache_on, write_through
#define NCB (1 << 2)	// cache_off,WR_BUF on
#define NCNB (0 << 2)	// cache_off,WR_BUF off
#define AP_RW (3 << 10) // supervisor=RW, user=RW
#define AP_RO (2 << 10) // supervisor=RW, user=RO

#define DOMAIN_FAULT (0x0)
#define DOMAIN_CHK (0x1)
#define DOMAIN_NOTCHK (0x3)
#define DOMAIN0 (0x0 << 5)
#define DOMAIN1 (0x1 << 5)

#define DOMAIN0_ATTR (DOMAIN_CHK << 0)
#define DOMAIN1_ATTR (DOMAIN_FAULT << 2)

#define RW_CB                                                                  \
	(AP_RW | DOMAIN0 | CB | DESC_SEC) /* Read/Write, cache, write back */
#define RW_CNB                                                                 \
	(AP_RW | DOMAIN0 | CNB | DESC_SEC) /* Read/Write, cache, write through */
#define RW_NCNB                                                                \
	(AP_RW | DOMAIN0 | NCNB |                                                  \
	 DESC_SEC) /* Read/Write without cache and write buffer */
#define RW_FAULT                                                               \
	(AP_RW | DOMAIN1 | NCNB |                                                  \
	 DESC_SEC) /* Read/Write without cache and write buffer */

struct mem_desc {
	uint32_t vaddr_start;
	uint32_t vaddr_end;
	uint32_t paddr_start;
	uint32_t attr;
};

void hw_mmu_init(struct mem_desc *mdesc, uint32_t size);
void hw_mmu_clean_invalidated_dcache(uint32_t buffer, uint32_t size);
void hw_mmu_clean_dcache(uint32_t buffer, uint32_t size);
void hw_mmu_invalidate_dcache(uint32_t buffer, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif