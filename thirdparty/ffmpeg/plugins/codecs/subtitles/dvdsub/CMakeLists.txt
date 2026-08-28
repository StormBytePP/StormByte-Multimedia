buildmaster_group(dvdsub "dvdsub")
buildmaster_group_add(ff-subtitles dvdsub)
register_builtin_plugin("dvdsub_decoder=enabled dvdsub_encoder=enabled dvdsub_parser=enabled vobsub_demuxer=enabled")
