buildmaster_group(ff-codecs "Codecs")
buildmaster_group_add(ff-plugins ff-codecs)

add_subdirectory(audio)
add_subdirectory(subtitles)
add_subdirectory(video)

set(FFMPEG_PLUGIN_OPTIONS "${FFMPEG_PLUGIN_OPTIONS}" PARENT_SCOPE)
