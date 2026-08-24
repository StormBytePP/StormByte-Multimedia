message_indented("Setting up ass_text" ${PLUGIN_LEVEL})

# Dummy targets required by register_plugin (built-in FFmpeg, no external lib)
add_library(ass_text INTERFACE)
add_custom_target(ass_text_install)

register_plugin(ass_text "-Dass_demuxer=enabled -Dass_muxer=enabled -Dass_decoder=enabled -Dass_encoder=enabled -Dssa_decoder=enabled -Dssa_encoder=enabled")
