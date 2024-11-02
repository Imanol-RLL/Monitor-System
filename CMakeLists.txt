cmake_minimum_required(VERSION 3.29)
project(so_i_24_Imanol_RLL C)
set(CMAKE_C_STANDARD 11)
include_directories(include)
add_executable(so_i_24_Imanol_RLL
        include/expose_metrics.h
        include/metrics.h
        src/expose_metrics.c
        src/main.c
        src/metrics.c)
target_link_libraries(so_i_24_Imanol_RLL prom)
target_link_libraries(so_i_24_Imanol_RLL promhttp)