cmake_minimum_required(VERSION 3.10)

set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")

project(cv_fps)

find_package(OpenCV CONFIG REQUIRED)
find_package(argparse CONFIG REQUIRED)
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
	find_package(libuvc CONFIG REQUIRED)
endif()

set(CV_TOOLS
	cv_fps.cpp
	main.cpp
	common.cpp
	uvc_utils.cpp
)

add_executable(cv_fps
	${CV_TOOLS}
)

target_compile_features(cv_fps PRIVATE cxx_std_17)
target_link_libraries(cv_fps PRIVATE ${OpenCV_LIBS})
target_link_libraries(cv_fps PRIVATE argparse::argparse)
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
	target_link_libraries(cv_fps PRIVATE uvc)
else()
	target_link_libraries(cv_fps PRIVATE strmiids.lib ole32.lib)
endif()