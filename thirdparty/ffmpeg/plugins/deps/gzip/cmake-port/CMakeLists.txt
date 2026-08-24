cmake_minimum_required(VERSION 3.20)
project(gzip_tool C)

add_executable(gzip gzip_main.c)

find_path(ZLIB_INCLUDE_DIR zlib.h
	HINTS
		"${CMAKE_INSTALL_PREFIX}/include"
		"${CMAKE_PREFIX_PATH}/include"
		ENV INCLUDE
	REQUIRED
)
find_library(ZLIB_LIBRARY
	NAMES z zlib zlibstatic libzsd libzs
	HINTS
		"${CMAKE_INSTALL_PREFIX}/lib"
		"${CMAKE_PREFIX_PATH}/lib"
		ENV LIB
	REQUIRED
)

target_include_directories(gzip PRIVATE "${ZLIB_INCLUDE_DIR}")
target_link_libraries(gzip PRIVATE "${ZLIB_LIBRARY}")

if(MSVC)
	target_compile_definitions(gzip PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

include(GNUInstallDirs)
install(TARGETS gzip RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
