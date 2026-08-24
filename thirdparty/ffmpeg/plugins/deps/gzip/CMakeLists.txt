message_indented("Setting up gzip" ${PLUGIN_LEVEL})

set(GZIP_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/cmake-port")
ensure_build_dir(GZIP_BUILD_DIR)
set(GZIP_COMPONENT "GZip host tool")

set(GZIP_OPTIONS
	"-DCMAKE_INSTALL_PREFIX=${BUILDMASTER_INSTALL_DIR}"
	"-DCMAKE_PREFIX_PATH=${BUILDMASTER_INSTALL_DIR}"
	"-DCMAKE_INSTALL_BINDIR=bin"
)

create_cmake_dependant_component(
	GZIP_CREATE_FILE
	"gzip"
	"${GZIP_COMPONENT}"
	"${GZIP_SRC_DIR}"
	"${GZIP_BUILD_DIR}"
	"${GZIP_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"gzip"
	"zlib_install"
	${PLUGIN_LEVEL}
)
include("${GZIP_CREATE_FILE}")

# Same install edge as register_plugin, without Meson options.
add_dependencies(ffmpeg-plugins gzip_install)
