message_indented("Subtitles:" ${PLUGIN_LEVEL})

math(EXPR PLUGIN_LEVEL "${PLUGIN_LEVEL} + 1")
add_subdirectory(ass)
add_subdirectory(ass_text)
add_subdirectory(dvbsub)
add_subdirectory(dvdsub)
add_subdirectory(movtext)
add_subdirectory(pgssub)
add_subdirectory(srt)
add_subdirectory(ttml)
add_subdirectory(webvtt)

set(FFMPEG_PLUGIN_OPTIONS "${FFMPEG_PLUGIN_OPTIONS}" PARENT_SCOPE)