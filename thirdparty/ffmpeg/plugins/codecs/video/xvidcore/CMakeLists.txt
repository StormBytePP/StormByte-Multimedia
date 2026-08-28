plugin_require(WITH_GPL "libxvid=disabled libxvid_encoder=disabled")

set(XVID_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake-port")
set(XVID_COMPONENT "XVID codec")

set(XVID_OPTIONS "")
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64|i[3-6]86|x86)$")
	list(APPEND XVID_OPTIONS "-DCMAKE_ASM_NASM_FLAGS=-DPIC -w-pp-macro-params-legacy")
endif()

buildmaster_component(
	xvidcore
	"${XVID_COMPONENT}"
	"${XVID_SRC_DIR}"
	"${XVID_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"xvidcore"
	"GIT={RESET;PATCH=${CMAKE_CURRENT_LIST_DIR}/0001-Guard-old-typedef-bool.patch;TITLE=XVID bool typedef;ROOT=src}"
)
buildmaster_group_add(ff-video xvidcore)
register_plugin(xvidcore "libxvid=enabled libxvid_encoder=enabled")
