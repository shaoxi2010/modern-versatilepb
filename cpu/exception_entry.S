/* 异常系统设计模式，匹配newlib栈模型
* newlib stack modes:               
*   +-----+ <- SL_sys,          模型设计：
*   |     |    SL_usr             1. 栈的关系比较混乱，就不统一栈模型来
*   | SYS |                       2. fiq本身用于快速中断除开必要保护不再处理  
*   | USR | -=0x10000             3. SYS和USR认为是用户态，捕获寄存器并保存
*   |     |                       4. 其他模式为系统态，lr固定为0xffffffff
*   +-----+ <- SP_sys,
*   |     |    SP_usr      
*   | SVC | -= 0x8000       
*   |     |                   
*   +-----+ <- SP_svc       
*   |     |                  
*   | IRQ | -= 0x2000       
*   |     |                  
* ^ +-----+ <- SP_und         
* s |     |                   
* t | UND | -= 0x1000         
* a |     |                
* c +-----+ <- SP_und       
* k |     |                
*   | ABT | -= 0x1000       
* g |     |               
* r +-----+ <- SP_abt,     
* o |     |    SL_fiq       
* w | FIQ | -= 0x1000       
* t |     |                
* h +-----+ <- initial SP,
*/
@ 定义ARM的工作模式常量
.equ  PSR_MASK,    0x0000001F       @ CSPR bits that define operating mode

.equ  MODE_USR,    0x00000010       @ User Mode
.equ  MODE_FIQ,    0x00000011       @ FIQ Mode
.equ  MODE_IRQ,    0x00000012       @ IRQ Mode
.equ  MODE_SVC,    0x00000013       @ Supervisor Mode
.equ  MODE_ABT,    0x00000017       @ Abort Mode
.equ  MODE_UND,    0x0000001B       @ Undefined Mode
.equ  MODE_SYS,    0x0000001F       @ System Mode

.equ FIQ_BIT,      0x00000040       @ FIQ exception enable/disable bit
.equ IRQ_BIT,      0x00000080       @ IRQ exception enable/disable bit

.macro dispatch_excption_by_mode sys_handler,usr_handler
    stmfd sp!, {r0, r1}
    mrs r1, spsr
    and r0, r1, #PSR_MASK
    cmp r0, #MODE_USR
    beq 1f
    cmp r0, #MODE_SYS
    beq 1f
    ldmfd sp!, {r0, r1}
    b \sys_handler
    b 2f
1:  ldmfd sp!, {r0, r1}
    b \usr_handler
2:  b .
.endm

.macro switch_mode mode,disable_irq:req
    .if \disable_irq == 1
        msr cpsr, #FIQ_BIT | IRQ_BIT | \mode
    .else
        stmfd sp!, {r0, r1}            @ 保护r0, r1寄存器，为切换做准备
        mrs r1, cpsr                    @ 将原始量存入r1
        and r0, r1, #FIQ_BIT | IRQ_BIT
        cmp r0, #FIQ_BIT | IRQ_BIT      
        beq 1f
        and r0, r1, #FIQ_BIT
        cmp r0, #FIQ_BIT
        beq 2f
        and r0, r1, #IRQ_BIT
        cmp r0, #IRQ_BIT
        beq 3f
        ldmfd sp!, {r0, r1}
        msr cpsr, #\mode
1:      ldmfd sp!, {r0, r1}
        msr cpsr, #FIQ_BIT | IRQ_BIT | \mode
2:      ldmfd sp!, {r0, r1}
        msr cpsr, #FIQ_BIT | \mode
3:      ldmfd sp!, {r0, r1}
        msr cpsr, #IRQ_BIT | \mode
    .endif
.endm

.macro usr_handler handler
    stmfd sp!, {lr}                     /*读取用户异常地址*/
    stmfd sp, {r0-lr}^					/*将用户模式下的寄存器入栈*/
    sub sp, sp, #60
    mrs r1, spsr                        	
    stmfd sp!, {r1}						/*压入spsr*/
    mov r0, sp                          /*传入struct reg变量*/
    bl \handler
    ldmfd sp!, {r0}						/*弹出spsr*/
    msr spsr, r0 						/*将spsr(中断前的cpsr)存入svc模式的spsr中*/
    ldmfd sp, {r0-sp}^					/*恢复中断前对应模式的寄存器*/
    add sp, sp, #60
    ldmfd sp!, {pc}^
.endm

