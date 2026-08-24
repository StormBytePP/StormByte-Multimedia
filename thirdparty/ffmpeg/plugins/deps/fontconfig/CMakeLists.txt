message_indented("Setting up fontconfig" ${PLUGIN_LEVEL})

set(FONTCONFIG_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
ensure_build_dir(FONTCONFIG_BUILD_DIR)
set(FONTCONFIG_COMPONENT "fontconfig")

set(FONTCONFIG_OPTIONS
	"-Ddoc=disabled"
	"-Dnls=disabled"
	"-Dtests=disabled"
	"-Dtools=disabled"
	"-Dcache-build=disabled"
)

create_meson_dependant_component(
	FONTCONFIG_LIBRARY_CREATE_FILE
	"fontconfig"
	"${FONTCONFIG_COMPONENT}"
	"${FONTCONFIG_SRC_DIR}"
	"${FONTCONFIG_BUILD_DIR}"
	"${FONTCONFIG_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"fontconfig"
	"freetype_install;expat_install"
	${PLUGIN_LEVEL}
)
include("${FONTCONFIG_LIBRARY_CREATE_FILE}")

register_dependency(fontconfig)