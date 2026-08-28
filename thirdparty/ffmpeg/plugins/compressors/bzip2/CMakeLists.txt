set(BZIP2_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/cmake-port")
set(BZIP2_COMPONENT "bzip2 compression library")
set(BZIP2_OPTIONS "")

buildmaster_component(
	bzip2
	"${BZIP2_COMPONENT}"
	"${BZIP2_SRC_DIR}"
	"${BZIP2_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"bz2"
	"PC={VERSION=1.0.8;NAME=bzip2}"
)
buildmaster_group_add(ff-compressors bzip2)
register_plugin(bzip2 "bzlib=enabled")
