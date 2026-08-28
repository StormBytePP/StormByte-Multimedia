buildmaster_group(dvbsub "dvbsub")
buildmaster_group_add(ff-subtitles dvbsub)
register_builtin_plugin("dvbsub_demuxer=enabled dvbsub_decoder=enabled dvbsub_encoder=enabled dvbsub_parser=enabled")
