project(cpu)

enable_language(ASM C)

#添加目录下的所以文件
aux_source_directory(. DIR_SRC)

#添加目录下的汇编文件
file(GLOB DIR_ASM "*.S")

#需要使用对null地址拷贝
set_source_files_properties(./exceptions.c PROPERTIES COMPILE_FLAGS -Wno-nonnull)

#添加目标链接库
add_library(${PROJECT_NAME} OBJECT ${DIR_SRC} ${DIR_ASM})

#添加到编译系统
set(TARGET_OBJS ${TARGET_OBJS} ${PROJECT_NAME} PARENT_SCOPE)

