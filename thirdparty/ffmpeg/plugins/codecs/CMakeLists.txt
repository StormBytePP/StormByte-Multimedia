message_indented("Codecs:" ${PLUGIN_LEVEL})

math(EXPR PLUGIN_LEVEL "${PLUGIN_LEVEL} + 1")
add_subdirectory(audio)
add_subdirectory(video)
add_subdirectory(subtitles)

set(FFMPEG_PLUGIN_OPTIONS "${FFMPEG_PLUGIN_OPTIONS}" PARENT_SCOPE)