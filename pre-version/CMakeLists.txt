cmake_minimum_required(VERSION 3.3)
project(pre_version)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    main.cpp
    SimJoiner.cpp
    SimJoiner.h)

add_executable(pre_version ${SOURCE_FILES})