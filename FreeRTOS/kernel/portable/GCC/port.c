#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "bsp.h"
#include "cpu.h"
#include "exceptions.h"
#include "interrupt.h"
#include "portable.h"
#include "portmacro.h"
#include "projdefs.h"
#include "task.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// #define CONTEXT_DEBUG
// #define CRITICAL_DEBUG
/*
 * Task control block.  A task control block (TCB) is allocated for each task,
 * and stores task state information, including a pointer to the task's context
 * (the task's run time environment, including register values)
 */
typedef struct tskTaskControlBlock /* The old naming convention is used to
									  prevent breaking kernel aware debuggers.
									*/
{
	volatile StackType_t
		*pxTopOfStack; /*< Points to the location of the last item placed on the
						  tasks stack.  THIS MUST BE THE FIRST MEMBER OF THE TCB
						  STRUCT. */

#if (portUSING_MPU_WRAPPERS == 1)
	xMPU_SETTINGS xMPUSettings; /*< The MPU settings are defined as part of the
								   port layer.  THIS MUST BE THE SECOND MEMBER
								   OF THE TCB STRUCT. */
#endif

	ListItem_t xStateListItem; /*< The list that the state list item of a task
								  is reference from denotes the state of that
								  task (Ready, Blocked, Suspended ). */
	ListItem_t
		xEventListItem; /*< Used to reference a task from an event list. */
	UBaseType_t
		uxPriority; /*< The priority of the task.  0 is the lowest priority. */
	StackType_t *pxStack; /*< Points to the start of the stack. */
	char pcTaskName[ configMAX_TASK_NAME_LEN ]; /*< Descriptive name given to the task when created.  Facilitates debugging only. */ /*lint !e971 Unqualified char types are allowed for strings and single characters only. */

#if ((portSTACK_GROWTH > 0) || (configRECORD_STACK_HIGH_ADDRESS == 1))
	StackType_t
		*pxEndOfStack; /*< Points to the highest valid address for the stack. */
#endif

#if (portCRITICAL_NESTING_IN_TCB == 1)
	UBaseType_t uxCriticalNesting; /*< Holds the critical section nesting depth
									  for ports that do not maintain their own
									  count in the port layer. */
#endif

#if (configUSE_TRACE_FACILITY == 1)
	UBaseType_t
		uxTCBNumber; /*< Stores a number that increments each time a TCB is
						created.  It allows debuggers to determine when a task
						has been deleted and then recreated. */
	UBaseType_t uxTaskNumber; /*< Stores a number specifically for use by third
								 party trace code. */
#endif

#if (configUSE_MUTEXES == 1)
	UBaseType_t
		uxBasePriority; /*< The priority last assigned to the task - used by the
						   priority inheritance mechanism. */
	UBaseType_t uxMutexesHeld;
#endif

#if (configUSE_APPLICATION_TASK_TAG == 1)
	TaskHookFunction_t pxTaskTag;
#endif

#if (configNUM_THREAD_LOCAL_STORAGE_POINTERS > 0)
	void *pvThreadLocalStoragePointers[configNUM_THREAD_LOCAL_STORAGE_POINTERS];
#endif

#if (configGENERATE_RUN_TIME_STATS == 1)
	configRUN_TIME_COUNTER_TYPE
		ulRunTimeCounter; /*< Stores the amount of time the task has spent in
							 the Running state. */
#endif

#if ((configUSE_NEWLIB_REENTRANT == 1) ||                                      \
	 (configUSE_C_RUNTIME_TLS_SUPPORT == 1))
	configTLS_BLOCK_TYPE xTLSBlock; /*< Memory block used as Thread Local
									   Storage (TLS) Block for the task. */
#endif

#if (configUSE_TASK_NOTIFICATIONS == 1)
	volatile uint32_t ulNotifiedValue[configTASK_NOTIFICATION_ARRAY_ENTRIES];
	volatile uint8_t ucNotifyState[configTASK_NOTIFICATION_ARRAY_ENTRIES];
#endif

/* See the comments in FreeRTOS.h with the definition of
 * tskSTATIC_AND_DYNAMIC_ALLOCATION_POSSIBLE. */
#if (tskSTATIC_AND_DYNAMIC_ALLOCATION_POSSIBLE !=                              \
	 0) /*lint !e731 !e9029 Macro has been consolidated for readability        \
		   reasons. */
	uint8_t ucStaticallyAllocated; /*< Set to pdTRUE if the task is a statically
									  allocated to ensure no attempt is made to
									  free the memory. */
#endif

#if (INCLUDE_xTaskAbortDelay == 1)
	uint8_t ucDelayAborted;
#endif

#if (configUSE_POSIX_ERRNO == 1)
	int iTaskErrno;
#endif
} tskTCB;

/* The old tskTCB name is maintained above then typedefed to the new TCB_t name
 * below to enable the use of older kernel aware debuggers. */
typedef tskTCB TCB_t;

struct tskInfo {
	struct registers regs;
	uint32_t critical_nesting;
};

static int nesting = 0;

