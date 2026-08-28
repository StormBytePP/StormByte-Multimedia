plugin_require(WITH_GPL "librubberband=disabled")

set(RUBBERBAND_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(RUBBERBAND_OPTIONS
	resampler=libsamplerate
	fft=builtin
	jni=disabled
	ladspa=disabled
	lv2=disabled
	vamp=disabled
	cmdline=disabled
	tests=disabled
)

buildmaster_component(
	rubberband
	"rubberband"
	"${RUBBERBAND_SRC_DIR}"
	"${RUBBERBAND_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"rubberband"
	"GIT={RESET;PATCH=${CMAKE_CURRENT_LIST_DIR}/0001-mathmisc-stddef.patch;TITLE=rubberband size_t}"
)
buildmaster_link(rubberband samplerate)
buildmaster_group_add(ff-audio rubberband)
register_plugin(rubberband "librubberband=enabled")
