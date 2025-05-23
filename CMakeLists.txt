cmake_minimum_required(VERSION 3.31)
project(echo_core C)

set(CMAKE_C_STANDARD 11)

if (MSVC)
    add_compile_options(/utf-8)
endif ()

add_executable(svr svr.c)
target_link_libraries(svr ws2_32)

add_executable(cli cli.c)
target_link_libraries(cli ws2_32)

add_library(cli_core cli_core.c cli_core.h)
target_link_libraries(cli_core ws2_32)