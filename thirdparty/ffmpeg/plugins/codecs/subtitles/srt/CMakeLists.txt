buildmaster_group(srt "srt")
buildmaster_group_add(ff-subtitles srt)
register_builtin_plugin("srt_demuxer=enabled srt_muxer=enabled srt_decoder=enabled srt_encoder=enabled")
