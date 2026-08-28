plugin_require(WITH_GPL "libvidstab=disabled vidstabdetect_filter=disabled vidstabtransform_filter=disabled")

set(VIDSTAB_PATCHES
	"${CMAKE_CURRENT_LIST_DIR}/0001-cmake-pc-no-libm-on-msvc.patch"
	"${CMAKE_CURRENT_LIST_DIR}/0002-api-VIDSTAB_STATIC-and-pkgconfig-cflags.patch"
)

set(VIDSTAB_OPTIONS
	"-DUSE_OMP=OFF"
	"-DCMAKE_POLICY_VERSION_MINIMUM=3.5"
)
if(FFMPEG_BUNDLED_TYPE STREQUAL "shared")
	list(APPEND VIDSTAB_OPTIONS "-DBUILD_SHARED_LIBS=ON")
else()
	list(APPEND VIDSTAB_OPTIONS "-DBUILD_SHARED_LIBS=OFF")
endif()

buildmaster_component(
	vidstab
	"vidstab"
	"${CMAKE_CURRENT_SOURCE_DIR}/src"
	"${VIDSTAB_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"vidstab"
	"GIT={RESET;PATCH={${VIDSTAB_PATCHES}};TITLE=vidstab pkg-config}"
)
buildmaster_group_add(ff-filters vidstab)
register_plugin(vidstab "libvidstab=enabled vidstabdetect_filter=enabled vidstabtransform_filter=enabled")
