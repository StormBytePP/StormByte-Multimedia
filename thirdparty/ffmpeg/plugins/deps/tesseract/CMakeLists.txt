set(TESSERACT_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
ensure_build_dir(TESSERACT_BUILD_DIR)
set(TESSERACT_COMPONENT "tesseract")

get_filename_component(_BM_PREFIX "${BUILDMASTER_INSTALL_LIBDIR}" DIRECTORY)
set(_TESS_BRIDGE "${CMAKE_CURRENT_LIST_DIR}/leptonica_link_bridge.cmake")

set(TESSERACT_OPTIONS
	"-DBUILD_TRAINING_TOOLS=OFF"
	"-DBUILD_TESTS=OFF"
	"-DINSTALL_CONFIGS=ON"
	"-DDISABLE_CURL=ON"
	"-DDISABLE_ARCHIVE=ON"
	"-DCMAKE_PREFIX_PATH=${_BM_PREFIX}"
	"-DBM_INSTALL_PREFIX=${_BM_PREFIX}"
	"-DCMAKE_PROJECT_INCLUDE=${_TESS_BRIDGE}"
	"-DLeptonica_DIR=${BUILDMASTER_INSTALL_LIBDIR}/cmake/leptonica"
	"-DZLIB_ROOT=${_BM_PREFIX}"
	"-DPNG_ROOT=${_BM_PREFIX}"
	"-DJPEG_ROOT=${_BM_PREFIX}"
	"-DTIFF_ROOT=${_BM_PREFIX}"
	"-DZLIB_INCLUDE_DIR=${BUILDMASTER_INSTALL_INCLUDEDIR}"
	"-DENABLE_UNITY_BUILD=ON"
)

if(MSVC)
	list(APPEND TESSERACT_OPTIONS
		"-DZLIB_LIBRARY=${BUILDMASTER_INSTALL_LIBDIR}/zlib.lib"
		"-DPNG_LIBRARY=${BUILDMASTER_INSTALL_LIBDIR}/png.lib"
		"-DJPEG_LIBRARY=${BUILDMASTER_INSTALL_LIBDIR}/jpeg.lib"
		"-DTIFF_LIBRARY=${BUILDMASTER_INSTALL_LIBDIR}/tiff.lib"
		# Static libtiff → jpeg12_*; append jpeg at end of exe link line
		"-DCMAKE_CXX_STANDARD_LIBRARIES=${BUILDMASTER_INSTALL_LIBDIR}/jpeg.lib"
	)
else()
	list(APPEND TESSERACT_OPTIONS
		"-DJPEG_LIBRARY=${BUILDMASTER_INSTALL_LIBDIR}/libjpeg.a"
		"-DTIFF_LIBRARY=${BUILDMASTER_INSTALL_LIBDIR}/libtiff.a"
		"-DPNG_LIBRARY=${BUILDMASTER_INSTALL_LIBDIR}/libpng.a"
		"-DZLIB_LIBRARY=${BUILDMASTER_INSTALL_LIBDIR}/libz.a"
		# libtiff JPEG codec needs jpeg12_*; -ljpeg in LINKER_FLAGS is too early
		"-DCMAKE_CXX_STANDARD_LIBRARIES=${BUILDMASTER_INSTALL_LIBDIR}/libjpeg.a"
	)
endif()

if(FFMPEG_BUNDLED_TYPE STREQUAL "shared")
	list(APPEND TESSERACT_OPTIONS "-DBUILD_SHARED_LIBS=ON")
else()
	list(APPEND TESSERACT_OPTIONS "-DBUILD_SHARED_LIBS=OFF")
endif()

# leptonica → tiff/png/jpeg; jpeg-turbo_install ensures libjpeg exists before link
create_cmake_dependant_component(
	TESSERACT_LIBRARY_CREATE_FILE
	"tesseract"
	"${TESSERACT_COMPONENT}"
	"${TESSERACT_SRC_DIR}"
	"${TESSERACT_BUILD_DIR}"
	"${TESSERACT_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"tesseract"
	"leptonica_install;jpeg-turbo_install"
	${PLUGIN_LEVEL}
)
include("${TESSERACT_LIBRARY_CREATE_FILE}")

if(MSVC AND FFMPEG_BUNDLED_TYPE STREQUAL "static")
	# Upstream installs tesseract55.lib; empty tesseract.lib breaks /WHOLEARCHIVE
	add_custom_command(TARGET tesseract_install POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E rm -f "${BUILDMASTER_INSTALL_LIBDIR}/tesseract.lib"
		COMMAND ${CMAKE_COMMAND}
			-DDIR=${BUILDMASTER_INSTALL_LIBDIR}
			-DPATTERN=tesseract*.lib
			-DOUT=tesseract.lib
			-DMODE=copy
			-P "${FFMPEG_RENAME_MSVC_LIB}"
		COMMENT "Normalize tesseract.lib (MSVC static)"
	)
endif()

register_plugin(tesseract "-Dlibtesseract=enabled -Docr_filter=enabled")