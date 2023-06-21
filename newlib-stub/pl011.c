#include "uart.h"
#include <reent.h>
#include <sys/types.h>

ssize_t _write_r(struct _reent *ptr, int fd, const void *buf, size_t count)
{
	const char *data = (const char *)buf;
	for (int i = 0; i < count; i++) {
		uart_printChar(0, data[i]);
	}
	return count;
}
