message_indented("Filters:" ${PLUGIN_LEVEL})

math(EXPR PLUGIN_LEVEL "${PLUGIN_LEVEL} + 1")

# Filters requiring external libraries to be built
add_subdirectory(vmaf)
add_subdirectory(vidstab)
add_subdirectory(zimg)

# Filters not requiring external libraries
add_subdirectory(ssim)
add_subdirectory(psnr)
add_subdirectory(xpsnr)
add_subdirectory(hqdn3d)
add_subdirectory(nlmeans)
add_subdirectory(bm3d)
add_subdirectory(atadenoise)
add_subdirectory(deband)
add_subdirectory(yadif)
add_subdirectory(bwdif)
add_subdirectory(loudnorm)

set(FFMPEG_PLUGIN_OPTIONS "${FFMPEG_PLUGIN_OPTIONS}" PARENT_SCOPE)