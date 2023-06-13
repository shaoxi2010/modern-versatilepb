project(drivers)

#添加目录下的所以文件
aux_source_directory(. DIR_SRC)

#添加目标链接库
add_library(${PROJECT_NAME} OBJECT ${DIR_SRC})

#添加到编译系统
set(TARGET_OBJS ${TARGET_OBJS} ${PROJECT_NAME} PARENT_SCOPE)