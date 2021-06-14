cmake_minimum_required(VERSION 3.10)
project(CS_wet3)

set(CMAKE_CXX_STANDARD 11)
add_compile_options(-std=c++11 -Wall)
add_executable(CS_wet3 dflow_calc.cpp dflow_main.c dflow_calc.h)