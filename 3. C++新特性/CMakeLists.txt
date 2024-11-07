cmake_minimum_required(VERSION 3.0)

project(zero_threadpool)

include_directories(.)

add_compile_options(-std=c++14 -o2)

add_executable(main.exe zero_threadpool.cpp main.cpp) 