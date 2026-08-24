message_indented("Setting up dvdsub" ${PLUGIN_LEVEL})

# Dummy targets required by register_plugin (built-in FFmpeg, no external lib)
add_library(dvdsub INTERFACE)
add_custom_target(dvdsub_install)

register_plugin(dvdsub "-Ddvdsub_decoder=enabled -Ddvdsub_encoder=enabled -Ddvdsub_parser=enabled -Dvobsub_demuxer=enabled")
