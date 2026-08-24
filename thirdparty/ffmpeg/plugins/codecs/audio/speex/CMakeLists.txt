set(SPEEX_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake-port")
ensure_build_dir(SPEEX_BUILD_DIR)
set(SPEEX_COMPONENT "speex")

set(SPEEX_OPTIONS "")

create_cmake_component(
	SPEEX_LIBRARY_CREATE_FILE
	"speex"
	"${SPEEX_COMPONENT}"
	"${SPEEX_SRC_DIR}"
	"${SPEEX_BUILD_DIR}"
	"${SPEEX_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"speex"
	${PLUGIN_LEVEL}
)
include("${SPEEX_LIBRARY_CREATE_FILE}")

register_plugin(speex "-Dlibspeex=enabled")