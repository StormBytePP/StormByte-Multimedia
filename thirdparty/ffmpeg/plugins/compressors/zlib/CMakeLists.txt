set(ZLIB_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/src")
ensure_build_dir(ZLIB_BUILD_DIR)
set(ZLIB_COMPONENT "zlib compression library")

# madler/zlib options: static only, no tests/examples as tests
set(ZLIB_OPTIONS
	"-DZLIB_BUILD_TESTING=OFF"
	"-DZLIB_BUILD_SHARED=OFF"
	"-DZLIB_BUILD_STATIC=ON"
	"-DZLIB_INSTALL=ON"
)

create_cmake_component(
	ZLIB_LIBRARY_CREATE_FILE
	"zlib"
	"${ZLIB_COMPONENT}"
	"${ZLIB_SRC_DIR}"
	"${ZLIB_BUILD_DIR}"
	"${ZLIB_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"z"
	${PLUGIN_LEVEL}
)
include("${ZLIB_LIBRARY_CREATE_FILE}")

# FFmpeg Meson / pkg-config use -lz → z.lib on MSVC.
# Upstream: libzsd.lib (Debug) / libzs.lib (Release). Move, do not leave the old name.
if(MSVC)
	add_custom_command(TARGET zlib_install POST_BUILD
		COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rm -f
			"${BUILDMASTER_INSTALL_LIBDIR}/z.lib"
		COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename
			"${BUILDMASTER_INSTALL_LIBDIR}/$<IF:$<CONFIG:Debug>,libzsd.lib,libzs.lib>"
			"${BUILDMASTER_INSTALL_LIBDIR}/z.lib"
		COMMENT "MSVC: libzs[d].lib → z.lib for pkg-config -lz / FFmpeg"
	)
endif()

register_plugin(zlib "-Dzlib=enabled")