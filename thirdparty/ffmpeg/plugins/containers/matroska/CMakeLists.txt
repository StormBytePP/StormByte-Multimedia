buildmaster_group(matroska "Matroska")
buildmaster_group_add(ff-containers matroska)
register_builtin_plugin("matroska_muxer=enabled matroska_audio_muxer=enabled matroska_demuxer=enabled")
