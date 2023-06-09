#include <reent.h>
#include <stdio.h>

__attribute__((noreturn)) void _exit(int status)
{
	int reg0 = 0x18;	// angel_SWIreason_ReportException
	int reg1 = 0x20026; // ADP_Stopped_ApplicationExit


	asm("mov r0, %0 \n\t"   // angel_SWIreason_ReportException
		"mov r1, %1\n\t" // ADP_Stopped_ApplicationExit
		"svc 0x00123456\n\t"   // make semihosting call
		:
        :"r"(reg0), "r"(reg1)
		: "r0", "r1");

	while (1)
		;
}