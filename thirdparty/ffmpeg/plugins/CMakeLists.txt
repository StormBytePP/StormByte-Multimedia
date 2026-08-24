include(helpers.cmake)

# For BuildMaster messages
set(PLUGIN_LEVEL 1)

message_indented("Setting up FFmpeg plugins" ${PLUGIN_LEVEL})

add_library(ffmpeg-plugins INTERFACE)

math(EXPR PLUGIN_LEVEL "${PLUGIN_LEVEL} + 1")

set(FFMPEG_PLUGIN_OPTIONS "")

# Compressors first: zlib (and friends) are required by deps (png, tiff, leptonica)
add_subdirectory(compressors)
add_subdirectory(deps)
add_subdirectory(containers) # Containers before codecs (e.g. vorbis → ogg)
add_subdirectory(codecs)
add_subdirectory(filters)

# Options collected by plugin_require / register_plugin
set(FFMPEG_PLUGIN_OPTIONS "${FFMPEG_PLUGIN_OPTIONS}" PARENT_SCOPE)