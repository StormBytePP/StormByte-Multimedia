message_indented("Setting up bwdif" ${PLUGIN_LEVEL})

# Dummy targets required by register_plugin
add_library(bwdif INTERFACE)
add_custom_target(bwdif_install)

register_plugin(bwdif "-Dbwdif_filter=enabled")
