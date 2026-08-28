set(AOM_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(AOM_COMPONENT "libaom")

set(AOM_OPTIONS
    "ENABLE_APPS=OFF"
    "ENABLE_DOCS=OFF"
    "ENABLE_EXAMPLES=OFF"
    "ENABLE_TESTS=OFF"
    "ENABLE_TOOLS=OFF"
    "CONFIG_AV1_ENCODER=1"
    "CONFIG_AV1_DECODER=1"
    "BUILD_SHARED_LIBS=OFF"
)
if(FFMPEG_BUNDLED_TYPE STREQUAL "shared")
	list(APPEND AOM_OPTIONS "-DBUILD_SHARED_LIBS=ON")
endif()

buildmaster_component(
	aom
	"${AOM_COMPONENT}"
	"${AOM_SRC_DIR}"
	"${AOM_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"aom"
	""
)
buildmaster_group_add(ff-video aom)
register_plugin(aom "libaom=enabled")
