#include <exceptions.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <compiler.h>

void dump_regs(struct registers *regs)
{
	printf("Execption:\n");
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
	printf("undefined instr\n");
	dump_regs(regs);
	__asm__ __volatile__("mcr p15, 0, %0, c7, c10, 4" : : "r"(0) : "memory");
}
WEAK void hal_swi_handle(void) { printf("swi\n"); }
WEAK void hal_prefectabort_handle(struct registers *regs)
{
	printf("prefect abort\n");
	dump_regs(regs);
	__asm__ __volatile__("mcr p15, 0, %0, c7, c10, 4" : : "r"(0) : "memory");
}
WEAK void hal_dataabort_handle(struct registers *regs)
{
	printf("data abort\n");
	dump_regs(regs);
	__asm__ __volatile__("mcr p15, 0, %0, c7, c10, 4" : : "r"(0) : "memory");
}
WEAK void hal_irq_handle(struct registers *regs) { dump_regs(regs); }
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