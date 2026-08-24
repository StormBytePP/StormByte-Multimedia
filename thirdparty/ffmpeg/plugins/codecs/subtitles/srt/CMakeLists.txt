message_indented("Setting up srt" ${PLUGIN_LEVEL})

# Dummy targets required by register_plugin (built-in FFmpeg, no external lib)
add_library(srt INTERFACE)
add_custom_target(srt_install)

register_plugin(srt "-Dsrt_demuxer=enabled -Dsrt_muxer=enabled -Dsrt_decoder=enabled -Dsrt_encoder=enabled")
