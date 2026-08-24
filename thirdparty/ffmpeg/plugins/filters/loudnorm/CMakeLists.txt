message_indented("Setting up loudnorm" ${PLUGIN_LEVEL})

# Dummy targets required by register_plugin
add_library(loudnorm INTERFACE)
add_custom_target(loudnorm_install)

register_plugin(loudnorm "-Dloudnorm_filter=enabled")
