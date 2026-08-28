plugin_require(WITH_GPL "hqdn3d_filter=disabled")
buildmaster_group(hqdn3d "hqdn3d")
buildmaster_group_add(ff-filters hqdn3d)
register_builtin_plugin("hqdn3d_filter=enabled")
