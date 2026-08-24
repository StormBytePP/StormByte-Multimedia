message_indented("Setting up bm3d" ${PLUGIN_LEVEL})

# Dummy targets required by register_plugin
add_library(bm3d INTERFACE)
add_custom_target(bm3d_install)

register_plugin(bm3d "-Dbm3d_filter=enabled")
