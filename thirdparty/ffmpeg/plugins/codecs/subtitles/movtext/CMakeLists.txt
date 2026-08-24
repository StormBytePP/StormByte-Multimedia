message_indented("Setting up movtext" ${PLUGIN_LEVEL})

# Dummy targets required by register_plugin (built-in FFmpeg, no external lib)
add_library(movtext INTERFACE)
add_custom_target(movtext_install)

register_plugin(movtext "-Dmovtext_decoder=enabled -Dmovtext_encoder=enabled")
