#ifndef PORTMACRO_H
#define PORTMACRO_H

#include "FreeRTOSConfig.h"
#include "cpu.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
/*-----------------------------------------------------------
 * Port specific definitions.
 *
 * The settings in this file configure FreeRTOS correctly for the
 * given hardware and compiler.
 *
 * These settings should not be altered.
 *-----------------------------------------------------------
 */

/* Type definitions. */
#define portCHAR	   char
#define portFLOAT	   float
#define portDOUBLE	   double
#define portLONG	   int32_t
#define portSHORT	   int16_t
#define portSTACK_TYPE uint8_t
#define portBASE_TYPE  portLONG

typedef portSTACK_TYPE StackType_t;
typedef int32_t BaseType_t;
typedef uint32_t UBaseType_t;

#if (configUSE_16_BIT_TICKS == 1)
typedef uint16_t TickType_t;
#define portMAX_DELAY (TickType_t)0xffff
#else
typedef uint32_t TickType_t;
#define portMAX_DELAY (TickType_t)0xffffffffUL
#endif
/*-----------------------------------------------------------*/
/* SYSCALLS */
enum {
	SCHED_START = 0,
	SCHED_YIELD,
	SCHED_STOP,
};

/* Architecture specifics. */
#define portARCH_NAME	   "arm9"
#define portSTACK_GROWTH   (-1)
#define portTICK_PERIOD_MS ((TickType_t)1000 / configTICK_RATE_HZ)
#define portBYTE_ALIGNMENT 4
#define portNOP()		   asm volatile("nop")
#define portYIELD()		   syscall(SCHED_YIELD, NULL)
// #define portYIELD()
#define portMEMORY_BARRIER()	   asm volatile("" ::: "memory")
#define portYIELD_FROM_ISR(higher) yield_in_isr(higher)
#define portENTER_CRITICAL()	   enter_critical()
#define portEXIT_CRITICAL()		   exit_critical()
#define portDISABLE_INTERRUPTS	   hw_irq_disableIrqMode
#define portTHREADMODE()		   get_arm_thread_mode()
/*-----------------------------------------------------------*/
void yield_in_isr(BaseType_t isr_yeild);
void syscall(int cmd, void *args);
void enter_critical();
void exit_critical();
bool get_arm_thread_mode();
/*-----------------------------------------------------------*/
/* Task function macros as described on the FreeRTOS.org WEB site. */
#define portTASK_FUNCTION_PROTO(vFunction, pvParameters)                       \
	void vFunction(void *pvParameters)
#define portTASK_FUNCTION(vFunction, pvParameters)                             \
	void vFunction(void *pvParameters)
#ifdef __cplusplus
}
#endif
#endif