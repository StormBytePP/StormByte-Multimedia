set(DAV1D_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
ensure_build_dir(DAV1D_BUILD_DIR)
set(DAV1D_COMPONENT "dav1d")

set(DAV1D_OPTIONS
	"-Denable_tools=false"
	"-Denable_examples=false"
	"-Denable_tests=false"
	"-Denable_docs=false"
	"-Denable_asm=true"
)

create_meson_component(
	DAV1D_LIBRARY_CREATE_FILE
	"dav1d"
	"${DAV1D_COMPONENT}"
	"${DAV1D_SRC_DIR}"
	"${DAV1D_BUILD_DIR}"
	"${DAV1D_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"dav1d"
	${PLUGIN_LEVEL}
)
include("${DAV1D_LIBRARY_CREATE_FILE}")

if(MSVC AND FFMPEG_BUNDLED_TYPE STREQUAL "static")
	add_custom_command(TARGET dav1d_install POST_BUILD
		COMMAND ${CMAKE_COMMAND}
			-DDIR=${BUILDMASTER_INSTALL_LIBDIR}
			-DPATTERN=libdav1d.a
			-DOUT=dav1d.lib
			-DMODE=copy
			-P "${FFMPEG_RENAME_MSVC_LIB}"
		COMMENT "MSVC: libdav1d.a → dav1d.lib (copy)"
	)
endif()

register_plugin(dav1d "-Dlibdav1d=enabled")