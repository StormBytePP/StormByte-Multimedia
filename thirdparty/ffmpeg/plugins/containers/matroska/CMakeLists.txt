message_indented("Setting up matroska" ${PLUGIN_LEVEL})

add_library(matroska INTERFACE)
add_custom_target(matroska_install)

register_plugin(matroska
	"-Dmatroska_muxer=enabled -Dmatroska_audio_muxer=enabled -Dmatroska_demuxer=enabled"
)