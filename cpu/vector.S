.equ  MODE_USR,    0b11010000       @ User Mode
.equ  MODE_FIQ,    0b11010001       @ FIQ Mode
.equ  MODE_IRQ,    0b11010010       @ IRQ Mode
.equ  MODE_SVC,    0b11010011       @ Supervisor Mode
.equ  MODE_ABT,    0b11010111       @ Abort Mode
.equ  MODE_UND,    0b11011011       @ Undefined Mode
.equ  MODE_SYS,    0b11011111       @ System Mode

/*
 * 设计只使用了SVC的堆栈，异常则是USR和SYS模式才行
 * struct reg {
 *  uint32_t cpsr;
 *  uint32_t r[13];
 *  uint32_t lr;
 *  uint32_t sp;
 *  uint32_t pc;
 *};
 */

.extern hal_undefine_handle
.extern hal_swi_handle
.extern hal_prefectabort_handle
.extern hal_dataabort_handle
.extern hal_irq_handle
.extern hal_fiq_handle

.global hal_undefine
.global hal_swi
.global hal_prefetch_abrot
.global hal_data_abrot
.global no_use
.global hal_irq
.global hal_fiq

.text
.code	32

.global	undef_handler
/*
 * 	对arm架构不管是多少级流水线都按三级流水线来计算pc
 *	指令地址
 *	A     pc-8 正在执行的指令
 *	A+4   pc-4 
 *	A+8	  pc
 *	发生异常或者中断时，arm处理器自动将pc-4放入对应模式的lr寄存器，cpsr放入spsr
 */
hal_undefine:
    /*
        发生未定义指令时，pc = A+8, 所以lr = A + 4,即下一条指令的地址
        中断处理完毕后需要执行下一条指令，所以lr不需要修改
    */
    sub lr, lr, #0                      /*计算异常时的PC*/
    msr cpsr_c, #MODE_SVC
    sub sp, sp, #4						/*留出pc*/
    stmfd sp, {r0-lr}^					/*切换到svc模式，使用它的堆栈指针sp,将用户模式下的寄存器入栈*/
    msr cpsr_c, #MODE_UND		        /*切换回未定义指令模式*/
    mov r0, lr
    mrs r1, spsr 
    msr cpsr_c, #MODE_SVC
    add sp, sp, #4		
    stmfd sp, {r0}				    	/*保存返回地址pc，和异常前模式的cprs*/
	sub sp, sp, #64						/*r0-r12, sp, lr 共15个寄存器，60个字节*/
    stmfd sp!, {r1}						/*压入spsr*/
    mov r0, sp                          /*传入struct reg变量*/
    bl hal_undefine_handle
    ldmfd sp!, {r0}						/*弹出spsr*/
    msr spsr, r0 						/*将spsr(中断前的cpsr)存入svc模式的spsr中*/
    ldmfd sp, {r0-sp}^					/*恢复中断前对应模式的寄存器*/
    add sp, sp, #60
    ldmfd sp!, {pc}^

hal_swi:
	/*
        发生软中断时，pc = A+8, 所以lr = A + 4,即下一条指令的地址
        中断处理完毕后需要执行下一条指令，所以lr不需要修改，软中断系统直接在SVC模式
	*/
	sub lr, lr, #0
	stmfd sp!, {r0-r12, lr}				/*使用它的堆栈指针sp,将公共寄存器压入堆栈*/                   
	bl hal_swi_handle
	ldmfd sp!, {r0-r12, pc}^			/*恢复中断前对应模式的寄存器*/

hal_prefetch_abrot:
	/*
		发生预取指令异常时，pc = A+8, 所以lr = A + 4,即下一条指令的地址
		中断处理完毕后需要重新执行这条指令，所以lr需要减4
	*/
	sub lr, lr, #4                      /*计算异常时的PC*/
	msr cpsr_c, #MODE_SVC
	sub sp, sp, #4						/*留出pc*/
	stmfd sp, {r0-lr}^					/*切换到svc模式，使用它的堆栈指针sp,将公共寄存器压入堆栈*/
	msr cpsr_c, #MODE_ABT			    /*切换回终止模式*/
	mov r0, lr
	mrs r1, spsr                       
	msr cpsr_c, #MODE_SVC
	add sp, sp, #4		
	stmfd sp, {r0}				    	/*保存返回地址pc，和异常前模式的cprs*/
	sub sp, sp, #64						/*r0-r12, sp, lr 共15个寄存器，60个字节*/
	stmfd sp!, {r1}						/*压入spsr*/
    mov r0, sp                          /*传入struct reg变量*/
	bl hal_prefectabort_handle
	ldmfd sp!, {r0}						/*弹出spsr*/
	msr spsr, r0 						/*将spsr(中断前的cpsr)存入svc模式的spsr中*/
	ldmfd sp, {r0-sp}^					/*恢复中断前对应模式的寄存器*/
	add sp, sp, #60
	ldmfd sp!, {pc}^

