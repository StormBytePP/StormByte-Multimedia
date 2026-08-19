include(helpers.cmake)

# For BuildMaster messages
set(PLUGIN_LEVEL 1)

message_indented("Setting up FFmpeg plugins" ${PLUGIN_LEVEL})

add_library(ffmpeg-plugins INTERFACE)

math(EXPR PLUGIN_LEVEL "${PLUGIN_LEVEL} + 1")

# Always (MSVC-only logic lives inside deps/)
add_subdirectory(deps)

set(FFMPEG_PLUGIN_OPTIONS "")

add_subdirectory(codecs)
add_subdirectory(filters)

# Options collected by plugin_require / register_plugin (via codecs & filters)
set(FFMPEG_PLUGIN_OPTIONS "${FFMPEG_PLUGIN_OPTIONS}" PARENT_SCOPE)