message_indented("Setting up libass" ${PLUGIN_LEVEL})

set(ASS_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
ensure_build_dir(ASS_BUILD_DIR)
set(ASS_COMPONENT "libass")

set(ASS_OPTIONS
	"-Dtest=disabled"
	"-Dcompare=disabled"
	"-Dprofile=disabled"
)

if(UNIX AND NOT APPLE)
	list(APPEND ASS_OPTIONS
		"-Dfontconfig=enabled"
	)
	set(ASS_DEPS "freetype_install;harfbuzz_install;fribidi_install;fontconfig_install")
else()
	# Windows: DirectWrite | macOS: Core Text
	list(APPEND ASS_OPTIONS
		"-Dfontconfig=disabled"
		"-Drequire-system-font-provider=false"
	)
	set(ASS_DEPS "freetype_install;harfbuzz_install;fribidi_install")
endif()

create_meson_dependant_component(
	ASS_LIBRARY_CREATE_FILE
	"ass"
	"${ASS_COMPONENT}"
	"${ASS_SRC_DIR}"
	"${ASS_BUILD_DIR}"
	"${ASS_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"ass"
	"${ASS_DEPS}"
	${PLUGIN_LEVEL}
)
include("${ASS_LIBRARY_CREATE_FILE}")

if(MSVC AND FFMPEG_BUNDLED_TYPE STREQUAL "static")
	add_custom_command(TARGET ass_install POST_BUILD
		COMMAND ${CMAKE_COMMAND}
			-DDIR=${BUILDMASTER_INSTALL_LIBDIR}
			-DPATTERN=libass.a
			-DOUT=ass.lib
			-DMODE=copy
			-P "${FFMPEG_RENAME_MSVC_LIB}"
		COMMENT "MSVC: libass.a → ass.lib (copy)"
	)
endif()

register_plugin(ass "-Dlibass=enabled")