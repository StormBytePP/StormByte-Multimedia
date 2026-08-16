set(VORBIS_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
ensure_build_dir(VORBIS_BUILD_DIR)
set(VORBIS_COMPONENT "Vorbis codec")

set(VORBIS_OPTIONS
	"-DBUILD_TESTING=OFF"
)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64|i[3-6]86|x86)$")
	list(APPEND VORBIS_OPTIONS "-DCMAKE_ASM_NASM_FLAGS=-DPIC -w-pp-macro-params-legacy")
endif()

create_cmake_dependant_component(
	VORBIS_LIBRARY_CREATE_FILE
	"vorbis"
	"${VORBIS_COMPONENT}"
	"${VORBIS_SRC_DIR}"
	"${VORBIS_BUILD_DIR}"
	"${VORBIS_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"vorbis;vorbisenc;vorbisfile"
	"ogg_install"
)
include("${VORBIS_LIBRARY_CREATE_FILE}")