message_indented("Setting up yadif" ${PLUGIN_LEVEL})

# Dummy targets required by register_plugin
add_library(yadif INTERFACE)
add_custom_target(yadif_install)

register_plugin(yadif "-Dyadif_filter=enabled")
