cmake_minimum_required(VERSION 3.20)
project(bzip2 C)

set(BZIP2_UPSTREAM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(BZIP2_LIB_SOURCES
	${BZIP2_UPSTREAM_DIR}/blocksort.c
	${BZIP2_UPSTREAM_DIR}/huffman.c
	${BZIP2_UPSTREAM_DIR}/crctable.c
	${BZIP2_UPSTREAM_DIR}/randtable.c
	${BZIP2_UPSTREAM_DIR}/compress.c
	${BZIP2_UPSTREAM_DIR}/decompress.c
	${BZIP2_UPSTREAM_DIR}/bzlib.c
)

add_library(bzip2 STATIC ${BZIP2_LIB_SOURCES})
set_target_properties(bzip2 PROPERTIES
	OUTPUT_NAME bz2
	POSITION_INDEPENDENT_CODE ON
)

target_include_directories(bzip2
	PUBLIC
		$<BUILD_INTERFACE:${BZIP2_UPSTREAM_DIR}>
		$<INSTALL_INTERFACE:include>
)
target_compile_definitions(bzip2 PRIVATE _FILE_OFFSET_BITS=64)

if(NOT MSVC)
	target_compile_options(bzip2 PRIVATE -Wall -Winline)
endif()

include(GNUInstallDirs)

install(TARGETS bzip2
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(FILES ${BZIP2_UPSTREAM_DIR}/bzlib.h
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
