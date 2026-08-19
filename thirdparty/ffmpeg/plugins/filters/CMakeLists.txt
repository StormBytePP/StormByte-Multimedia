message(STATUS "\t\tFilters:")

add_subdirectory(vmaf)

set(FFMPEG_PLUGIN_OPTIONS "${FFMPEG_PLUGIN_OPTIONS}" PARENT_SCOPE)