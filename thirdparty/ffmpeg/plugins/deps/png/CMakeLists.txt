set(PNG_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
ensure_build_dir(PNG_BUILD_DIR)
set(PNG_COMPONENT "libpng")

set(PNG_OPTIONS
	"-DPNG_SHARED=OFF"
	"-DPNG_STATIC=ON"
	"-DPNG_TESTS=OFF"
	"-DPNG_EXECUTABLES=OFF"
	"-DPNG_FRAMEWORK=OFF"
	"-DPNG_BUILD_ZLIB=OFF"
)

if(FFMPEG_BUNDLED_TYPE STREQUAL "shared")
	list(APPEND PNG_OPTIONS
		"-DPNG_SHARED=ON"
		"-DPNG_STATIC=OFF"
	)
endif()

create_cmake_dependant_component(
	PNG_LIBRARY_CREATE_FILE
	"png"
	"${PNG_COMPONENT}"
	"${PNG_SRC_DIR}"
	"${PNG_BUILD_DIR}"
	"${PNG_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"png"
	"zlib_install"
	${PLUGIN_LEVEL}
)
include("${PNG_LIBRARY_CREATE_FILE}")

get_filename_component(_BM_PREFIX "${BUILDMASTER_INSTALL_LIBDIR}" DIRECTORY)

# lept.pc Requires: libpng — FFmpeg meson resolves tesseract via pkg-config
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/libpng.pc"
"prefix=${_BM_PREFIX}
exec_prefix=\${prefix}
libdir=${BUILDMASTER_INSTALL_LIBDIR}
includedir=${BUILDMASTER_INSTALL_INCLUDEDIR}

Name: libpng
Description: PNG reference library (StormByte bundled)
Version: 1.6.58
Requires: zlib
Libs: -L\${libdir} -lpng
Libs.private: -lz
Cflags: -I\${includedir}
")

add_custom_command(TARGET png_install POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory
		"${BUILDMASTER_INSTALL_LIBDIR}/pkgconfig"
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"${CMAKE_CURRENT_BINARY_DIR}/libpng.pc"
		"${BUILDMASTER_INSTALL_LIBDIR}/pkgconfig/libpng.pc"
	COMMENT "Install libpng.pc for pkg-config (lept / FFmpeg)"
)

if(MSVC AND FFMPEG_BUNDLED_TYPE STREQUAL "static")
	# Real archive is libpng16_static.lib; empty png.lib breaks consumers
	add_custom_command(TARGET png_install POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E rm -f "${BUILDMASTER_INSTALL_LIBDIR}/png.lib"
		COMMAND ${CMAKE_COMMAND}
			-DDIR=${BUILDMASTER_INSTALL_LIBDIR}
			-DPATTERN=libpng16*_static*.lib
			-DOUT=png.lib
			-DMODE=copy
			-P "${FFMPEG_RENAME_MSVC_LIB}"
		COMMENT "Normalize png.lib (MSVC static)"
	)
endif()

register_dependency(png)