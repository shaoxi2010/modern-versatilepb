#include "bsp.h"
#include "interrupt.h"
#include "timer.h"
#include <stdio.h>

static bool triged = false;

void timer_isr(void){
    printf("timer isr enter\n");
    triged = true;
    timer_clearInterrupt(0, 0);
}

int main()
{
	for (int i = 1; i < 10; i++) {
		if (i % 3 == 0)
			printf("%d is divisible by 3\n", i);
		if (i % 11 == 0)
			printf("%d is divisible by 11\n", i);
	}

	const uint8_t irqs[BSP_NR_TIMERS] = BSP_TIMER_IRQS;
	const uint8_t irq = irqs[0];

	timer_init(0, 0);
	timer_setLoad(0, 0, 1000);
	timer_enableInterrupt(0, 0);
	pic_enableInterrupt(irq);
	timer_start(0, 0);
	pic_registerIrq(irq, &timer_isr, PIC_MAX_PRIORITY);
	irq_enableIrqMode();

    while (!triged) {
        asm("nop");
    }
    
    printf("exit!!!\n");
	return 0;
}