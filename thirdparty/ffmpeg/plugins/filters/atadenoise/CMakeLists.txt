message_indented("Setting up atadenoise" ${PLUGIN_LEVEL})

# Dummy targets required by register_plugin
add_library(atadenoise INTERFACE)
add_custom_target(atadenoise_install)

register_plugin(atadenoise "-Datadenoise_filter=enabled")
