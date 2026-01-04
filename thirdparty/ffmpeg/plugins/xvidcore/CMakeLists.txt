set(XVID_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake-port")
ensure_build_dir(XVID_BUILD_DIR)
set(XVID_COMPONENT "XVID codec")
set(XVID_OPTIONS
	"-DCMAKE_ASM_NASM_FLAGS=-DPIC -w-pp-macro-params-legacy"
)
create_git_reset_file(
	XVID_GIT_RESET_FILE
	"${XVID_COMPONENT}"
	"${XVID_SRC_DIR}/src"
)
include("${XVID_GIT_RESET_FILE}")
create_git_patch_file(
	XVID_GIT_PATCH_FILE
	"${XVID_COMPONENT}"
	"${XVID_SRC_DIR}/src"
	"${CMAKE_CURRENT_LIST_DIR}/0001-Guard-old-typedef-bool.patch"
)
include("${XVID_GIT_PATCH_FILE}")
create_cmake_component(
	XVID_LIBRARY_CREATE_FILE
	"xvidcore"
	"${XVID_COMPONENT}"
	"${XVID_SRC_DIR}"
	"${XVID_BUILD_DIR}"
	"${XVID_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"xvidcore"
	2
)
include("${XVID_LIBRARY_CREATE_FILE}")

# Reset repo after install
add_custom_command(TARGET xvidcore_install POST_BUILD
	COMMAND ${ENV_CMAKE_SILENT_COMMAND} -P "${XVID_GIT_RESET_FILE}"
	COMMENT "Resetting XVID source directory after installation"
)