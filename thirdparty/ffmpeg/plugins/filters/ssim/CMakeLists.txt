message_indented("Setting up ssim" ${PLUGIN_LEVEL})

# Dummy targets required by register_plugin
add_library(ssim INTERFACE)
add_custom_target(ssim_install)

register_plugin(ssim "-Dssim_filter=enabled")
