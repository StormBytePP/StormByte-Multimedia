set(ZIMG_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake-port")
ensure_build_dir(ZIMG_BUILD_DIR)
set(ZIMG_COMPONENT "zimg")

set(ZIMG_OPTIONS
	"-DZIMG_ENABLE_SIMD=ON"
)

create_cmake_component(
	ZIMG_LIBRARY_CREATE_FILE
	"zimg"
	"${ZIMG_COMPONENT}"
	"${ZIMG_SRC_DIR}"
	"${ZIMG_BUILD_DIR}"
	"${ZIMG_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"zimg"
	${PLUGIN_LEVEL}
)
include("${ZIMG_LIBRARY_CREATE_FILE}")

register_plugin(zimg "-Dlibzimg=enabled -Dzscale_filter=enabled")