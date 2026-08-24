message_indented("Setting up ttml" ${PLUGIN_LEVEL})

# Dummy targets required by register_plugin (built-in FFmpeg, no external lib)
add_library(ttml INTERFACE)
add_custom_target(ttml_install)

register_plugin(ttml "-Dttml_muxer=enabled -Dttml_encoder=enabled")
