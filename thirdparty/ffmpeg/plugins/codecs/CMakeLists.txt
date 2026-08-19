message(STATUS "\t\tCodecs:")

add_subdirectory(fdk-aac)
add_subdirectory(mp3lame)
add_subdirectory(ogg)
add_subdirectory(opus)
add_subdirectory(vorbis)
add_subdirectory(vpx)
add_subdirectory(vvenc)
add_subdirectory(x264)
add_subdirectory(x265)
add_subdirectory(xvidcore)

set(FFMPEG_PLUGIN_OPTIONS "${FFMPEG_PLUGIN_OPTIONS}" PARENT_SCOPE)