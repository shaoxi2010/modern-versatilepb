/*
 *  newlib/newlib/libc/misc/init.c
 *  hardware_init_hook -> software_init_hook ->preinit_array -> _init ->
 * init_array -> main
 */
#include "bsp.h"
#include "compiler.h"
#include "cpu.h"
#include "exceptions.h"
#include "interrupt.h"
#include "mmu.h"
#include "timer.h"
#include "uart.h"
#include <stdint.h>

void hardware_init_hook(void)
{
	struct mem_desc r6_mem_desc[] = {
		{0x00000000, 0x4000000 - 1, 0x00000000, 1},
		{0x10000000, 0x20000000 - 1, 0x10000000, 0},
		{0xffff0000, 0xffff1000 - 1, 0x00000000, 1},
        };
	hw_mmu_init(r6_mem_desc, sizeof(r6_mem_desc) / sizeof(r6_mem_desc[0]));
	hw_install_high_vector();
	hw_dcache_enable();
	hw_icache_enable();
	const uint8_t ctrs = timer_countersPerTimer();

	/* Disable IRQ triggering (may be reenabled after ISRs are properly set) */
	irq_disableIrqMode();

	/* Init the vectored interrupt controller */
	pic_init();

	/* Init all counters of all available timers */
	for (int i = 0; i < BSP_NR_TIMERS; ++i) {
		for (int j = 0; j < ctrs; ++j) {
			timer_init(i, j);
		}
	}

	/* Init all available UARTs */
	for (int i = 0; i < BSP_NR_UARTS; ++i) {
		uart_init(i);
	}
}

#define MODE_SYS 0x0000001F
#define FIQ_BIT	 0x00000040
#define IRQ_BIT	 0x00000080

static uint32_t lr = 0;
/*NAKED防止gcc生成不必要的栈，模式切换会导致lr失效，故需要保护*/
NAKED void software_init_hook(void)
{
	/* 切换运行模式到system mode*/
	asm("mov %0, lr" : "=r"(lr)); // 保存lr到内存
	asm("msr cpsr, %0" ::"i"(MODE_SYS | FIQ_BIT | IRQ_BIT));
	asm("mov lr, %0" ::"r"(lr)); // 恢复lr
	asm("bx lr");				 // 返回,入口时blx传入
}
