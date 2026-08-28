set(OGG_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/src")
set(OGG_COMPONENT "OGG container")

set(OGG_OPTIONS
	"-DBUILD_TESTING=OFF"
	"-DINSTALL_DOCS=OFF"
	"-DCMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_LIBDIR}"
)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64|i[3-6]86|x86)$")
	list(APPEND OGG_OPTIONS "-DCMAKE_ASM_NASM_FLAGS=-DPIC -w-pp-macro-params-legacy")
endif()

buildmaster_component(
	ogg
	"${OGG_COMPONENT}"
	"${OGG_SRC_DIR}"
	"${OGG_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"ogg"
	""
)
buildmaster_group_add(ff-containers ogg)
register_plugin(ogg "oga_muxer=enabled ogg_muxer=enabled ogv_muxer=enabled ogg_demuxer=enabled")
