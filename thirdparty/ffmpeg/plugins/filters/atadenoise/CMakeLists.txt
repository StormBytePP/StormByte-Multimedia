buildmaster_group(atadenoise "atadenoise")
buildmaster_group_add(ff-filters atadenoise)
register_builtin_plugin("atadenoise_filter=enabled")
