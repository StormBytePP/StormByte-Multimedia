set(TIFF_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
ensure_build_dir(TIFF_BUILD_DIR)
set(TIFF_COMPONENT "libtiff")

set(TIFF_OPTIONS
	"-DBUILD_SHARED_LIBS=OFF"
	"-Dtiff-tools=OFF"
	"-Dtiff-tests=OFF"
	"-Dtiff-contrib=OFF"
	"-Dtiff-docs=OFF"
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

create_cmake_dependant_component(
	TIFF_LIBRARY_CREATE_FILE
	"tiff"
	"${TIFF_COMPONENT}"
	"${TIFF_SRC_DIR}"
	"${TIFF_BUILD_DIR}"
	"${TIFF_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"tiff"
	"zlib_install;jpeg-turbo_install"
	${PLUGIN_LEVEL}
)
include("${TIFF_LIBRARY_CREATE_FILE}")

if(MSVC AND FFMPEG_BUNDLED_TYPE STREQUAL "static")
	# Exact member name from: lib /LIST tiff.lib
	add_custom_command(TARGET tiff_install POST_BUILD
		COMMAND ${CMAKE_COMMAND}
			-DLIB=${BUILDMASTER_INSTALL_LIBDIR}/tiff.lib
			-DMEMBER=libtiff\\CMakeFiles\\tiff.dir\\tif_win32_versioninfo.rc.res
			-P "${FFMPEG_STRIP_MSVC_RES}"
		COMMENT "Strip Windows version .res from tiff.lib"
	)
endif()

register_dependency(tiff)