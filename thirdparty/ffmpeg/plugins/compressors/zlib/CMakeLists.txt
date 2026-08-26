set(ZLIB_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
ensure_build_dir(ZLIB_BUILD_DIR)
set(ZLIB_COMPONENT "zlib compression library")

set(ZLIB_OPTIONS
	"-DZLIB_BUILD_EXAMPLES=OFF"
	"-DZLIB_BUILD_TESTING=OFF"
	"-DBUILD_SHARED_LIBS=OFF"
)

if(FFMPEG_BUNDLED_TYPE STREQUAL "shared")
	list(APPEND ZLIB_OPTIONS "-DBUILD_SHARED_LIBS=ON")
endif()

create_cmake_component(
	ZLIB_LIBRARY_CREATE_FILE
	"zlib"
	"${ZLIB_COMPONENT}"
	"${ZLIB_SRC_DIR}"
	"${ZLIB_BUILD_DIR}"
	"${ZLIB_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"z"
	${PLUGIN_LEVEL}
)
include("${ZLIB_LIBRARY_CREATE_FILE}")

if(MSVC AND FFMPEG_BUNDLED_TYPE STREQUAL "static")
	# FindZLIB / Leptonica export expect zlib.lib; upstream often installs z.lib
	add_custom_command(TARGET zlib_install POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E rm -f "${BUILDMASTER_INSTALL_LIBDIR}/zlib.lib"
		COMMAND ${CMAKE_COMMAND}
			-DDIR=${BUILDMASTER_INSTALL_LIBDIR}
			-DPATTERN=z.lib
			-DOUT=zlib.lib
			-DMODE=copy
			-P "${FFMPEG_RENAME_MSVC_LIB}"
		COMMENT "Normalize zlib.lib for FindZLIB (MSVC)"
	)
endif()

register_plugin(zlib "-Dzlib=enabled")