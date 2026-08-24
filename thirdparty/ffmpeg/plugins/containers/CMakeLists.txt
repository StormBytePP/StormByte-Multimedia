message_indented("Containers:" ${PLUGIN_LEVEL})

math(EXPR PLUGIN_LEVEL "${PLUGIN_LEVEL} + 1")
add_subdirectory(matroska)
add_subdirectory(ogg)

set(FFMPEG_PLUGIN_OPTIONS "${FFMPEG_PLUGIN_OPTIONS}" PARENT_SCOPE)