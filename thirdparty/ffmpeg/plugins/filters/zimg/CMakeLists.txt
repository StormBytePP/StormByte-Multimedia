set(ZIMG_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake-port")
set(ZIMG_COMPONENT "zimg")

buildmaster_component(
	zimg
	"${ZIMG_COMPONENT}"
	"${ZIMG_SRC_DIR}"
	"-DZIMG_ENABLE_SIMD=ON"
	"${FFMPEG_BUNDLED_TYPE}"
	"zimg"
	""
)
buildmaster_group_add(ff-filters zimg)
register_plugin(zimg "libzimg=enabled zscale_filter=enabled")
