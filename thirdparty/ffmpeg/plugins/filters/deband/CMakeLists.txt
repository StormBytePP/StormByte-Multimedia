buildmaster_group(deband "deband")
buildmaster_group_add(ff-filters deband)
register_builtin_plugin("deband_filter=enabled")
