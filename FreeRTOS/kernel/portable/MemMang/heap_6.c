// Simplified and customized version based on a code from article below.
// http://www.nadler.com/embedded/newlibAndFreeRTOS.html
#include "FreeRTOS.h"
#include "task.h"
#include "compiler.h"
#include <errno.h>
#include <malloc.h>
#include <reent.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#if !defined(configUSE_NEWLIB_REENTRANT) || (configUSE_NEWLIB_REENTRANT != 1)
#warning "#define configUSE_NEWLIB_REENTRANT 1"
#endif

uint32_t heap_total_size;
uint32_t heap_bytes_remaining;

//
// FreeRTOS memory API
//

void *pvPortMalloc(size_t xSize) PRIVILEGED_FUNCTION
{
	void *p = malloc(xSize);
	return p;
}
void vPortFree(void *pv) PRIVILEGED_FUNCTION { free(pv); };

// error handing
void vApplicationMallocFailedHook() { printf("Could not alloc memory!!!\r\n"); }

size_t xPortGetFreeHeapSize(void) PRIVILEGED_FUNCTION
{
	struct mallinfo mi = mallinfo(); // available space now managed by newlib
	return mi.fordblks +
		   heap_bytes_remaining; // plus space not yet handed to newlib by sbrk
}

void vPortInitialiseBlocks(void) PRIVILEGED_FUNCTION{};

#define ENTER_CRITICAL_SECTION(_usis)                                          \
	{                                                                          \
		portENTER_CRITICAL();                                                  \
	} // Disables interrupts (after saving prior state)
#define EXIT_CRITICAL_SECTION(_usis)                                           \
	{                                                                          \
		portEXIT_CRITICAL();                                                   \
	} // Re-enables interrupts (unless already disabled prior
	  // taskENTER_CRITICAL)

//
// _sbrk implementation
//

#define __HeapBase	((char *)0x1000000)
#define __HeapLimit ((char *)0x2000000)

static char *current_heap_end = __HeapBase;

void *_sbrk_r(struct _reent *pReent, int incr)
{
	UNUSED UBaseType_t usis; // saved interrupt status

	if (heap_total_size == 0) {
		heap_total_size = heap_bytes_remaining = __HeapLimit - __HeapBase;
	};
	ENTER_CRITICAL_SECTION(usis);
	char *previous_heap_end = current_heap_end;
	if (current_heap_end + incr > __HeapLimit) {
		EXIT_CRITICAL_SECTION(usis);
		vApplicationMallocFailedHook();
		pReent->_errno = ENOMEM;
		return (char *)-1; // the malloc-family routine that called sbrk will
						   // return 0
	}
	// 'incr' of memory is available: update accounting and return it.
	current_heap_end += incr;
	heap_bytes_remaining -= incr;
	EXIT_CRITICAL_SECTION(usis);
	return (char *)previous_heap_end;
}

void *sbrk(int incr) { return _sbrk_r(_impure_ptr, incr); }

void *_sbrk(int incr) { return sbrk(incr); };

//
// malloc_[un]lock implementation
//

static UBaseType_t malloc_saved_interrupt_status;
static int malloc_lock_counter = 0;

void __malloc_lock(struct _reent *r)
{
	UNUSED UBaseType_t interrupt_status;
	if (malloc_lock_counter == 0) {
		ENTER_CRITICAL_SECTION(interrupt_status);
		malloc_saved_interrupt_status = interrupt_status;
	}
	malloc_lock_counter += 1;

};

void __malloc_unlock(struct _reent *r)
{
	malloc_lock_counter -= 1;
	if (malloc_lock_counter == 0)
		EXIT_CRITICAL_SECTION(malloc_saved_interrupt_status);

};

uint32_t mem_is_heap_allocated(const void *ptr)
{
	return (ptr >= (void *)__HeapBase && ptr < (void *)__HeapLimit);
}