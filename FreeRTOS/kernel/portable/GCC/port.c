#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "bsp.h"
#include "exceptions.h"
#include "interrupt.h"
#include "portable.h"
#include "portmacro.h"
#include "projdefs.h"
#include "task.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MODE_SYS 0x0000001F
#define FIQ_BIT	 0x00000040
#define IRQ_BIT	 0x00000080

StackType_t *pxPortInitialiseStack(StackType_t *pxTopOfStack,
								   TaskFunction_t pxCode,
								   void *pvParameters) PRIVILEGED_FUNCTION
{
	struct registers *regs = (struct registers *)(pxTopOfStack - sizeof(*regs));
	memset(regs, 0, sizeof(*regs));
	regs->pc = (uint32_t)pxCode;
	regs->r0 = (uint32_t)pvParameters;
	regs->sp = (uint32_t)pxTopOfStack;
	regs->cpsr = MODE_SYS ;
	return pxTopOfStack;
}

static void timer_isr(void);
BaseType_t xPortStartScheduler(void)
{
    puts(__FUNCTION__);
	uint32_t ulCompareMatch;
	/* Calculate the match value required for our desired tick rate. */
	ulCompareMatch =
		(0 != configTICK_RATE_HZ ? configCPU_CLOCK_HZ / configTICK_RATE_HZ
								 : (uint32_t)(-1));

	/* Counter's load should always be greater than 0 */
	if (0 == ulCompareMatch) {
		ulCompareMatch = 1;
	}

	const uint8_t irqs[BSP_NR_TIMERS] = BSP_TIMER_IRQS;
	const uint8_t irq = irqs[0];

	timer_init(0, 0);
	timer_setLoad(0, 0, ulCompareMatch);
	timer_enableInterrupt(0, 0);
	pic_enableInterrupt(irq);
	timer_start(0, 0);
	pic_registerIrq(irq, &timer_isr, PIC_MAX_PRIORITY);

	portYIELD();
	return 0;
}

void vPortEndScheduler(void) { exit(EXIT_SUCCESS); }

void hal_swi_handle(struct registers *regs)
{
	extern volatile char **volatile pxCurrentTCB;
	struct registers *tcb_regs;
	tcb_regs = (struct registers *)(*pxCurrentTCB - sizeof(*regs));
	memcpy(tcb_regs, regs, sizeof(struct registers));
	/* Find the highest priority task that is ready to run. */
	vTaskSwitchContext();
	/* replace */
	tcb_regs = (struct registers *)(*pxCurrentTCB - sizeof(*regs));
	memcpy(regs, tcb_regs, sizeof(struct registers));
	printf("switch by yield %x\n",tcb_regs->pc);
}

static void timer_isr(void)
{
	/* Increment the tick count - this may wake a task. */
	if (xTaskIncrementTick() != pdFALSE) {
		/* Find the highest priority task that is ready to run. */
		yield_in_isr(pdTRUE);
	}
	timer_clearInterrupt(0, 0);
}

static BaseType_t need_switch = pdFALSE;

void yield_in_isr(BaseType_t isr_yeild)
{
	need_switch = isr_yeild;
	portMEMORY_BARRIER();
}

void hal_irq_handle(struct registers *regs)
{
	need_switch = pdFALSE;
	extern void _pic_IrqHandler(void);
	_pic_IrqHandler();
	if (need_switch) {
		extern volatile char **volatile pxCurrentTCB;
		struct registers *tcb_regs;
		tcb_regs = (struct registers *)(*pxCurrentTCB - sizeof(*regs));
		memcpy(tcb_regs, regs, sizeof(struct registers));
		/* Find the highest priority task that is ready to run. */
		vTaskSwitchContext();
		/* replace */
		tcb_regs = (struct registers *)(*pxCurrentTCB - sizeof(*regs));
		memcpy(regs, tcb_regs, sizeof(struct registers));
        printf("switch by isr %x\n", tcb_regs->pc);
	}
}