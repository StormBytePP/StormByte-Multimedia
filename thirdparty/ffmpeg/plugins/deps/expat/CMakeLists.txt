set(EXPAT_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/expat")
set(EXPAT_COMPONENT "expat")

set(EXPAT_OPTIONS
	"-DEXPAT_SHARED_LIBS=OFF"
	"-DEXPAT_BUILD_TOOLS=OFF"
	"-DEXPAT_BUILD_EXAMPLES=OFF"
	"-DEXPAT_BUILD_TESTS=OFF"
	"-DEXPAT_BUILD_DOCS=OFF"
)
if(FFMPEG_BUNDLED_TYPE STREQUAL "shared")
	list(APPEND EXPAT_OPTIONS "-DEXPAT_SHARED_LIBS=ON")
endif()

buildmaster_component(
	expat
	"${EXPAT_COMPONENT}"
	"${EXPAT_SRC_DIR}"
	"${EXPAT_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"expat"
	""
)
buildmaster_group_add(ff-deps expat)
register_dependency(expat)
