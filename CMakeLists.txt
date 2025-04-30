cmake_minimum_required(VERSION 3.31)
project(echo_server C)

set(CMAKE_C_STANDARD 11)

add_executable(svr svr.c)
target_link_libraries(svr ws2_32)

add_executable(cli cli.c)
target_link_libraries(cli ws2_32)
#trysth