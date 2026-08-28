buildmaster_group(pgssub "pgssub")
buildmaster_group_add(ff-subtitles pgssub)
register_builtin_plugin("pgssub_decoder=enabled sup_demuxer=enabled sup_muxer=enabled")
