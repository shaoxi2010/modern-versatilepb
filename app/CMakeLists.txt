project(app)

#添加目录下的所以文件
aux_source_directory(. DIR_SRC)

#添加覆盖率测试
if(GCOV)
# make sure add_compile_options() command is before the add_library()/add_executable
add_compile_options(-fprofile-arcs -ftest-coverage)
endif()

#添加目标链接库
add_library(${PROJECT_NAME} OBJECT ${DIR_SRC})

#添加到编译系统
set(TARGET_OBJS ${TARGET_OBJS} ${PROJECT_NAME} PARENT_SCOPE)

