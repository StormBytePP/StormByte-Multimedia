set(DAV1D_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(DAV1D_COMPONENT "dav1d")

set(DAV1D_OPTIONS
	"-Denable_tools=false"
	"-Denable_examples=false"
	"-Denable_tests=false"
	"-Denable_docs=false"
	"-Denable_asm=true"
)

buildmaster_component(
	dav1d
	"${DAV1D_COMPONENT}"
	"${DAV1D_SRC_DIR}"
	"${DAV1D_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"dav1d"
	""
)
buildmaster_group_add(ff-video dav1d)
register_plugin(dav1d "libdav1d=enabled")
