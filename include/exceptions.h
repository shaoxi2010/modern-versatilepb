#ifndef __EXCEPTIONS__
#define __EXCEPTIONS__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h> 

struct registers {
	uint32_t cpsr;
	uint32_t r0, r1, r2, r3;
	uint32_t r4, r5, r6, r7, r8;
	uint32_t r9;
	uint32_t r10;
	uint32_t fp, ip;
	uint32_t sp;
	uint32_t lr;
	uint32_t pc;
};

void hal_undefine_handle(struct registers *regs);
void hal_swi_handle(void);
void hal_prefectabort_handle(struct registers *regs);
void hal_dataabort_handle(struct registers *regs);
void hal_irq_handle(struct registers *regs);
void hal_fiq_handle(void);
void hw_install_low_vector();
void hw_install_high_vector();

#ifdef __cplusplus
}
#endif

#endif