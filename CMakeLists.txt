cmake_minimum_required(VERSION 3.16)
project(loongrisc_simulator CXX)

# 供 clangd / IDE 使用：生成 build/compile_commands.json（路径固定为相对仓库根的 build/）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_program(CLANG_FORMAT_BIN clang-format)
if(CLANG_FORMAT_BIN)
    add_custom_target(
        format
        COMMAND ${CMAKE_SOURCE_DIR}/scripts/format.sh
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Run clang-format on all tracked source files")

    add_custom_target(
        format-check
        COMMAND ${CMAKE_SOURCE_DIR}/scripts/format-check.sh
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Check clang-format on all tracked source files")
else()
    message(WARNING "clang-format not found: format targets are unavailable")
endif()

add_subdirectory(sim)

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(test)
endif()
