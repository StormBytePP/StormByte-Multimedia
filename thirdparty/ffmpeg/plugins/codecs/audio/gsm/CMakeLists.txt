set(GSM_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake-port")
set(GSM_COMPONENT "gsm")

buildmaster_component(
	gsm
	"${GSM_COMPONENT}"
	"${GSM_SRC_DIR}"
	""
	"${FFMPEG_BUNDLED_TYPE}"
	"gsm"
	""
)
buildmaster_group_add(ff-audio gsm)
register_plugin(gsm "libgsm=enabled")
