#include "exceptions.h"
#include <compiler.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void dump_regs(const char *info, struct registers *regs)
{
	printf("%s:\n", info);
	printf("r00:0x%08" PRIx32 " r01:0x%08" PRIx32 " r02:0x%08" PRIx32
		   " r03:0x%08" PRIx32 "\n",
		   regs->r0, regs->r1, regs->r2, regs->r3);
	printf("r04:0x%08" PRIx32 " r05:0x%08" PRIx32 " r06:0x%08" PRIx32
		   " r07:0x%08" PRIx32 "\n",
		   regs->r4, regs->r5, regs->r6, regs->r7);
	printf("r08:0x%08" PRIx32 " r09:0x%08" PRIx32 " r10:0x%08" PRIx32 "\n",
		   regs->r8, regs->r9, regs->r10);
	printf("fp :0x%08" PRIx32 " ip :0x%08" PRIx32 "\n", regs->fp, regs->ip);
	printf("sp :0x%08" PRIx32 " lr :0x%08" PRIx32 " pc :0x%08" PRIx32 "\n",
		   regs->sp, regs->lr, regs->pc);
	printf("cpsr:0x%08" PRIx32 "\n", regs->cpsr);
}

WEAK void hal_undefine_handle(struct registers *regs)
{
	dump_regs("undefined instr", regs);
	__asm__ __volatile__("mcr p15, 0, %0, c7, c10, 4" : : "r"(0) : "memory");
}
WEAK void hal_swi_handle(struct registers *regs)
{
	dump_regs("swi", regs);
	__asm__ __volatile__("mcr p15, 0, %0, c7, c10, 4" : : "r"(0) : "memory");
}
WEAK void hal_prefectabort_handle(struct registers *regs)
{
	dump_regs("prefect abort", regs);
	__asm__ __volatile__("mcr p15, 0, %0, c7, c10, 4" : : "r"(0) : "memory");
}
WEAK void hal_dataabort_handle(struct registers *regs)
{
	dump_regs("data abort", regs);
	__asm__ __volatile__("mcr p15, 0, %0, c7, c10, 4" : : "r"(0) : "memory");
}
WEAK void hal_irq_handle(struct registers *regs) { dump_regs("irq", regs); }
WEAK void hal_fiq_handle(void) {}

extern char hal_vectors_start[];
extern char hal_vectors_end[];

#define HIGHVECTOR_MASK (uint32_t)(1 << 13)

void hw_install_high_vector()
{

	uint32_t bit = HIGHVECTOR_MASK;
	__asm__ volatile("mrc  p15,0,r0,c1,c0,0\n\t"
					 "orr  r0,r0,%0\n\t"
					 "mcr  p15,0,r0,c1,c0,0"
					 :
					 : "r"(bit)
					 : "memory");

	memcpy((void *)0xffff0000, hal_vectors_start,
		   hal_vectors_end - hal_vectors_start);
	__asm__ __volatile__("mcr p15, 0, %0, c7, c10, 4" : : "r"(0) : "memory");
}

void hw_install_low_vector()
{
	uint32_t bit = HIGHVECTOR_MASK;
	__asm__ volatile("mrc  p15,0,r0,c1,c0,0\n\t"
					 "bic  r0,r0,%0\n\t"
					 "mcr  p15,0,r0,c1,c0,0"
					 :
					 : "r"(bit)
					 : "memory");
	memcpy((void *)0x00000000, hal_vectors_start,
		   hal_vectors_end - hal_vectors_start);
	__asm__ __volatile__("mcr p15, 0, %0, c7, c10, 4" : : "r"(0) : "memory");
}