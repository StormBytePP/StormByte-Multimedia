set(JPEG_TURBO_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(JPEG_TURBO_COMPONENT "libjpeg-turbo")
set(JPEG_TURBO_PATCHES
	"${CMAKE_CURRENT_LIST_DIR}/0001-cmake-pc-match-installed-jpeg-stem.patch"
)

set(JPEG_TURBO_OPTIONS
	"-DENABLE_SHARED=OFF"
	"-DENABLE_STATIC=ON"
	"-DWITH_TURBOJPEG=OFF"
	"-DWITH_JAVA=OFF"
	"-DWITH_TOOLS=OFF"
	"-DWITH_TESTS=OFF"
	"-DWITH_JPEG8=ON"
)

# libjpeg-turbo on MSVC static installs jpeg-static.lib and writes that
# path into libjpeg-turboTargets.cmake. BuildMaster RENAME follows PRODUCED:
#   PRODUCED=jpeg  → looks for jpeg.lib, then may rewrite jpeg-static.lib
#                     to jpeg.lib. The Config still names jpeg-static.lib
#                     → tiff/leptonica find_package dies ("file does not exist").
#   PRODUCED=jpeg-static → RENAME is a no-op on the real artifact, Config
#                     stays honest, FindJPEG already searches jpeg-static,
#                     buildmaster_link passes jpeg-static to MSVC.
# Do not "simplify" this back to jpeg on WIN32 static.
if(FFMPEG_BUNDLED_TYPE STREQUAL "shared")
	list(APPEND JPEG_TURBO_OPTIONS "-DENABLE_SHARED=ON" "-DENABLE_STATIC=OFF")
	set(JPEG_TURBO_PRODUCED "jpeg")
elseif(WIN32)
	set(JPEG_TURBO_PRODUCED "jpeg-static")
else()
	set(JPEG_TURBO_PRODUCED "jpeg")
endif()

buildmaster_component(
	jpeg-turbo
	"${JPEG_TURBO_COMPONENT}"
	"${JPEG_TURBO_SRC_DIR}"
	"${JPEG_TURBO_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"${JPEG_TURBO_PRODUCED}"
	"GIT={RESET;PATCH={${JPEG_TURBO_PATCHES}};TITLE=libjpeg-turbo pkg-config}"
)
buildmaster_group_add(ff-deps jpeg-turbo)
register_dependency(jpeg-turbo)
