message_indented("Audio:" ${PLUGIN_LEVEL})

math(EXPR PLUGIN_LEVEL "${PLUGIN_LEVEL} + 1")
add_subdirectory(fdk-aac)
add_subdirectory(gsm)
add_subdirectory(mp3lame)
add_subdirectory(opus)
add_subdirectory(rubberband)
add_subdirectory(speex)
add_subdirectory(vorbis)

set(FFMPEG_PLUGIN_OPTIONS "${FFMPEG_PLUGIN_OPTIONS}" PARENT_SCOPE)