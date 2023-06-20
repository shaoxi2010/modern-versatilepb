project(FreeRTOS)

add_subdirectory(kernel)

add_subdirectory(c++addon)

#添加到编译系统
set(TARGET_OBJS ${TARGET_OBJS} ${FREERTOS_OBJS} PARENT_SCOPE)