static inline void save_context(struct registers *regs)
{
	extern volatile TCB_t *volatile pxCurrentTCB;
	struct tskInfo *info;
	pxCurrentTCB->pxTopOfStack = (StackType_t *)regs->sp;
	info = (struct tskInfo *)((UBaseType_t)pxCurrentTCB->pxTopOfStack -
							  sizeof(*info));
	memcpy(&info->regs, regs, sizeof(struct registers));
	info->critical_nesting = nesting;
#ifdef CONTEXT_DEBUG
	printf("save_context %s(%08" PRIx32 " ) nest %d\n",
		   pxCurrentTCB->pcTaskName, info->regs.pc, nesting);
#if portSTACK_GROWTH < 0
	if ((UBaseType_t)pxCurrentTCB->pxTopOfStack -
			(UBaseType_t)pxCurrentTCB->pxStack <
		sizeof(*regs)) {
		for (;;)
			puts("Stack Overflow!!!\n");
	}
#endif
#endif
}

static inline void restore_context(struct registers *regs)
{
	extern volatile TCB_t *volatile pxCurrentTCB;
	struct tskInfo *info;
	info = (struct tskInfo *)((UBaseType_t)pxCurrentTCB->pxTopOfStack -
							  sizeof(*info));
	memcpy(regs, &info->regs, sizeof(struct registers));
	nesting = info->critical_nesting;
#ifdef CONTEXT_DEBUG
	printf("restore_context %s(%08" PRIx32 " ) nest %d\n",
		   pxCurrentTCB->pcTaskName, info->regs.pc, nesting);
	printf("stack: %p limit %p\n", pxCurrentTCB->pxTopOfStack,
		   pxCurrentTCB->pxStack);
#endif
}

#define PSR_MASK 0x0000001F
#define MODE_SYS 0x0000001F
#define MODE_USR 0x00000010
#define FIQ_BIT	 0x00000040
#define IRQ_BIT	 0x00000080

StackType_t *pxPortInitialiseStack(StackType_t *pxTopOfStack,
								   TaskFunction_t pxCode,
								   void *pvParameters) PRIVILEGED_FUNCTION
{
	struct tskInfo *info =
		(struct tskInfo *)((UBaseType_t)pxTopOfStack - sizeof(*info));
	memset(info, 0, sizeof(*info));
	info->regs.pc = (uint32_t)pxCode;
	info->regs.r0 = (uint32_t)pvParameters;
	info->regs.sp = (uint32_t)pxTopOfStack;
	info->regs.cpsr = MODE_SYS | FIQ_BIT;
	return pxTopOfStack;
}

static void timer_isr(void);
BaseType_t xPortStartScheduler(void)
{
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

	syscall(SCHED_START, NULL);
	return 0;
}

static struct registers main_state = {};

void vPortEndScheduler(void) { exit(EXIT_SUCCESS); }

void hal_swi_handle(struct registers *regs)
{
	switch (regs->r0) {
	case SCHED_START:
		memcpy(&main_state, regs, sizeof(*regs));
		vTaskSwitchContext();
		restore_context(regs);
		break;
	case SCHED_YIELD:
		save_context(regs);
		vTaskSwitchContext();
		restore_context(regs);
		break;
	case SCHED_STOP:
		memcpy(regs, &main_state, sizeof(*regs));
		break;
	default:
		printf("SWI call %08" PRIx32 " is not impl\n", regs->r0);
		break;
	}
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
	if (need_switch == pdFALSE)
		need_switch = isr_yeild;
	portMEMORY_BARRIER();
}

void syscall(int cmd, void *args) { asm("svc 0"); }

void hal_irq_handle(struct registers *regs)
{
	need_switch = pdFALSE;
	extern void _pic_IrqHandler(void);
	_pic_IrqHandler();
	portMEMORY_BARRIER();
	if (need_switch) {
		save_context(regs);
		/* Find the highest priority task that is ready to run. */
		vTaskSwitchContext();
		restore_context(regs);
	}
}

void enter_critical()
{
	uint32_t cpsr = 0;
	asm("mrs %0, cpsr" : "=r"(cpsr));
	switch (cpsr & PSR_MASK) {
	case MODE_SYS:
	case MODE_USR:
		if (!nesting)
			hw_irq_disableIrqMode();
		break;
	}
#ifdef CRITICAL_DEBUG
	extern volatile TCB_t *volatile pxCurrentTCB;
	uint32_t pc = 0;
	asm("mov %0, lr" : "=r"(pc));
	printf("enter_critical %s(%08" PRIx32 ") nest %d\n",
		   pxCurrentTCB->pcTaskName, pc, nesting);
#endif
	portMEMORY_BARRIER();
	nesting++;
}

void exit_critical()
{
	uint32_t cpsr = 0;

	nesting--;
	portMEMORY_BARRIER();
	asm("mrs %0, cpsr" : "=r"(cpsr));
	switch (cpsr & PSR_MASK) {
	case MODE_SYS:
	case MODE_USR:
		if (!nesting)
			hw_irq_enableIrqMode();
		break;
	}

#ifdef CRITICAL_DEBUG
	extern volatile TCB_t *volatile pxCurrentTCB;
	uint32_t pc = 0;
	asm("mov %0, lr" : "=r"(pc));
	printf("exit_critical %s(%08" PRIx32 ") nest %d\n",
		   pxCurrentTCB->pcTaskName, pc, nesting);
#endif
}