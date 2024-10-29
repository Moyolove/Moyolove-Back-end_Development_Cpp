
cmake_minimum_required(VERSION 3.0)

project(strategy)

add_compile_options(-Wall -std=c++11 -O2)

add_executable(main strategy.cpp)