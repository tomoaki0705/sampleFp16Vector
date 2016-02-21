cmake_minimum_required(VERSION 2.8)

project(sampleFp16x86)

if(UNIX)
	set(CMAKE_CXX_FLAGS "-mavx -mf16c")
endif(UNIX)

add_executable(sampleFp16x86 main.cpp)
