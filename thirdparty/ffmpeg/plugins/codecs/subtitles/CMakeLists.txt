buildmaster_group(ff-subtitles "Subtitles")
buildmaster_group_add(ff-codecs ff-subtitles)

add_subdirectory(ass)
add_subdirectory(ass_text)
add_subdirectory(dvbsub)
add_subdirectory(dvdsub)
add_subdirectory(movtext)
add_subdirectory(pgssub)
add_subdirectory(srt)
add_subdirectory(ttml)
add_subdirectory(webvtt)

set(FFMPEG_PLUGIN_OPTIONS "${FFMPEG_PLUGIN_OPTIONS}" PARENT_SCOPE)
