message_indented("Setting up psnr" ${PLUGIN_LEVEL})

# Dummy targets required by register_plugin
add_library(psnr INTERFACE)
add_custom_target(psnr_install)

register_plugin(psnr "-Dpsnr_filter=enabled")
