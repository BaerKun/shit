cmake_minimum_required(VERSION 3.10)
project(lll)

set(CMAKE_CXX_STANDARD 11)

add_library(lll_int
        integer/division.cpp
        integer/internal.cpp
        integer/multiply.cpp
        integer/simply.cpp)

add_executable(test _test.cpp)
target_link_libraries(test PRIVATE lll_int)