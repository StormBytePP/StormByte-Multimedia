# Initialize Buildmaster
set(BUILDMASTER_INITIALIZE_EXTRA_TOOLS "pkgconf")
add_subdirectory(buildmaster)
include(buildmaster/helpers.cmake)

# Rest of deps
add_subdirectory(StormByte)
add_subdirectory(ffmpeg)
