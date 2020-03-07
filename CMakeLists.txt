cmake_minimum_required(VERSION 3.14)
project(pnm_lines C)

set(CMAKE_C_STANDARD 99)

add_executable(pnm_lines main.c)
target_link_libraries(pnm_lines m)
