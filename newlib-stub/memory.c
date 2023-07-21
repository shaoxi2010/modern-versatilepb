#if defined(FREERTOS)
#include "FreeRTOS.h"
#include "portmacro.h"
#include "semphr.h"
#include "task.h"
#endif
#include <assert.h>
#include <errno.h>
#include <malloc.h>
#include <reent.h>
#include <stdio.h>

#if defined(FREERTOS)
#if !defined(configUSE_NEWLIB_REENTRANT) || (configUSE_NEWLIB_REENTRANT != 1)
#warning "#define configUSE_NEWLIB_REENTRANT 1"
#endif
#if !defined(configUSE_RECURSIVE_MUTEXES) || (configUSE_RECURSIVE_MUTEXES != 1)
#warning "#define configUSE_RECURSIVE_MUTEXES 1"
#endif
#else
#define portENTER_CRITICAL()
#define portEXIT_CRITICAL()
#endif

static uint32_t heap_total_size;
static uint32_t heap_bytes_remaining;

//
// _sbrk implementation
//

extern char _end[];

#define __HeapBase	(_end)
#define __HeapLimit (_end + (4 << 20))

static char *current_heap_end = __HeapBase;

void *_sbrk_r(struct _reent *pReent, int incr)
{
	if (heap_total_size == 0) {
		heap_total_size = heap_bytes_remaining = __HeapLimit - __HeapBase;
	};
	portENTER_CRITICAL();
	char *previous_heap_end = current_heap_end;
	if (current_heap_end + incr > __HeapLimit) {
		portEXIT_CRITICAL();
		pReent->_errno = ENOMEM;
		return (char *)-1; // the malloc-family routine that called sbrk will
						   // return 0
	}
	// 'incr' of memory is available: update accounting and return it.
	current_heap_end += incr;
	heap_bytes_remaining -= incr;
	portEXIT_CRITICAL();
	return (char *)previous_heap_end;
}

void *sbrk(int incr) { return _sbrk_r(_impure_ptr, incr); }

void *_sbrk(int incr) { return sbrk(incr); };

//
// malloc_[un]lock implementation
//

#if defined (FREERTOS)
static SemaphoreHandle_t malloc_mutex = NULL;
static bool malloc_mutex_in_progress = false;
#endif

/* malloc 要求锁机制为递归锁，非rtos不再做任何检查自己小心 */
void __malloc_lock(struct _reent *r)
{
#if defined(FREERTOS)
	if (malloc_mutex_in_progress)
		return ;
	if (malloc_mutex == NULL) {
        malloc_mutex_in_progress = true;
		malloc_mutex = xSemaphoreCreateRecursiveMutex();
		malloc_mutex_in_progress = false;
	}
	if (portTHREADMODE() && xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
		xSemaphoreTakeRecursive(malloc_mutex, portMAX_DELAY);
	}
#endif
};

void __malloc_unlock(struct _reent *r)
{
#if defined(FREERTOS)
	if (malloc_mutex_in_progress)
		return;
	if (portTHREADMODE() && xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
		xSemaphoreGiveRecursive(malloc_mutex);
	}
#endif
};
