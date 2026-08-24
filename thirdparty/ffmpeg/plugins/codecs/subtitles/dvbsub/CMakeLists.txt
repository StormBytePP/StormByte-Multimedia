message_indented("Setting up dvbsub" ${PLUGIN_LEVEL})

# Dummy targets required by register_plugin (built-in FFmpeg, no external lib)
add_library(dvbsub INTERFACE)
add_custom_target(dvbsub_install)

register_plugin(dvbsub "-Ddvbsub_demuxer=enabled -Ddvbsub_decoder=enabled -Ddvbsub_encoder=enabled -Ddvbsub_parser=enabled")
