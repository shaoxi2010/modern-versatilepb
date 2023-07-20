project(newlib-stub)

enable_language(ASM C)

add_compile_options(-DFREERTOS)

include_directories(
    ${CMAKE_SOURCE_DIR}/FreeRTOS/kernel/include
    ${CMAKE_SOURCE_DIR}/FreeRTOS/kernel/portable/GCC
    )

#添加目录下的汇编文件
file(GLOB DIR_ASM "*.S")

#添加目录下的所以文件
aux_source_directory(. DIR_SRC)

#默认的libc是没有打开lto优化的，强行优化会导致链接依赖找不到
if(${LTO})
add_compile_options(-fno-lto)
endif()

#添加目标链接库
add_library(${PROJECT_NAME} OBJECT ${DIR_SRC} ${DIR_ASM})

#添加到编译系统
set(TARGET_OBJS ${TARGET_OBJS} ${PROJECT_NAME} PARENT_SCOPE)