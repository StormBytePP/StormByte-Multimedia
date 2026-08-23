message_indented("Compressors:" ${PLUGIN_LEVEL})

math(EXPR PLUGIN_LEVEL "${PLUGIN_LEVEL} + 1")
add_subdirectory(bzip2)
add_subdirectory(lzma)
add_subdirectory(zlib)
if(MSVC)
	add_subdirectory(gzip)
endif()

set(FFMPEG_PLUGIN_OPTIONS "${FFMPEG_PLUGIN_OPTIONS}" PARENT_SCOPE)