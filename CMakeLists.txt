cmake_minimum_required(VERSION 3.18)
project(versatilepb)

# CMAKE_TOOLCHAIN_FILE 指定工具链集合

option(MAPS "输出链接信息文件")

option(VERBOSE "详细编译输出")
set(CMAKE_VERBOSE_MAKEFILE ${VERBOSE})

option(TEXT "代码段起始地址")
option(DATA "数据段起始地址")
option(STACK "堆栈段起始地址")
option(GCOV "代码覆盖率检查")

#目标程序名称
set(TARGET ${PROJECT_NAME})

add_subdirectory(app)
add_subdirectory(newlib-stub)
add_subdirectory(cpu)
if(GCOV)
# Check GCC version
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 12)
        message(FATAL_ERROR "GCOV need GCC version must be less than 12")
    endif()
endif()
add_subdirectory(gcov-stub)
endif()

#编译链接
add_executable(${TARGET})
target_link_libraries(${TARGET} ${TARGET_OBJS})

target_link_options(${TARGET} PRIVATE -T ${CMAKE_CURRENT_SOURCE_DIR}/scripts/arm-none-eabi.ld)
if(${MAPS})
target_link_options(${TARGET} PRIVATE -Wl,-Map=${TARGET}.map)
endif()

if(${TEXT})
target_link_options(${TARGET} PRIVATE -Wl,-Ttext-segment=${TEXT})
endif()

if(${DATA})
target_link_options(${TARGET} PRIVATE -Wl,-Tdata-segment=${DATA})
endif()

if(${STACK})
target_link_options(${TARGET} PRIVATE -Wl,-Tstack-segment=${STACK})
endif()

#QEMU 运行
add_custom_command(
    TARGET ${TARGET} 
    POST_BUILD
    COMMAND qemu-system-arm -machine versatilepb -display none -semihosting -serial stdio  -kernel ${CMAKE_BINARY_DIR}/${TARGET} > ${CMAKE_BINARY_DIR}/log.txt
    COMMENT "test on qemu"
    VERBATIM
)