plugin_require(WITH_GPL "-Dlibxvid=disabled -Dlibxvid_encoder=disabled")

set(XVID_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake-port")
ensure_build_dir(XVID_BUILD_DIR)
set(XVID_COMPONENT "XVID codec")

set(XVID_OPTIONS "")
# NASM is x86-only; xvidcore ASM is classic x86 — do not pass NASM flags on ARM
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64|i[3-6]86|x86)$")
	list(APPEND XVID_OPTIONS "-DCMAKE_ASM_NASM_FLAGS=-DPIC -w-pp-macro-params-legacy")
endif()

create_git_reset_file(
	XVID_GIT_RESET_FILE
	xvidcore
	"XVID reset"
	"${XVID_SRC_DIR}/src"
)
include("${XVID_GIT_RESET_FILE}")
create_git_patch_file(
	XVID_GIT_PATCH_FILE
	xvidcore
	"XVID bool typedef patch"
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
	${PLUGIN_LEVEL}
)
include("${XVID_LIBRARY_CREATE_FILE}")

# Post-install git reset is handled automatically by BuildMaster (component id: xvidcore)

register_plugin(xvidcore "-Dlibxvid=enabled -Dlibxvid_encoder=enabled")