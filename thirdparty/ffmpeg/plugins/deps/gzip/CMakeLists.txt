set(GZIP_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/cmake-port")
set(GZIP_COMPONENT "GZip host tool")

set(GZIP_OPTIONS "")

buildmaster_component(
	gzip
	"${GZIP_COMPONENT}"
	"${GZIP_SRC_DIR}"
	"${GZIP_OPTIONS}"
	"executable"
	"gzip"
	""
)
buildmaster_link(gzip zlib)
buildmaster_group_add(ff-deps gzip)
register_dependency(gzip)
