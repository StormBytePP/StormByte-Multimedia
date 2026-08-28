set(FREETYPE_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
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

buildmaster_component(
	freetype
	"${FREETYPE_COMPONENT}"
	"${FREETYPE_SRC_DIR}"
	"${FREETYPE_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"freetype"
	"BACKEND=cmake"
)
buildmaster_link(freetype zlib)
buildmaster_link(freetype bzip2)
buildmaster_group_add(ff-deps freetype)
register_plugin(freetype "libfreetype=enabled")
