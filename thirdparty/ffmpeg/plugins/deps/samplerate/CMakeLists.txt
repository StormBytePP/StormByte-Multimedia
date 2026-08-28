set(SAMPLERATE_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(SAMPLERATE_COMPONENT "libsamplerate")
set(SAMPLERATE_PATCH_FILE "${CMAKE_CURRENT_LIST_DIR}/0001-cmake4-minimum-version.patch")

set(SAMPLERATE_OPTIONS
	"-DLIBSAMPLERATE_EXAMPLES=OFF"
	"-DBUILD_TESTING=OFF"
	"-DBUILD_SHARED_LIBS=OFF"
)
if(FFMPEG_BUNDLED_TYPE STREQUAL "shared")
	list(APPEND SAMPLERATE_OPTIONS "-DBUILD_SHARED_LIBS=ON")
endif()

buildmaster_component(
	samplerate
	"${SAMPLERATE_COMPONENT}"
	"${SAMPLERATE_SRC_DIR}"
	"${SAMPLERATE_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"samplerate"
	"GIT={RESET;PATCH=${SAMPLERATE_PATCH_FILE};TITLE=samplerate CMake 4}"
)
buildmaster_group_add(ff-deps samplerate)
register_dependency(samplerate)
