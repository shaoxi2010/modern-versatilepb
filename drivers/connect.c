#include "exceptions.h"
#include "interrupt.h"

void hal_irq_handle(struct registers *regs)
{
	extern void _pic_IrqHandler(void);
	_pic_IrqHandler();
}