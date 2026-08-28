set(ZLIB_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(ZLIB_COMPONENT "zlib compression library")

set(ZLIB_OPTIONS
	"-DZLIB_BUILD_EXAMPLES=OFF"
	"-DZLIB_BUILD_TESTING=OFF"
	"-DBUILD_SHARED_LIBS=OFF"
)

if(FFMPEG_BUNDLED_TYPE STREQUAL "shared")
	list(APPEND ZLIB_OPTIONS "-DBUILD_SHARED_LIBS=ON")
endif()

buildmaster_component(
	zlib
	"${ZLIB_COMPONENT}"
	"${ZLIB_SRC_DIR}"
	"${ZLIB_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"z"
	""
)
buildmaster_group_add(ff-compressors zlib)
register_plugin(zlib "zlib=enabled")
