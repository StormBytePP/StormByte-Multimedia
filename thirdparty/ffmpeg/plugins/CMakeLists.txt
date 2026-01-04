message(STATUS "\tSetting up FFmpeg plugins")

# Add helpers so they are available
include(helpers.cmake)

# Create a target that will aggregate all plugins
add_library(ffmpeg-plugins INTERFACE)

# Add dependencies
add_subdirectory(deps)

# Initialize FFMPEG plugin options
set(FFMPEG_PLUGIN_OPTIONS "")

# Plugins registration: since plugins are external projects (from ffmpeg point of view), they are handled with
# register_plugin functions which make add_subdirectory if needed and add their corresponding options
# to FFMPEG_PLUGIN_OPTIONS variable.
register_plugin_optional("fdk-aac" WITH_NONFREE "-Dlibfdk_aac=enabled" "-Dlibfdk_aac=disabled")
register_plugin("mp3lame" "-Dlibmp3lame=enabled")
register_plugin("ogg" "-Doga_muxer=enabled -Dogg_muxer=enabled -Dogv_muxer=enabled -Dogg_demuxer=enabled")
register_plugin("opus" "-Dlibopus=enabled")
register_plugin("vmaf" "-Dlibvmaf=enabled -Dlibvmaf_filter=enabled -Dvmafmotion_filter=enabled")
register_plugin("vorbis" "-Dlibvorbis=enabled -Dlibvorbisenc=enabled")
register_plugin("vpx" "-Dlibvpx=enabled -Dlibvpx_vp8_encoder=enabled -Dlibvpx_vp8_decoder=enabled -Dlibvpx_vp9_encoder=enabled -Dlibvpx_vp9_decoder=enabled")
register_plugin("vvenc" "-Dlibvvenc=enabled -Dlibvvenc_encoder=enabled")
register_plugin_optional(
	"x264"
	WITH_GPL
	"-Dlibx264=enabled -Dlibx264_encoder=enabled -Dlibx264rgb_encoder=enabled"
	"-Dlibx264=disabled -Dlibx264_encoder=disabled -Dlibx264rgb_encoder=disabled"
)
register_plugin_optional(
	"x265"
	WITH_GPL
	"-Dlibx265=enabled -Dlibx265_encoder=enabled"
	"-Dlibx265=disabled -Dlibx265_encoder=disabled"
)
register_plugin_optional(
	"xvid"
	WITH_GPL
	"-Dlibxvid=enabled -Dlibxvid_encoder=enabled"
	"-Dlibxvid=disabled -Dlibxvid_encoder=disabled"
)

# Propagate options to ffmpeg build
set(FFMPEG_PLUGIN_OPTIONS ${FFMPEG_PLUGIN_OPTIONS} PARENT_SCOPE)
if(WIN32)
	set(LIB "${FFMPEG_INSTALL_LIB_DIR}" PARENT_SCOPE)
	set(INCLUDE "${FFMPEG_INSTALL_INCLUDE_DIR}" PARENT_SCOPE)
else()
	set(CFLAGS "${CFLAGS}" PARENT_SCOPE)
	set(CXXFLAGS "${CXXFLAGS}" PARENT_SCOPE)
	set(LDFLAGS "${LDFLAGS}" PARENT_SCOPE)
endif()