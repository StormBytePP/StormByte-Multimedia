set(GSM_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake-port")
ensure_build_dir(GSM_BUILD_DIR)
set(GSM_COMPONENT "gsm")

set(GSM_OPTIONS "")

create_cmake_component(
	GSM_LIBRARY_CREATE_FILE
	"gsm"
	"${GSM_COMPONENT}"
	"${GSM_SRC_DIR}"
	"${GSM_BUILD_DIR}"
	"${GSM_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"gsm"
	${PLUGIN_LEVEL}
)
include("${GSM_LIBRARY_CREATE_FILE}")

register_plugin(gsm "-Dlibgsm=enabled")