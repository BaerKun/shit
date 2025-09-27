cmake_minimum_required(VERSION 3.10)
project(lll)

add_library(lll_int integer.cpp)

add_executable(test _test.cpp)
target_link_libraries(test PRIVATE lll_int)