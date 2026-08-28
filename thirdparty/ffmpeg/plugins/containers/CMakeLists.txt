buildmaster_group(ff-containers "Containers")
buildmaster_group_add(ff-plugins ff-containers)

add_subdirectory(matroska)
add_subdirectory(ogg)

set(FFMPEG_PLUGIN_OPTIONS "${FFMPEG_PLUGIN_OPTIONS}" PARENT_SCOPE)