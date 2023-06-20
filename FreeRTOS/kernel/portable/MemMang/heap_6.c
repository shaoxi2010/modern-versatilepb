// Simplified and customized version based on a code from article below.
// http://www.nadler.com/embedded/newlibAndFreeRTOS.html
#include "FreeRTOS.h"
#include "portmacro.h"
#include "task.h"
#include <errno.h>
#include <malloc.h>
#include <reent.h>
#include <stdio.h>

#if !defined(configUSE_NEWLIB_REENTRANT) || (configUSE_NEWLIB_REENTRANT != 1)
#warning "#define configUSE_NEWLIB_REENTRANT 1"
#endif

static uint32_t heap_total_size;
static uint32_t heap_bytes_remaining;

//
// FreeRTOS memory API
//

void *pvPortMalloc(size_t xSize) PRIVILEGED_FUNCTION { return malloc(xSize); }

void vPortFree(void *pv) PRIVILEGED_FUNCTION { free(pv); };

// error handing
void vApplicationMallocFailedHook() { printf("Could not alloc memory!!!\r\n"); }

size_t xPortGetFreeHeapSize(void) PRIVILEGED_FUNCTION
{
	/* available space now managed by newlib */
	struct mallinfo mi = mallinfo();
	/* plus space not yet handed to newlib by sbrk */
	return mi.fordblks + heap_bytes_remaining;
}

void vPortInitialiseBlocks(void) PRIVILEGED_FUNCTION{};

//
// _sbrk implementation
//

#define __HeapBase	((char *)(configTOTAL_HEAP_START))
#define __HeapLimit ((char *)(configTOTAL_HEAP_SIZE + configTOTAL_HEAP_START))

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
		vApplicationMallocFailedHook();
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

static int malloc_lock_counter = 0;

void __malloc_lock(struct _reent *r)
{
	if (malloc_lock_counter == 0) {
		portENTER_CRITICAL();
	}
    portMEMORY_BARRIER();
	malloc_lock_counter += 1;
};

void __malloc_unlock(struct _reent *r)
{
	malloc_lock_counter -= 1;
	portMEMORY_BARRIER();
	if (malloc_lock_counter == 0)
		portEXIT_CRITICAL();
};

uint32_t mem_is_heap_allocated(const void *ptr)
{
	return (ptr >= (void *)__HeapBase && ptr < (void *)__HeapLimit);
}