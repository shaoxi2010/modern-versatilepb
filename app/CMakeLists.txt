project(app)

#添加目录下的所以文件
aux_source_directory(. DIR_SRC)

#添加目标链接库
add_library(${PROJECT_NAME} OBJECT ${DIR_SRC})

#添加到编译系统
set(TARGET_OBJS ${TARGET_OBJS} ${PROJECT_NAME} PARENT_SCOPE)

#添加覆盖率测试
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")