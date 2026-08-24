message_indented("Setting up Vorbis codec" ${PLUGIN_LEVEL})

set(VORBIS_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
ensure_build_dir(VORBIS_BUILD_DIR)
set(VORBIS_COMPONENT "Vorbis codec")

set(VORBIS_PATCH_FILE "${CMAKE_CURRENT_LIST_DIR}/0001-cmake4-minimum-version.patch")
create_git_reset_file(VORBIS_RESET_REPO vorbis "Vorbis reset" "${VORBIS_SRC_DIR}")
create_git_patch_file(VORBIS_PATCH_REPO vorbis "Vorbis CMake 4 patch" "${VORBIS_SRC_DIR}" "${VORBIS_PATCH_FILE}")
include("${VORBIS_RESET_REPO}")
include("${VORBIS_PATCH_REPO}")

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

register_plugin(vorbis "-Dlibvorbis=enabled -Dlibvorbisenc=enabled")