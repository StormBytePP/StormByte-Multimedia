set(TESSDATA_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake-port")
ensure_build_dir(TESSDATA_BUILD_DIR)
set(TESSDATA_COMPONENT "tessdata")

set(TESSDATA_OPTIONS "")

create_cmake_headers_component(
	TESSDATA_CREATE_FILE
	"tessdata"
	"${TESSDATA_COMPONENT}"
	"${TESSDATA_SRC_DIR}"
	"${TESSDATA_BUILD_DIR}"
	"${TESSDATA_OPTIONS}"
	${PLUGIN_LEVEL}
)
include("${TESSDATA_CREATE_FILE}")

register_dependency(tessdata)
