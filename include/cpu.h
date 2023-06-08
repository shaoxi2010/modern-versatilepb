#ifndef __CPU_H__
#define __CPU_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void hw_icache_enable();
void hw_icache_disable();
uint32_t hw_icache_status();

void hw_dcache_enable();
void hw_dcache_disable();
uint32_t hw_dcache_status();

void hw_high_vector_enable();
void hw_high_vector_disable();
uint32_t hw_high_vector_status();

void hw_irq_enableIrqMode(void);
void hw_irq_disableIrqMode(void);

#ifdef __cplusplus
}
#endif

#endif