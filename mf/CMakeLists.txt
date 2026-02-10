cmake_minimum_required(VERSION 3.10)
project(mf)

set(CMAKE_CXX_STANDARD 11)

set(SRC_MF math_func.cpp)

add_library(mf ${SRC_MF})

add_executable(test _test.cpp)
target_link_libraries(test PRIVATE mf)