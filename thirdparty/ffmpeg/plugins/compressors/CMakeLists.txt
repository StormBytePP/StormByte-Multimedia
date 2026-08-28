buildmaster_group(ff-compressors "Compressors")
buildmaster_group_add(ff-plugins ff-compressors)

add_subdirectory(bzip2)
add_subdirectory(lzma)
add_subdirectory(zlib)

set(FFMPEG_PLUGIN_OPTIONS "${FFMPEG_PLUGIN_OPTIONS}" PARENT_SCOPE)
