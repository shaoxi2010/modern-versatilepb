/*
 *  newlib/newlib/libc/misc/init.c
 *  hardware_init_hook -> software_init_hook ->preinit_array -> _init ->
 * init_array -> main
 */
#include <mmu.h>
#include <exceptions.h>
#include <cpu.h>
#include <compiler.h>
#include <stdint.h>

void hardware_init_hook(void)
{
	struct mem_desc r6_mem_desc[] = {
		{0x00000000, 0xFFFFFFFF, 0x00000000,
		 RW_NCNB}, /* None cached for 4G memory */
		{0x00000000, 0x4000000 - 1, 0x00000000,
		 RW_CB}, /* 64M cached SDRAM memory */
	};
	hw_install_low_vector();
	hw_mmu_init(r6_mem_desc, sizeof(r6_mem_desc) / sizeof(r6_mem_desc[0]));
	hw_dcache_enable();
	hw_icache_enable();
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
	asm("mov lr, %0" ::"r"(lr)); //恢复lr
    asm("bx lr"); //返回,入口时blx传入
}
