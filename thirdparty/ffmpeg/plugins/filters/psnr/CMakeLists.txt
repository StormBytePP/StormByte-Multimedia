buildmaster_group(psnr "psnr")
buildmaster_group_add(ff-filters psnr)
register_builtin_plugin("psnr_filter=enabled")
