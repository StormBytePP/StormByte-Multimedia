set(BZIP2_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/cmake-port")
ensure_build_dir(BZIP2_BUILD_DIR)
set(BZIP2_COMPONENT "bzip2 compression library")

# cmake-port/CMakeLists.txt builds static libbz2 (OUTPUT_NAME bz2) from cmake-port/src
set(BZIP2_OPTIONS "")

create_cmake_component(
	BZIP2_LIBRARY_CREATE_FILE
	"bzip2"
	"${BZIP2_COMPONENT}"
	"${BZIP2_SRC_DIR}"
	"${BZIP2_BUILD_DIR}"
	"${BZIP2_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"bz2"
	${PLUGIN_LEVEL}
)
include("${BZIP2_LIBRARY_CREATE_FILE}")

# register_plugin name must match component id → bzip2_install
register_plugin(bzip2 "-Dbzlib=enabled")