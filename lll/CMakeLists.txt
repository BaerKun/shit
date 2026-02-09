cmake_minimum_required(VERSION 3.10)
project(lll)

set(CMAKE_CXX_STANDARD 11)

set(SRC_LLL_INTEGER
    integer/division.cpp
    integer/math.cpp
    integer/multiply.cpp
    integer/simply.cpp
    integer/io.cpp
)

add_library(lll ${SRC_LLL_INTEGER})

add_executable(test _test.cpp)
target_link_libraries(test PRIVATE lll)