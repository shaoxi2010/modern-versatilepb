cmake_minimum_required(VERSION 3.18)
project(dload)

OPTION(BINARY_ELF "link target elf")
if(${BINARY_ELF})
message(FATAL_ERROR "You Must specfiy one runtime")
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}/../include
)

add_compile_options(-fPIC)

aux_source_directory(. DIR_SRC)

add_executable(${PROJECT_NAME} ${DIR_SRC})

target_link_options(${PROJECT_NAME} PRIVATE -Wl,-R${BINARY_ELF})
target_link_options(${PROJECT_NAME} PRIVATE -nostdlib -nostartfiles)

add_custom_command(
    TARGET ${PROJECT_NAME}
    COMMAND arm-none-eabi-objcopy --strip-all -O binary ${PROJECT_NAME}  ${PROJECT_NAME}.bin
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)