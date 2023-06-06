#include <reent.h>
#include <stdio.h>

__attribute__((noreturn)) void _exit(int status)
{
	register int reg0 asm("r0") __attribute__((unused));
	register int reg1 asm("r1") __attribute__((unused));

	reg0 = 0x18;	// angel_SWIreason_ReportException
	reg1 = 0x20026; // ADP_Stopped_ApplicationExit

	asm("svc 0x00123456"); // make semihosting call
}