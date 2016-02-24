cmake_minimum_required(VERSION 2.8)

project(sampleFp16x86)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm.*|ARM.*)")
	set(ARM 1)
else()
	set(X86 1)
endif()

if (x86)
	if (UNIX)
		set(CMAKE_CXX_FLAGS "-mavx -mf16c")
	elseif (WIN32)
		set(CMAKE_CXX_FLAGS "/arch:AVX /EHsc")
	endif()
elseif (ARM)
	if (UNIX)
		set(CMAKE_CXX_FLAGS "-mfp16-format=ieee")
	endif()
endif()

add_executable(sampleFp16x86 main.cpp floatMul.cpp)
