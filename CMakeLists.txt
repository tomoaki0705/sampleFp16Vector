cmake_minimum_required(VERSION 2.8)

project(sampleFp16)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm.*|ARM.*)")
	set(ARM 1)
else()
	set(X86 1)
endif()

if (X86)
	message(STATUS "detected x86 architecture")
	if (UNIX)
		set(CMAKE_CXX_FLAGS "-mavx -mf16c")
	endif()
elseif (ARM)
	message(STATUS "detected ARM architecture")
	if (UNIX)
		set(CMAKE_CXX_FLAGS "-mfp16-format=ieee -mfpu=neon-fp16 -flax-vector-conversions")
	endif()
endif()

add_executable(sampleFp16 main.cpp floatMul.cpp)
