message(STATUS "\tSetting up FFmpeg plugins")

# Add helpers so they are available
include(helpers.cmake)

# Initialize environment variables for plugins
if(MSVC)
  	windows_path(_include_path "${FFMPEG_INSTALL_INCLUDE_DIR}")
  	windows_path(_lib_path "${FFMPEG_INSTALL_LIB_DIR}")
	set(INCLUDE "${_include_path}")
	set(LIB "${_lib_path}")
else()
	set(_include_flags "-I${FFMPEG_INSTALL_INCLUDE_DIR}")
	set(CFLAGS "${_include_flags}")
	set(CXXFLAGS "${_include_flags}")
  	set(LDFLAGS "-L${FFMPEG_INSTALL_LIB_DIR}")
endif()
set(PKG_CONFIG_PATH "${FFMPEG_INSTALL_LIB_DIR}/pkgconfig")

# Update runner with new env vars
update_env_runner()

# Create a target that will aggregate all plugins
add_library(ffmpeg-plugins INTERFACE)
add_custom_target(ffmpeg-plugins-install)

# Initialize FFMPEG plugin options
set(FFMPEG_PLUGIN_OPTIONS "")

# Plugins registration: since plugins are external projects (from ffmpeg point of view), they are handled with
# register_plugin functions which make add_subdirectory if needed and add their corresponding options
# to FFMPEG_PLUGIN_OPTIONS variable.
register_plugin_optional("fdk-aac" WITH_NONFREE "-Dlibfdk_aac=enabled" "-Dlibfdk_aac=disabled")
register_plugin("ogg" "-Doga_muxer=enabled -Dogg_muxer=enabled -Dogv_muxer=enabled -Dogg_demuxer=enabled")
register_plugin("mp3lame" "-Dlibmp3lame=enabled")
register_plugin("opus" "-Dlibopus=enabled")
register_plugin_optional_negated("vmaf" MSVC "-Dlibvmaf=enabled -Dlibvmaf_filter=enabled -Dvmafmotion_filter=enabled" "-Dlibvmaf=disabled -Dlibvmaf_filter=disabled -Dvmafmotion_filter=disabled")
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