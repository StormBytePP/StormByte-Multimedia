set(FRIBIDI_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
ensure_build_dir(FRIBIDI_BUILD_DIR)
set(FRIBIDI_COMPONENT "fribidi")

set(FRIBIDI_OPTIONS
	"-Ddocs=false"
	"-Dtests=false"
	"-Dbin=false"
)

create_meson_component(
	FRIBIDI_LIBRARY_CREATE_FILE
	"fribidi"
	"${FRIBIDI_COMPONENT}"
	"${FRIBIDI_SRC_DIR}"
	"${FRIBIDI_BUILD_DIR}"
	"${FRIBIDI_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"fribidi"
	${PLUGIN_LEVEL}
)
include("${FRIBIDI_LIBRARY_CREATE_FILE}")

# Meson installs libfribidi.a; MSVC consumers expect fribidi.lib
if(FFMPEG_BUNDLED_TYPE STREQUAL "static" AND MSVC)
	add_custom_command(TARGET fribidi_install POST_BUILD
		COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename
			"${BUILDMASTER_INSTALL_LIBDIR}/libfribidi.a"
			"${BUILDMASTER_INSTALL_LIBDIR}/fribidi.lib"
		COMMENT "Renaming fribidi library for MSVC"
	)
endif()

register_plugin(fribidi "-Dlibfribidi=enabled")