.macro sys_handler handler
    stmfd sp!, {lr}                     /*保存PC*/
    ldr lr, =0xffffffff                          
    stmfd sp!, {lr}                     /*虚拟lr*/
    add lr, sp, #8                      /*恢复栈顶位置*/
    stmfd sp!, {lr}                     /*保存SP*/
    stmfd sp!, {r0-r12}
    mrs lr, spsr 
    stmfd sp!, {lr}                     /*保存CPSR*/  
    mov r0, sp                          /*传入struct reg变量*/
    bl \handler
    ldmfd sp!, {r0}						/*弹出spsr*/
    msr spsr, r0 						/*将spsr(中断前的cpsr)存入svc模式的spsr中*/
    ldmfd sp, {r0-r12}					/*恢复中断前对应模式的寄存器*/
    add sp, sp, #60
    ldmfd sp!, {pc}^
.endm

.extern hal_undefine_handle
.extern hal_swi_handle
.extern hal_prefectabort_handle
.extern hal_dataabort_handle
.extern hal_irq_handle
.extern hal_fiq_handle

.text
.code	32

.global hal_vectors_start

hal_vectors_start:
    @ Exception vectors, relative to the base address, see page 2-26 of DDI0222
    ldr pc, reset_handler_addr             @ Reset (and startup) vector
    ldr pc, undef_handler_addr             @ Undefined (unknown) instruction
    ldr pc, swi_handler_addr               @ Software interrupt
    ldr pc, prefetch_abort_handler_addr    @ Prefetch abort
    ldr pc, data_abort_handler_addr        @ Data abort (system bus cannot access a peripheral)
    ldr pc, invalid_addr_handler           @ Reserved (early ARM only supported 26-bit addresses)
    ldr pc, irq_handler_addr               @ IRQ handler
    ldr pc, fiq_handler_addr               @ FIQ handler

@ Labels with addresses to exception handler routines, referenced above:
reset_handler_addr:
    .word _start
undef_handler_addr:
    .word hal_undefine
swi_handler_addr:
    .word hal_swi
prefetch_abort_handler_addr:
    .word hal_prefetch_abort
data_abort_handler_addr:
    .word hal_data_abort
invalid_addr_handler:
    .word no_use
irq_handler_addr:
    .word hal_irq
fiq_handler_addr:
    .word hal_fiq

.global hal_vectors_end
hal_vectors_end:

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
    dispatch_excption_by_mode 3f, 4f
3:  sys_handler hal_undefine_handle
4:  usr_handler hal_undefine_handle

hal_swi:
	/*
        发生软中断时，pc = A+8, 所以lr = A + 4,即下一条指令的地址
        中断处理完毕后需要执行下一条指令，所以lr不需要修改，软中断系统直接在SVC模式
	*/
	sub lr, lr, #0
    dispatch_excption_by_mode 3f, 4f
3:  sys_handler hal_swi_handle
4:  usr_handler hal_swi_handle


hal_prefetch_abort:
	/*
		发生预取指令异常时，pc = A+8, 所以lr = A + 4,即下一条指令的地址
		中断处理完毕后需要重新执行这条指令，所以lr需要减4
	*/
	sub lr, lr, #4                      /*计算异常时的PC*/
    dispatch_excption_by_mode 3f, 4f
3:  sys_handler hal_prefectabort_handle
4:  usr_handler hal_prefectabort_handle


hal_data_abort:
	/*
		发生数据访问异常时，pc = A+12, 所以lr = A + 8,即下一条指令的地址
		中断处理完毕后需要重新预取数据，所以lr需要减8
	*/
	sub lr, lr, #8                      /*计算异常时的PC*/
    dispatch_excption_by_mode 3f, 4f
3:  sys_handler hal_dataabort_handle
4:  usr_handler hal_dataabort_handle


no_use:
	b .

hal_irq:
	/*
		发生irq时，pc = A+12, 所以lr = A + 8,即下二条指令的地址
		中断处理完毕后需要执行下一条指令，所以lr需要减4
	*/
	sub lr, lr, #4                      /*计算异常时的PC*/
    dispatch_excption_by_mode 3f, 4f
3:  sys_handler hal_irq_handle
4:  usr_handler hal_irq_handle
	
hal_fiq:
	/*
		发生fiq时，pc = A+12, 所以lr = A + 8,即下一条指令的地址
		中断处理完毕后需要执行下一条指令，所以lr需要减4
	*/
	sub lr, lr, #4                      /*计算异常时的PC*/
	stmfd sp!, {r0-r7, lr}				/*保存可能被修改的寄存器*/
    mrs r0, spsr                        	
    stmfd sp!, {r0}						/*压入spsr*/
	bl hal_fiq_handle
    ldmfd sp!, {r0}						/*弹出spsr*/
    msr spsr, r0 						/*将spsr(中断前的cpsr)存入svc模式的spsr中*/
	ldmfd sp!, {r0-r7, pc}^				/*恢复中断前对应模式的寄存器*/
