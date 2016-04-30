cmake_minimum_required(VERSION 3.3)
project(test2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp SimJoiner.h SimJoiner.cpp)
add_executable(test2 ${SOURCE_FILES})