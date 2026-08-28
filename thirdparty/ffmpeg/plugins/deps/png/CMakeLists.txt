set(PNG_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(PNG_COMPONENT "libpng")
set(PNG_PATCHES
	"${CMAKE_CURRENT_LIST_DIR}/0001-cmake-always-emit-libpng.pc-including-MSVC.patch"
	"${CMAKE_CURRENT_LIST_DIR}/0002-cmake-unversioned-png-stem-and-no-self-symlink.patch"
	"${CMAKE_CURRENT_LIST_DIR}/0003-cmake-put-zlib-libm-on-public-Libs-for-Meson-1.10.patch"
)

set(PNG_OPTIONS
	"-DPNG_SHARED=OFF"
	"-DPNG_STATIC=ON"
	"-DPNG_TESTS=OFF"
	"-DPNG_EXECUTABLES=OFF"
	"-DPNG_FRAMEWORK=OFF"
	"-DPNG_BUILD_ZLIB=OFF"
)
if(FFMPEG_BUNDLED_TYPE STREQUAL "shared")
	list(APPEND PNG_OPTIONS "-DPNG_SHARED=ON" "-DPNG_STATIC=OFF")
endif()

buildmaster_component(
	png
	"${PNG_COMPONENT}"
	"${PNG_SRC_DIR}"
	"${PNG_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"png"
	"GIT={RESET;PATCH={${PNG_PATCHES}};TITLE=libpng pkg-config}"
)
buildmaster_link(png zlib)
buildmaster_group_add(ff-deps png)
register_dependency(png)
