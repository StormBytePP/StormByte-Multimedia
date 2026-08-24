message_indented("Setting up nlmeans" ${PLUGIN_LEVEL})

# Dummy targets required by register_plugin
add_library(nlmeans INTERFACE)
add_custom_target(nlmeans_install)

register_plugin(nlmeans "-Dnlmeans_filter=enabled")
