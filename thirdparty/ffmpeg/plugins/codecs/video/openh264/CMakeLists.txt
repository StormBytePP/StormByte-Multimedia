set(OPENH264_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
ensure_build_dir(OPENH264_BUILD_DIR)
set(OPENH264_COMPONENT "openh264")

set(OPENH264_OPTIONS
	"-Dtests=disabled"
)

create_meson_component(
	OPENH264_LIBRARY_CREATE_FILE
	"openh264"
	"${OPENH264_COMPONENT}"
	"${OPENH264_SRC_DIR}"
	"${OPENH264_BUILD_DIR}"
	"${OPENH264_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"openh264"
	${PLUGIN_LEVEL}
)
include("${OPENH264_LIBRARY_CREATE_FILE}")

if(MSVC AND FFMPEG_BUNDLED_TYPE STREQUAL "static")
	add_custom_command(TARGET openh264_install POST_BUILD
		COMMAND ${CMAKE_COMMAND}
			-DDIR=${BUILDMASTER_INSTALL_LIBDIR}
			-DPATTERN=libopenh264.a
			-DOUT=openh264.lib
			-DMODE=copy
			-P "${FFMPEG_RENAME_MSVC_LIB}"
		COMMENT "MSVC: libopenh264.a → openh264.lib (copy)"
	)
endif()

register_plugin(openh264 "-Dlibopenh264=enabled")