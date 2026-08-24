plugin_require(WITH_GPL "-Dhqdn3d_filter=disabled")

message_indented("Setting up hqdn3d" ${PLUGIN_LEVEL})

add_library(hqdn3d INTERFACE)
add_custom_target(hqdn3d_install)

register_plugin(hqdn3d "-Dhqdn3d_filter=enabled")