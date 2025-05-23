cmake_minimum_required(VERSION 3.10)

set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")

project(cv_fps)

find_package(OpenCV CONFIG REQUIRED)
find_package(argparse CONFIG REQUIRED)
#if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
	find_package(libuvc CONFIG REQUIRED)
	#find_package(libusb CONFIG REQUIRED)
#endif()

#if (NOT DEFINED CMAKE_WIN_BACK)
#	set(CMAKE_WIN_BACK "MSMF")
#endif()

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
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	target_link_libraries(cv_fps PRIVATE LibUVC::uvc_static)
	#target_link_libraries(cv_fps PRIVATE ${libusb_LIBS})
else()
	target_link_libraries(cv_fps PRIVATE uvc)
#	if (CMAKE_WIN_BACK STREQUAL "DSHOW")
#		message("use dshow library...")
#		target_compile_definitions(cv_fps PRIVATE -DWIN_UVC_BACK=0)
#		target_link_libraries(cv_fps PRIVATE ole32.lib)
#	elseif (CMAKE_WIN_BACK STREQUAL "MSMF")
#		message("use msmf library...")
#		target_compile_definitions(cv_fps PRIVATE -DWIN_UVC_BACK=1)
#		target_link_libraries(cv_fps PRIVATE mf.lib mfplat.lib mfuuid.lib mfreadwrite.lib)
#	endif()
endif()