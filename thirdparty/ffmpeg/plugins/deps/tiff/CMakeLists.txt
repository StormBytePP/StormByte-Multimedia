set(TIFF_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(TIFF_COMPONENT "libtiff")

set(TIFF_OPTIONS
	"-DBUILD_SHARED_LIBS=OFF"
	"-Dtiff-tools=OFF"
	"-Dtiff-tests=OFF"
	"-Dtiff-contrib=OFF"
	"-Dtiff-docs=OFF"
	# tiffxx (tif_stream.cxx) is the C++ stream wrapper. Leptonica /
	# tesseract / FFmpeg consume the C API only. Leave this OFF.
	"-Dtiff-cxx=OFF"
	"-Djpeg=ON"
	"-Dold-jpeg=OFF"
	"-Djbig=OFF"
	"-Dlzma=OFF"
	"-Dzstd=OFF"
	"-Dwebp=OFF"
	"-Dlibdeflate=OFF"
)
if(FFMPEG_BUNDLED_TYPE STREQUAL "shared")
	list(APPEND TIFF_OPTIONS "-DBUILD_SHARED_LIBS=ON")
endif()

buildmaster_component(
	tiff
	"${TIFF_COMPONENT}"
	"${TIFF_SRC_DIR}"
	"${TIFF_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"tiff"
	""
)
buildmaster_link(tiff zlib)
buildmaster_link(tiff jpeg-turbo)
buildmaster_group_add(ff-deps tiff)
register_dependency(tiff)
