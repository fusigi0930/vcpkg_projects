cmake_minimum_required(VERSION 3.10)

set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")

project(cv_fps)

find_package(OpenCV CONFIG REQUIRED)
find_package(argparse CONFIG REQUIRED)

add_executable(cv_fps cv_fps.cpp)
target_compile_features(cv_fps PRIVATE cxx_std_17)
target_link_libraries(cv_fps PRIVATE ${OpenCV_LIBS})
target_link_libraries(cv_fps PRIVATE argparse::argparse)