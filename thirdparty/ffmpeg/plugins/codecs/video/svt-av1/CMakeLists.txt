set(SVTAV1_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(SVTAV1_COMPONENT "SVT-AV1")

set(SVTAV1_OPTIONS
	"-DBUILD_APPS=OFF"
	"-DBUILD_DEC=OFF"
	"-DBUILD_TESTING=OFF"
	"-DBUILD_SHARED_LIBS=OFF"
)
if(FFMPEG_BUNDLED_TYPE STREQUAL "shared")
	list(APPEND SVTAV1_OPTIONS "-DBUILD_SHARED_LIBS=ON")
endif()

buildmaster_component(
	svt-av1
	"${SVTAV1_COMPONENT}"
	"${SVTAV1_SRC_DIR}"
	"${SVTAV1_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"SvtAv1Enc"
	""
)
buildmaster_group_add(ff-video svt-av1)
register_plugin(svt-av1 "libsvtav1=enabled")
