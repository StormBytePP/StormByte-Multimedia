message_indented("Setting up xpsnr" ${PLUGIN_LEVEL})

# Dummy targets required by register_plugin
add_library(xpsnr INTERFACE)
add_custom_target(xpsnr_install)

register_plugin(xpsnr "-Dxpsnr_filter=enabled")
