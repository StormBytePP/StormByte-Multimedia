plugin_require(WITH_GPL "-Dlibvidstab=disabled -Dvidstabdetect_filter=disabled -Dvidstabtransform_filter=disabled")

set(VIDSTAB_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
ensure_build_dir(VIDSTAB_BUILD_DIR)

set(VIDSTAB_COMPONENT "vidstab")
set(VIDSTAB_OPTIONS
	"-DUSE_OMP=OFF"          # evita dependencia de OpenMP
	"-DCMAKE_POLICY_VERSION_MINIMUM=3.5"
)

create_cmake_component(
	VIDSTAB_LIBRARY_CREATE_FILE
	"vidstab"
	"${VIDSTAB_COMPONENT}"
	"${VIDSTAB_SRC_DIR}"
	"${VIDSTAB_BUILD_DIR}"
	"${VIDSTAB_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"vidstab"
	${PLUGIN_LEVEL}
)
include("${VIDSTAB_LIBRARY_CREATE_FILE}")

register_plugin(vidstab "-Dlibvidstab=enabled -Dvidstabdetect_filter=enabled -Dvidstabtransform_filter=enabled")