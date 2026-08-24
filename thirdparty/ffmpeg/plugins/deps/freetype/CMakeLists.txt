set(FREETYPE_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
ensure_build_dir(FREETYPE_BUILD_DIR)
set(FREETYPE_COMPONENT "freetype")

set(FREETYPE_OPTIONS
	"-DFT_DISABLE_ZLIB=OFF"
	"-DFT_DISABLE_BZIP2=OFF"
	"-DFT_DISABLE_PNG=ON"
	"-DFT_DISABLE_HARFBUZZ=ON"
	"-DFT_DISABLE_BROTLI=ON"
	"-DBUILD_SHARED_LIBS=OFF"
	"-DDISABLE_FORCE_DEBUG_POSTFIX=ON"
)

if(FFMPEG_BUNDLED_TYPE STREQUAL "shared")
	list(APPEND FREETYPE_OPTIONS "-DBUILD_SHARED_LIBS=ON")
endif()

create_cmake_component(
	FREETYPE_LIBRARY_CREATE_FILE
	"freetype"
	"${FREETYPE_COMPONENT}"
	"${FREETYPE_SRC_DIR}"
	"${FREETYPE_BUILD_DIR}"
	"${FREETYPE_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"freetype"
	${PLUGIN_LEVEL}
)
include("${FREETYPE_LIBRARY_CREATE_FILE}")

# MSVC: static freetype embeds ftver.rc → LNK1241 when WHOLEARCHIVE with other .res libs
if(MSVC AND FFMPEG_BUNDLED_TYPE STREQUAL "static")
	add_custom_command(TARGET freetype_install POST_BUILD
		COMMAND ${CMAKE_COMMAND}
			-DLIB=${BUILDMASTER_INSTALL_LIBDIR}/freetype.lib
			-DMEMBER=CMakeFiles\\freetype.dir\\src\\base\\ftver.rc.res
			-P "${FFMPEG_STRIP_MSVC_RES}"
		COMMENT "MSVC: strip version resource from freetype.lib (avoid LNK1241)"
		VERBATIM
	)
endif()

register_plugin(freetype "-Dlibfreetype=enabled")