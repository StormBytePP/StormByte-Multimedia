message_indented("Setting up pgssub" ${PLUGIN_LEVEL})

# Dummy targets required by register_plugin (built-in FFmpeg, no external lib)
add_library(pgssub INTERFACE)
add_custom_target(pgssub_install)

register_plugin(pgssub "-Dpgssub_decoder=enabled -Dsup_demuxer=enabled -Dsup_muxer=enabled")
