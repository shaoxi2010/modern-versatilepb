/*
 *  newlib/newlib/libc/misc/init.c
 *  hardware_init_hook -> softwae_init_hook ->preinit_array -> _init -> init_array -> main
 */
#include <mmu.h>
#include <exceptions.h>
#include <cpu.h>
#include <compiler.h>

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
