cmake_minimum_required(VERSION 3.10)
project(CS_wet4)

set(CMAKE_CXX_STANDARD 11)
add_compile_options(-std=c++11 -Wall)
add_executable(CS_wet4 core_api.c core_api.h main.c sim_api.c sim_api.h)