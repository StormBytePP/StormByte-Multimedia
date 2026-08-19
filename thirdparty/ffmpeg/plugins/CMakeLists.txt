message(STATUS "\tSetting up FFmpeg plugins")

include(helpers.cmake)

add_library(ffmpeg-plugins INTERFACE)

# Always (MSVC-only logic lives inside deps/)
add_subdirectory(deps)

set(FFMPEG_PLUGIN_OPTIONS "")

add_subdirectory(codecs)
add_subdirectory(filters)

# Options collected by plugin_require / register_plugin (via codecs & filters)
set(FFMPEG_PLUGIN_OPTIONS "${FFMPEG_PLUGIN_OPTIONS}" PARENT_SCOPE)