set(FONTCONFIG_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(FONTCONFIG_COMPONENT "fontconfig")

set(FONTCONFIG_OPTIONS
	"-Ddoc=disabled"
	"-Dnls=disabled"
	"-Dtests=disabled"
	"-Dtools=disabled"
	"-Dcache-build=disabled"
)

buildmaster_component(
	fontconfig
	"${FONTCONFIG_COMPONENT}"
	"${FONTCONFIG_SRC_DIR}"
	"${FONTCONFIG_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"fontconfig"
	""
)
buildmaster_link(fontconfig freetype)
buildmaster_link(fontconfig expat)
buildmaster_group_add(ff-deps fontconfig)
register_dependency(fontconfig)
