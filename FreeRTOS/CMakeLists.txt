project(FreeRTOS)

add_subdirectory(kernel)

#添加到编译系统
set(TARGET_OBJS ${TARGET_OBJS} ${FREERTOS_OBJS} PARENT_SCOPE)