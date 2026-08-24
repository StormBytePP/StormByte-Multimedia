plugin_require(WITH_GPL "-Dlibrubberband=disabled")

message_indented("Setting up rubberband" ${PLUGIN_LEVEL})

set(RUBBERBAND_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
ensure_build_dir(RUBBERBAND_BUILD_DIR)
set(RUBBERBAND_COMPONENT "rubberband")

set(RUBBERBAND_PATCH_FILE "${CMAKE_CURRENT_LIST_DIR}/0001-mathmisc-stddef.patch")
create_git_reset_file(RUBBERBAND_RESET_REPO rubberband "rubberband reset" "${RUBBERBAND_SRC_DIR}")
create_git_patch_file(RUBBERBAND_PATCH_REPO rubberband "rubberband libc++ size_t patch" "${RUBBERBAND_SRC_DIR}" "${RUBBERBAND_PATCH_FILE}")
include("${RUBBERBAND_RESET_REPO}")
include("${RUBBERBAND_PATCH_REPO}")

set(RUBBERBAND_OPTIONS
	"-Dresampler=libsamplerate"
	"-Dfft=builtin"
	"-Djni=disabled"
	"-Dladspa=disabled"
	"-Dlv2=disabled"
	"-Dvamp=disabled"
	"-Dcmdline=disabled"
	"-Dtests=disabled"
)

create_meson_dependant_component(
	RUBBERBAND_LIBRARY_CREATE_FILE
	"rubberband"
	"${RUBBERBAND_COMPONENT}"
	"${RUBBERBAND_SRC_DIR}"
	"${RUBBERBAND_BUILD_DIR}"
	"${RUBBERBAND_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"rubberband"
	"samplerate_install"
	${PLUGIN_LEVEL}
)
include("${RUBBERBAND_LIBRARY_CREATE_FILE}")

if(MSVC AND FFMPEG_BUNDLED_TYPE STREQUAL "static")
	add_custom_command(TARGET rubberband_install POST_BUILD
		COMMAND ${CMAKE_COMMAND}
			-DDIR=${BUILDMASTER_INSTALL_LIBDIR}
			-DPATTERN=librubberband.a
			-DOUT=rubberband.lib
			-DMODE=copy
			-P "${FFMPEG_RENAME_MSVC_LIB}"
		COMMENT "MSVC: librubberband.a → rubberband.lib (copy)"
	)
endif()

register_plugin(rubberband "-Dlibrubberband=enabled")