set(WEBP_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
ensure_build_dir(WEBP_BUILD_DIR)
set(WEBP_COMPONENT "libwebp")

set(WEBP_OPTIONS
	"-DWEBP_BUILD_ANIM_UTILS=OFF"
	"-DWEBP_BUILD_CWEBP=OFF"
	"-DWEBP_BUILD_DWEBP=OFF"
	"-DWEBP_BUILD_GIF2WEBP=OFF"
	"-DWEBP_BUILD_IMG2WEBP=OFF"
	"-DWEBP_BUILD_VWEBP=OFF"
	"-DWEBP_BUILD_WEBPINFO=OFF"
	"-DWEBP_BUILD_WEBPMUX=OFF"
	"-DWEBP_BUILD_EXTRAS=OFF"
	"-DWEBP_BUILD_TESTS=OFF"
	"-DBUILD_SHARED_LIBS=OFF"
)

if(FFMPEG_BUNDLED_TYPE STREQUAL "shared")
	list(APPEND WEBP_OPTIONS "-DBUILD_SHARED_LIBS=ON")
endif()

create_cmake_component(
	WEBP_LIBRARY_CREATE_FILE
	"webp"
	"${WEBP_COMPONENT}"
	"${WEBP_SRC_DIR}"
	"${WEBP_BUILD_DIR}"
	"${WEBP_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"webp;webpdemux;webpmux;sharpyuv"
	${PLUGIN_LEVEL}
)
include("${WEBP_LIBRARY_CREATE_FILE}")

# CMake/libwebp installs libwebp*.lib; DSL stamps expect webp.lib / webpdemux.lib / webpmux.lib
if(FFMPEG_BUNDLED_TYPE STREQUAL "static" AND MSVC)
	add_custom_command(TARGET webp_install POST_BUILD
		COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rm -f
			"${BUILDMASTER_INSTALL_LIBDIR}/webp.lib"
			"${BUILDMASTER_INSTALL_LIBDIR}/webpdemux.lib"
			"${BUILDMASTER_INSTALL_LIBDIR}/webpmux.lib"
			"${BUILDMASTER_INSTALL_LIBDIR}/webpdecoder.lib"
			"${BUILDMASTER_INSTALL_LIBDIR}/sharpyuv.lib"
		COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename
			"${BUILDMASTER_INSTALL_LIBDIR}/libwebp.lib"
			"${BUILDMASTER_INSTALL_LIBDIR}/webp.lib"
		COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename
			"${BUILDMASTER_INSTALL_LIBDIR}/libwebpdemux.lib"
			"${BUILDMASTER_INSTALL_LIBDIR}/webpdemux.lib"
		COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename
			"${BUILDMASTER_INSTALL_LIBDIR}/libwebpmux.lib"
			"${BUILDMASTER_INSTALL_LIBDIR}/webpmux.lib"
		COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename
			"${BUILDMASTER_INSTALL_LIBDIR}/libwebpdecoder.lib"
			"${BUILDMASTER_INSTALL_LIBDIR}/webpdecoder.lib"
		COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename
			"${BUILDMASTER_INSTALL_LIBDIR}/libsharpyuv.lib"
			"${BUILDMASTER_INSTALL_LIBDIR}/sharpyuv.lib"
		COMMENT "Renaming libwebp* libraries for MSVC"
	)
endif()

register_plugin(webp "-Dlibwebp=enabled")