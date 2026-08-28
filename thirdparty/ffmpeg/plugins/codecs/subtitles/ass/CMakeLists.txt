set(ASS_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(ASS_COMPONENT "libass")

set(ASS_OPTIONS
	"-Dtest=disabled"
	"-Dcompare=disabled"
	"-Dprofile=disabled"
)
if(UNIX AND NOT APPLE)
	list(APPEND ASS_OPTIONS "-Dfontconfig=enabled")
else()
	list(APPEND ASS_OPTIONS
		"-Dfontconfig=disabled"
		"-Drequire-system-font-provider=false"
	)
endif()

buildmaster_component(
	ass
	"${ASS_COMPONENT}"
	"${ASS_SRC_DIR}"
	"${ASS_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"ass"
	""
)
buildmaster_link(ass freetype)
buildmaster_link(ass harfbuzz)
buildmaster_link(ass fribidi)
if(UNIX AND NOT APPLE)
	buildmaster_link(ass fontconfig)
endif()
buildmaster_group_add(ff-subtitles ass)
register_plugin(ass "libass=enabled")
