set ORI_PATH=%PATH%

if "%~1" == "" (
	set MINGW_PATH=c:\msys64\ucrt64\bin
) else (
	set MINGW_PATH=%~1
)

set PATH=%MINGW_PATH%;%PATH%

cmake -B build_win -G Ninja ^
	-DCMAKE_BUILD_TYPE='Release' ^
	-DVCPKG_BUILD_TYPE='Release' ^
	-DVCPKG_TARGET_TRIPLET=x64-mingw-static ^
	-DCMAKE_C_COMPILER=gcc.exe ^
	-DCMAKE_CXX_COMPILER=g++.exe ^
	-DCMAKE_MAKE_PROGRAM=ninja.exe ^
	-DVCPKG_HOST_TRIPLET=x64-mingw-static ^
	-DVCPKG_OVERLAY_PORTS=local_ports ^
	-DVCPKG_BINARY_SOURCES="clear;files,c:/users/coder/project/vcpkg_cache,readwrite"

cmake --build build_win -j8

set PATH=%ORI_PATH%