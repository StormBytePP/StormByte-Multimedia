set(MP3LAME_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/cmake")
ensure_build_dir(MP3LAME_BUILD_DIR)
set(MP3LAME_COMPONENT "MP3 encoder LAME")

set(MP3LAME_OPTIONS "")
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64|i[3-6]86|x86)$")
	list(APPEND MP3LAME_OPTIONS "-DCMAKE_ASM_NASM_FLAGS=-DPIC -w-pp-macro-params-legacy")
endif()

create_cmake_component(
	MP3LAME_LIBRARY_CREATE_FILE
	"mp3lame"
	"${MP3LAME_COMPONENT}"
	"${MP3LAME_SRC_DIR}"
	"${MP3LAME_BUILD_DIR}"
	"${MP3LAME_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"mp3lame"
	3
)
include("${MP3LAME_LIBRARY_CREATE_FILE}")

register_plugin(mp3lame "-Dlibmp3lame=enabled")