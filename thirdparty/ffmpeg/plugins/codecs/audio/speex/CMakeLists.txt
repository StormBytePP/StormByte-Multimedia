set(SPEEX_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake-port")
set(SPEEX_COMPONENT "speex")

buildmaster_component(
	speex
	"${SPEEX_COMPONENT}"
	"${SPEEX_SRC_DIR}"
	""
	"${FFMPEG_BUNDLED_TYPE}"
	"speex"
	""
)
buildmaster_group_add(ff-audio speex)
register_plugin(speex "libspeex=enabled")
