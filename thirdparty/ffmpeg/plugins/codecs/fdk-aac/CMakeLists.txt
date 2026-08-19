plugin_require(WITH_NONFREE "-Dlibfdk_aac=disabled")

set(FDK_AAC_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/src")
ensure_build_dir(FDK_AAC_BUILD_DIR)
set(FDK_AAC_COMPONENT "FDK_AAC codec")

set(FDK_AAC_OPTIONS
	"-DBUILD_PROGRAMS=OFF"
	"-DFDK_AAC_INSTALL_CMAKE_CONFIG_MODULE=OFF"
	"-DFDK_AAC_INSTALL_PKGCONFIG_MODULE=ON"
)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64|i[3-6]86|x86)$")
	list(APPEND FDK_AAC_OPTIONS "-DCMAKE_ASM_NASM_FLAGS=-DPIC -w-pp-macro-params-legacy")
endif()

create_cmake_component(
	FDK_LIBRARY_CREATE_FILE
	"fdk-aac"
	"${FDK_AAC_COMPONENT}"
	"${FDK_AAC_SRC_DIR}"
	"${FDK_AAC_BUILD_DIR}"
	"${FDK_AAC_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"fdk-aac"
	${PLUGIN_LEVEL}
)
include("${FDK_LIBRARY_CREATE_FILE}")

register_plugin(fdk-aac "-Dlibfdk_aac=enabled")