hal_data_abrot:
	/*
		发生数据访问异常时，pc = A+12, 所以lr = A + 8,即下一条指令的地址
		中断处理完毕后需要重新预取数据，所以lr需要减8
	*/
	sub lr, lr, #8                      /*计算异常时的PC*/
	msr cpsr_c, #MODE_SVC
	sub sp, sp, #4						/*留出pc*/
	stmfd sp, {r0-lr}^					/*切换到svc模式，使用它的堆栈指针sp,将公共寄存器压入堆栈*/
	msr cpsr_c, #MODE_ABT   			/*切换回终止模式*/
	mov r0, lr
	mrs r1, spsr                       
	msr cpsr_c, #MODE_SVC
	add sp, sp, #4		
	stmfd sp, {r0}				    	/*保存返回地址pc，和异常前模式的cprs*/
	sub sp, sp, #64						/*r0-r12, sp, lr 共15个寄存器，60个字节*/
	stmfd sp!, {r1}						/*压入spsr*/
    mov r0, sp                          /*传入struct reg变量*/
	bl hal_dataabort_handle
	ldmfd sp!, {r0}						/*弹出spsr*/
	msr spsr, r0 						/*将spsr(中断前的cpsr)存入svc模式的spsr中*/
	ldmfd sp, {r0-sp}^					/*恢复中断前对应模式的寄存器*/
	add sp, sp, #60
	ldmfd sp!, {pc}^
no_use:
	b .

hal_irq:
	/*
		发生irq时，pc = A+12, 所以lr = A + 8,即下二条指令的地址
		中断处理完毕后需要执行下一条指令，所以lr需要减4
	*/
	sub lr, lr, #4                      /*计算异常时的PC*/
	msr cpsr_c, #MODE_SVC
	sub sp, sp, #4						/*留出pc*/
	stmfd sp, {r0-lr}^					/*切换到svc模式，使用它的堆栈指针sp,将用户模式的寄存器压栈*/
	msr cpsr_c, #MODE_IRQ   			/*切换irq模式*/
	mov r0, lr
	mrs r1, spsr  
	msr cpsr_c, #MODE_SVC
	add sp, sp, #4		
	stmfd sp, {r0}				    	/*保存返回地址pc，和异常前模式的cprs*/
	sub sp, sp, #64						/*用户模式r0-lr, pc共16个寄存器，64个字节*/
	stmfd sp!, {r1}						/*压入spsr, lr*/
	mrs r1, spsr  						/*保存irq的spsr到svc的spsr*/
    mov r0, sp                          /*传入struct reg变量*/
	bl hal_irq_handle
	ldmfd sp!, {r1}						/*弹出spsr, lr*/
	msr spsr, r1 						/*将spsr(中断前的cpsr)存入svc模式的spsr中*/
	ldmfd sp, {r0-lr}^					/*恢复用户模式的寄存器*/
	add sp, sp, #60
	ldmfd sp!, {pc}^					/*恢复返回地址，同时将spsr放入cpsr*/
	
hal_fiq:
	/*
		发生fiq时，pc = A+12, 所以lr = A + 8,即下一条指令的地址
		中断处理完毕后需要执行下一条指令，所以lr需要减4
	*/
	sub lr, lr, #4                      /*计算异常时的PC*/
	msr cpsr_c, #MODE_SVC
	sub sp, sp, #4						/*留出pc*/
	stmfd sp, {r0-r7}					/*切换到svc模式，使用它的堆栈指针sp,将公共寄存器压入堆栈*/
	msr cpsr_c, #MODE_FIQ   			/*切换fiq模式*/
	mov r0, lr
	mrs r1, spsr                       
	msr cpsr_c, #MODE_SVC
	add sp, sp, #4		
	stmfd sp, {r0}				    	/*保存返回地址pc，和异常前模式的cprs*/
	sub sp, sp, #36						/*r0-r7, lr 共9个寄存器，36个字节*/
	stmfd sp!, {r1}						/*压入spsr*/
	bl hal_fiq_handle
	ldmfd sp!, {r0}						/*弹出spsr*/
	msr spsr, r0 						/*将spsr(中断前的cpsr)存入svc模式的spsr中*/
	ldmfd sp!, {r0-r7, pc}^				/*恢复中断前对应模式的寄存器*/

