buildmaster_group(loudnorm "loudnorm")
buildmaster_group_add(ff-filters loudnorm)
register_builtin_plugin("loudnorm_filter=enabled")
