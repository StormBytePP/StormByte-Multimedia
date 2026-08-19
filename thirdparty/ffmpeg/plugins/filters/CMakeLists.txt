message_indented("Filters:" ${PLUGIN_LEVEL})

math(EXPR PLUGIN_LEVEL "${PLUGIN_LEVEL} + 1")
add_subdirectory(vmaf)

set(FFMPEG_PLUGIN_OPTIONS "${FFMPEG_PLUGIN_OPTIONS}" PARENT_SCOPE)