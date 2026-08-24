cmake_minimum_required(VERSION 3.16)
project(gsm C)

# Upstream layout: cmake-port/src/{src,inc}/
set(GSM_UPSTREAM "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(GSM_SRC      "${GSM_UPSTREAM}/src")
set(GSM_INC      "${GSM_UPSTREAM}/inc")

option(BUILD_SHARED_LIBS "Build shared library" OFF)

set(GSM_SOURCES
	${GSM_SRC}/add.c
	${GSM_SRC}/code.c
	${GSM_SRC}/debug.c
	${GSM_SRC}/decode.c
	${GSM_SRC}/long_term.c
	${GSM_SRC}/lpc.c
	${GSM_SRC}/preprocess.c
	${GSM_SRC}/rpe.c
	${GSM_SRC}/gsm_destroy.c
	${GSM_SRC}/gsm_decode.c
	${GSM_SRC}/gsm_encode.c
	${GSM_SRC}/gsm_explode.c
	${GSM_SRC}/gsm_implode.c
	${GSM_SRC}/gsm_create.c
	${GSM_SRC}/gsm_print.c
	${GSM_SRC}/gsm_option.c
	${GSM_SRC}/short_term.c
	${GSM_SRC}/table.c
)

add_library(gsm ${GSM_SOURCES})
set_target_properties(gsm PROPERTIES
	OUTPUT_NAME gsm
	POSITION_INDEPENDENT_CODE ON
	VERSION 1.0.17
	SOVERSION 1
)

target_include_directories(gsm
	PUBLIC
		$<BUILD_INTERFACE:${GSM_INC}>
		$<INSTALL_INTERFACE:include>
	PRIVATE
		${GSM_INC}
)

# Matches classic Makefile defaults used by FFmpeg builds
target_compile_definitions(gsm PRIVATE
	SASR
	WAV49
	NeedFunctionPrototypes=1
)

if(MSVC)
	target_compile_definitions(gsm PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
	target_compile_options(gsm PRIVATE -Wall)
endif()

include(GNUInstallDirs)

install(TARGETS gsm
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# FFmpeg has libavcodec/gsm.h; install toast header as gsm/gsm.h so
# #include <gsm.h> is not used (HAVE_GSM_H=0 → #include <gsm/gsm.h>).
install(FILES ${GSM_INC}/gsm.h
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gsm
)

# pkg-config (FFmpeg looks for gsm / -lgsm)
set(prefix "${CMAKE_INSTALL_PREFIX}")
set(exec_prefix "\${prefix}")
set(libdir "\${exec_prefix}/${CMAKE_INSTALL_LIBDIR}")
set(includedir "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
set(VERSION "1.0.17")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/gsm.pc"
"prefix=${prefix}
exec_prefix=\${prefix}
libdir=\${exec_prefix}/${CMAKE_INSTALL_LIBDIR}
includedir=\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}

Name: gsm
Description: GSM 06.10 lossy speech compression
Version: ${VERSION}
Libs: -L\${libdir} -lgsm
Cflags: -I\${includedir}
")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/gsm.pc"
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
