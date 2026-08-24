message_indented("Setting up webvtt" ${PLUGIN_LEVEL})

# Dummy targets required by register_plugin (built-in FFmpeg, no external lib)
add_library(webvtt INTERFACE)
add_custom_target(webvtt_install)

register_plugin(webvtt "-Dwebvtt_demuxer=enabled -Dwebvtt_muxer=enabled -Dwebvtt_decoder=enabled -Dwebvtt_encoder=enabled")
