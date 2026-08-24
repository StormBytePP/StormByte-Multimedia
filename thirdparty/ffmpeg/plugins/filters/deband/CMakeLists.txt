message_indented("Setting up deband" ${PLUGIN_LEVEL})

# Dummy targets required by register_plugin
add_library(deband INTERFACE)
add_custom_target(deband_install)

register_plugin(deband "-Ddeband_filter=enabled")
