cmake_minimum_required(VERSION 2.8)

project(sampleFp16)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm.*|ARM.*)")
	set(ARM 1)
else()
	set(X86 1)
endif()

if (X86)
	message(STATUS "detected x86 architecture")
	set (FEATURESUPPORT featureSupportX86.cpp)
	if (UNIX)
		set(CMAKE_CXX_FLAGS "-mf16c")
	endif()
elseif (ARM)
	message(STATUS "detected ARM architecture")
	set (FEATURESUPPORT featureSupportARM.cpp)
	if (UNIX)
		set(CMAKE_CXX_FLAGS "-mfp16-format=ieee -mfpu=neon-fp16 -flax-vector-conversions")
	endif()
endif()

find_package(OpenCV REQUIRED)

add_executable(verifyMultiply verifyMul.cpp floatMul.cpp ${FEATURESUPPORT})
add_executable(sampleFp16 main.cpp floatMul.cpp ${FEATURESUPPORT})
add_executable(floatImage2HalfImage converter.cpp floatMul.cpp ${FEATURESUPPORT})

if(OpenCV_FOUND)
  include_directories(${OpenCV_INCLUDE_DIRS})

  target_link_libraries(floatImage2HalfImage ${OpenCV_LIBS})

  link_directories(${CUDA_CUDART_LIBRARY})
endif(OpenCV_FOUND)
				
