set(LEPTONICA_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(LEPTONICA_COMPONENT "leptonica")

# Upstream MSVC OUTPUT_NAME is leptonica-${VERSION}. BM produced is
# "leptonica"; rename then moves the .lib and LeptonicaTargets.cmake
# still points at leptonica-1.87.0.lib → tesseract find_package dies.
# Patch forces OUTPUT_NAME=leptonica on every OS (no-op on Unix).
set(LEPTONICA_OPTIONS
	"-DSW_BUILD=OFF"
	"-DBUILD_PROG=OFF"
	"-DENABLE_ZLIB=ON"
	"-DENABLE_PNG=ON"
	"-DENABLE_JPEG=ON"
	"-DENABLE_TIFF=ON"
	"-DENABLE_GIF=OFF"
	"-DENABLE_WEBP=OFF"
	"-DENABLE_OPENJPEG=OFF"
	"-DSTRICT_CONF=ON"
)
if(FFMPEG_BUNDLED_TYPE STREQUAL "shared")
	list(APPEND LEPTONICA_OPTIONS "-DBUILD_SHARED_LIBS=ON")
else()
	list(APPEND LEPTONICA_OPTIONS "-DBUILD_SHARED_LIBS=OFF")
endif()

set(LEPTONICA_PATCHES
	${CMAKE_CURRENT_LIST_DIR}/0001-msvc-unversioned-output-name.patch
	${CMAKE_CURRENT_LIST_DIR}/0002-always-create-lept.pc.patch
	${CMAKE_CURRENT_LIST_DIR}/0003-config-find-dependency-image-libs.patch
)

buildmaster_component(
	leptonica
	"${LEPTONICA_COMPONENT}"
	"${LEPTONICA_SRC_DIR}"
	"${LEPTONICA_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"leptonica"
	"GIT={RESET;PATCH={${LEPTONICA_PATCHES}};TITLE=Leptonica patches}"
)
buildmaster_link(leptonica zlib)
buildmaster_link(leptonica png)
buildmaster_link(leptonica jpeg-turbo)
buildmaster_link(leptonica tiff)
buildmaster_group_add(ff-deps leptonica)
register_dependency(leptonica)
