set(VVENC_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
ensure_build_dir(VVENC_BUILD_DIR)
set(VVENC_COMPONENT "VVenc (H266) codec")

set(VVENC_OPTIONS
	"-DVVENC_LIBRARY_ONLY=ON"
)
# NASM flags only on x86 / x86_64 (ARM uses other SIMD paths)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64|i[3-6]86|x86)$")
	list(APPEND VVENC_OPTIONS "-DCMAKE_ASM_NASM_FLAGS=-DPIC -w-pp-macro-params-legacy")
endif()

create_cmake_component(
	VVENC_LIBRARY_CREATE_FILE
	"vvenc"
	"${VVENC_COMPONENT}"
	"${VVENC_SRC_DIR}"
	"${VVENC_BUILD_DIR}"
	"${VVENC_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"vvenc"
	${PLUGIN_LEVEL}
)
include("${VVENC_LIBRARY_CREATE_FILE}")

register_plugin(vvenc "-Dlibvvenc=enabled -Dlibvvenc_encoder=enabled")
