# Initialize Buildmaster
add_subdirectory(buildmaster)

# Rest of deps
add_subdirectory(StormByte)
add_subdirectory(ffmpeg)
add_subdirectory(ocr)
if(MSVC)
	add_subdirectory(pthread-win32)
endif()
add_subdirectory(vmaf)