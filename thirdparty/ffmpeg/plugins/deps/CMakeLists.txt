message_indented("Dependencies:" ${PLUGIN_LEVEL})
math(EXPR PLUGIN_LEVEL "${PLUGIN_LEVEL} + 1")

# Font / text stack
add_subdirectory(freetype)
add_subdirectory(fribidi)
add_subdirectory(harfbuzz)

# Audio
add_subdirectory(samplerate)

# Image codecs for leptonica (zlib is under compressors; must be built before png/tiff)
add_subdirectory(png)          # → zlib_install
add_subdirectory(jpeg-turbo)
add_subdirectory(tiff)         # → zlib_install + jpeg-turbo_install

# OCR
add_subdirectory(leptonica)    # → zlib, png, jpeg-turbo, tiff
add_subdirectory(tesseract)
add_subdirectory(tessdata)

if(MSVC)
	add_subdirectory(pthread-win32)
	add_subdirectory(gzip)
endif()

if(UNIX AND NOT APPLE)
	add_subdirectory(expat)
	add_subdirectory(fontconfig)
endif()

set(FFMPEG_PLUGIN_OPTIONS "${FFMPEG_PLUGIN_OPTIONS}" PARENT_SCOPE)