#include <stdint.h>

#define ICACHE_MASK (uint32_t)(1 << 12)
#define DCACHE_MASK (uint32_t)(1 << 2)
#define HIGHVECTOR_MASK (uint32_t)(1 << 13)

static inline uint32_t cp15_rd(void) {
	uint32_t i;

	__asm volatile("mrc p15, 0, %0, c1, c0, 0" : "=r"(i));
	return i;
}

static inline void cache_enable(uint32_t bit) {
	__asm volatile("mrc  p15,0,r0,c1,c0,0\n\t"
				   "orr  r0,r0,%0\n\t"
				   "mcr  p15,0,r0,c1,c0,0"
				   :
				   : "r"(bit)
				   : "memory");
}

static inline void cache_disable(uint32_t bit) {
	__asm volatile("mrc  p15,0,r0,c1,c0,0\n\t"
				   "bic  r0,r0,%0\n\t"
				   "mcr  p15,0,r0,c1,c0,0"
				   :
				   : "r"(bit)
				   : "memory");
}

/**
 * enable I-Cache
 *
 */
void hw_icache_enable() { cache_enable(ICACHE_MASK); }

/**
 * disable I-Cache
 *
 */
void hw_icache_disable() { cache_disable(ICACHE_MASK); }

/**
 * return the status of I-Cache
 *
 */
uint32_t hw_icache_status() { return (cp15_rd() & ICACHE_MASK); }

/**
 * enable D-Cache
 *
 */
void hw_dcache_enable() { cache_enable(DCACHE_MASK); }

/**
 * disable D-Cache
 *
 */
void hw_dcache_disable() { cache_disable(DCACHE_MASK); }

/**
 * return the status of D-Cache
 *
 */
uint32_t hw_dcache_status() { return (cp15_rd() & DCACHE_MASK); }

/**
 * enable high-vector
 *
 */
void hw_high_vector_enable() { cache_enable(HIGHVECTOR_MASK); }

/**
 * disable high-vector
 *
 */
void hw_high_vector_disable() { cache_disable(HIGHVECTOR_MASK); }

/**
 * return the status of high-vector
 *
 */
uint32_t hw_high_vector_status() { return (cp15_rd() & HIGHVECTOR_MASK); }

/**
 * Enable CPU's IRQ mode that handles IRQ interrupt requests.
 */
void hw_irq_enableIrqMode(void) {
	/*
	 * To enable IRQ mode, bit 7 of the Program Status Register (CSPR)
	 * must be cleared to 0. See pp. 2-15 to 2-17 of the DDI0222 for more
	 * details. The CSPR can only be accessed using assembler.
	 */

	__asm volatile("MRS r0, cpsr");		 /* Read in the CPSR register. */
	__asm volatile("BIC r0, r0, #0x80"); /* Clear bit 8, (0x80) -- Causes IRQs
											to be enabled. */
	__asm volatile("MSR cpsr_c, r0"); /* Write it back to the CPSR register */
}

/**
 * Disable CPU's IRQ and FIQ mode that handle IRQ interrupt requests.
 */
void hw_irq_disableIrqMode(void) {
	/*
	 * To disable IRQ mode, bit 7 of the Program Status Register (CSPR)
	 * must be set t1 0. See pp. 2-15 to 2-17 of the DDI0222 for more details.
	 * The CSPR can only be accessed using assembler.
	 */

	__asm volatile("MRS r0, cpsr");		 /* Read in the CPSR register. */
	__asm volatile("ORR r0, r0, #0xC0"); /* Disable IRQ and FIQ exceptions. */
	__asm volatile("MSR cpsr_c, r0"); /* Write it back to the CPSR register. */
}
