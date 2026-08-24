cmake_minimum_required(VERSION 3.16)
project(speex C)

# Upstream: cmake-port/src/  (libspeex/ + include/)
set(SPEEX_UPSTREAM "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(SPEEX_LIBDIR   "${SPEEX_UPSTREAM}/libspeex")
set(SPEEX_INCDIR   "${SPEEX_UPSTREAM}/include")

option(BUILD_SHARED_LIBS "Build shared library" OFF)

set(SPEEX_SOURCES
	${SPEEX_LIBDIR}/bits.c
	${SPEEX_LIBDIR}/cb_search.c
	${SPEEX_LIBDIR}/exc_10_16_table.c
	${SPEEX_LIBDIR}/exc_10_32_table.c
	${SPEEX_LIBDIR}/exc_20_32_table.c
	${SPEEX_LIBDIR}/exc_5_256_table.c
	${SPEEX_LIBDIR}/exc_5_64_table.c
	${SPEEX_LIBDIR}/exc_8_128_table.c
	${SPEEX_LIBDIR}/filters.c
	${SPEEX_LIBDIR}/gain_table.c
	${SPEEX_LIBDIR}/gain_table_lbr.c
	${SPEEX_LIBDIR}/hexc_10_32_table.c
	${SPEEX_LIBDIR}/hexc_table.c
	${SPEEX_LIBDIR}/high_lsp_tables.c
	${SPEEX_LIBDIR}/lpc.c
	${SPEEX_LIBDIR}/lsp.c
	${SPEEX_LIBDIR}/lsp_tables_nb.c
	${SPEEX_LIBDIR}/ltp.c
	${SPEEX_LIBDIR}/modes.c
	${SPEEX_LIBDIR}/modes_wb.c
	${SPEEX_LIBDIR}/nb_celp.c
	${SPEEX_LIBDIR}/quant_lsp.c
	${SPEEX_LIBDIR}/sb_celp.c
	${SPEEX_LIBDIR}/smallft.c
	${SPEEX_LIBDIR}/speex.c
	${SPEEX_LIBDIR}/speex_callbacks.c
	${SPEEX_LIBDIR}/speex_header.c
	${SPEEX_LIBDIR}/stereo.c
	${SPEEX_LIBDIR}/vbr.c
	${SPEEX_LIBDIR}/vq.c
	${SPEEX_LIBDIR}/window.c
)

# Minimal config.h (floating-point, smallft, no SpeexDSP).
# MSVC has no C99 VLAs: stack_alloc.h uses #ifdef VAR_ARRAYS (defined-or-not),
# so VAR_ARRAYS must NOT be defined at all; use USE_ALLOCA instead.
# -DVAR_ARRAYS=0 is wrong: #ifdef is still true.
if(MSVC)
	file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/config.h"
"#pragma once
#define EXPORT
#define FLOATING_POINT
#define USE_SMALLFT
#define USE_ALLOCA
")
else()
	file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/config.h"
"#pragma once
#define EXPORT
#define FLOATING_POINT
#define USE_SMALLFT
#define VAR_ARRAYS
")
endif()

# speex_types.h → #include "speex_config_types.h"
# Must be findable via -I .../speex
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/speex")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/speex/speex_config_types.h"
"#pragma once
typedef short spx_int16_t;
typedef unsigned short spx_uint16_t;
typedef int spx_int32_t;
typedef unsigned int spx_uint32_t;
")

add_library(speex ${SPEEX_SOURCES})
set_target_properties(speex PROPERTIES
	OUTPUT_NAME speex
	POSITION_INDEPENDENT_CODE ON
	VERSION 1.2.1
	SOVERSION 1
)

target_include_directories(speex
	PUBLIC
		$<BUILD_INTERFACE:${SPEEX_INCDIR}>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/speex>
		$<INSTALL_INTERFACE:include>
	PRIVATE
		${SPEEX_LIBDIR}
		${CMAKE_CURRENT_BINARY_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/speex
)

target_compile_definitions(speex PRIVATE HAVE_CONFIG_H)

if(MSVC)
	target_compile_definitions(speex PRIVATE
		_CRT_SECURE_NO_WARNINGS
		USE_ALLOCA
	)
	# Do NOT define VAR_ARRAYS (even as 0): stack_alloc.h uses #ifdef
else()
	find_library(MATH_LIBRARY m)
	if(MATH_LIBRARY)
		target_link_libraries(speex PRIVATE ${MATH_LIBRARY})
	endif()
endif()

include(GNUInstallDirs)

install(TARGETS speex
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY ${SPEEX_INCDIR}/speex
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	FILES_MATCHING PATTERN "*.h"
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/speex/speex_config_types.h"
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/speex
)

set(prefix "${CMAKE_INSTALL_PREFIX}")
set(exec_prefix "\${prefix}")
set(libdir "\${exec_prefix}/${CMAKE_INSTALL_LIBDIR}")
set(includedir "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
set(VERSION "1.2.1")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/speex.pc"
"prefix=${prefix}
exec_prefix=\${prefix}
libdir=\${exec_prefix}/${CMAKE_INSTALL_LIBDIR}
includedir=\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}

Name: speex
Description: Speex audio codec
Version: ${VERSION}
Libs: -L\${libdir} -lspeex
Libs.private: -lm
Cflags: -I\${includedir}
")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/speex.pc"
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)