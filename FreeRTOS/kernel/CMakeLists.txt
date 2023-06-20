cmake_minimum_required(VERSION 3.15)

# User is responsible to set two mandatory options:
# FREERTOS_CONFIG_FILE_DIRECTORY
# FREERTOS_PORT
#
# User can choose which heap implementation to use (either the implementations
# included with FreeRTOS [1..5] or a custom implementation ) by providing the
# option FREERTOS_HEAP. If the option is not set, the cmake will default to
# using heap_4.c.

# Heap number or absolute path to custom heap implementation provided by user
set(FREERTOS_HEAP "4" CACHE STRING "FreeRTOS heap model number. 1 .. 5. Or absolute path to custom heap source file")

add_library(kernel OBJECT
    croutine.c
    event_groups.c
    list.c
    queue.c
    stream_buffer.c
    tasks.c
    timers.c
    portable/GCC/port.c
    # If FREERTOS_HEAP is digit between 1 .. 5 - it is heap number, otherwise - it is path to custom heap source file
    $<IF:$<BOOL:$<FILTER:${FREERTOS_HEAP},EXCLUDE,^[1-5]$>>,${FREERTOS_HEAP},portable/MemMang/heap_${FREERTOS_HEAP}.c>
)

include_directories(
    include
    portable/GCC
    )
#添加到编译系统
set(FREERTOS_OBJS ${FREERTOS_OBJS} kernel PARENT_SCOPE)