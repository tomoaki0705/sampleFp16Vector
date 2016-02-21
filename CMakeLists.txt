cmake_minimum_required(VERSION 2.8)

project(sampleFp16x86)

set(CMAKE_CXX_FLAGS "-mavx -mf16c")

add_executable(sampleFp16x86 main.cpp)
