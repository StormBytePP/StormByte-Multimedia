set(JPEG_TURBO_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
ensure_build_dir(JPEG_TURBO_BUILD_DIR)
set(JPEG_TURBO_COMPONENT "libjpeg-turbo")

set(JPEG_TURBO_OPTIONS
	"-DENABLE_SHARED=OFF"
	"-DENABLE_STATIC=ON"
	"-DWITH_TURBOJPEG=OFF"
	"-DWITH_JAVA=OFF"
	"-DWITH_TOOLS=OFF"
	"-DWITH_TESTS=OFF"
	"-DWITH_JPEG8=ON"
)

if(FFMPEG_BUNDLED_TYPE STREQUAL "shared")
	list(APPEND JPEG_TURBO_OPTIONS
		"-DENABLE_SHARED=ON"
		"-DENABLE_STATIC=OFF"
	)
endif()

create_cmake_component(
	JPEG_TURBO_LIBRARY_CREATE_FILE
	"jpeg-turbo"
	"${JPEG_TURBO_COMPONENT}"
	"${JPEG_TURBO_SRC_DIR}"
	"${JPEG_TURBO_BUILD_DIR}"
	"${JPEG_TURBO_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"jpeg"
	${PLUGIN_LEVEL}
)
include("${JPEG_TURBO_LIBRARY_CREATE_FILE}")

if(MSVC AND FFMPEG_BUNDLED_TYPE STREQUAL "static")
	# Real archive: jpeg-static.lib; empty jpeg.lib is a stub
	add_custom_command(TARGET jpeg-turbo_install POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E rm -f "${BUILDMASTER_INSTALL_LIBDIR}/jpeg.lib"
		COMMAND ${CMAKE_COMMAND}
			-DDIR=${BUILDMASTER_INSTALL_LIBDIR}
			-DPATTERN=jpeg-static*.lib
			-DOUT=jpeg.lib
			-DMODE=copy
			-P "${FFMPEG_RENAME_MSVC_LIB}"
		COMMENT "Normalize jpeg.lib (MSVC static)"
	)
endif()

register_dependency(jpeg-turbo)