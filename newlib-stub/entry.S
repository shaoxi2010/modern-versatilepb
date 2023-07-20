.extern _start

.section .text.entry
.code 32                                   @ 32-bit ARM instruction set

newlib_entry:
    ldr pc, reset_addr

@ Labels with addresses to exception handler routines, referenced above:
reset_addr:
    .word _start