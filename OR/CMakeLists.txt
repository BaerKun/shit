cmake_minimum_required(VERSION 3.16)
project(OR CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(Eigen3 CONFIG REQUIRED)

add_library(OR STATIC linear.cpp transport.cpp)
target_include_directories(OR PUBLIC ${EIGEN3_INCLUDE_DIRS})