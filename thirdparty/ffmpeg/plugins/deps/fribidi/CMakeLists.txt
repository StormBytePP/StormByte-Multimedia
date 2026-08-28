set(FRIBIDI_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(FRIBIDI_COMPONENT "fribidi")

buildmaster_component(
	fribidi
	"${FRIBIDI_COMPONENT}"
	"${FRIBIDI_SRC_DIR}"
	"-Ddocs=false;-Dtests=false;-Dbin=false"
	"${FFMPEG_BUNDLED_TYPE}"
	"fribidi"
	"RENAME"
)
buildmaster_group_add(ff-deps fribidi)
register_plugin(fribidi "libfribidi=enabled")
