plugin_require(WITH_GPL "-Dlibx264=disabled -Dlibx264_encoder=disabled -Dlibx264rgb_encoder=disabled")

set(X264_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/src")
ensure_build_dir(X264_BUILD_DIR)
set(X264_COMPONENT "X264 codec")

set(X264_OPTIONS
	"-Dbashcompletion=false"
	"-Dcli=false"
	"-Dopencl=false"
	"-Dbit-depth=all"
)

# x264 has useful ASM on both x86_64 and aarch64; keep enabled by default.
# If a platform lacks working ASM, Meson/x264 still fall back or fail clearly.
list(APPEND X264_OPTIONS "-Dasm=enabled")

create_meson_component(
	X264_LIBRARY_CREATE_FILE
	"x264"
	"${X264_COMPONENT}"
	"${X264_SRC_DIR}"
	"${X264_BUILD_DIR}"
	"${X264_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"x264"
	${PLUGIN_LEVEL}
)
include("${X264_LIBRARY_CREATE_FILE}")

if(MSVC AND FFMPEG_BUNDLED_TYPE STREQUAL "static")
	add_custom_command(TARGET x264_install POST_BUILD
		COMMAND ${CMAKE_COMMAND}
			-DDIR=${BUILDMASTER_INSTALL_LIBDIR}
			-DPATTERN=libx264.a
			-DOUT=x264.lib
			-DMODE=copy
			-P "${FFMPEG_RENAME_MSVC_LIB}"
		COMMAND ${CMAKE_COMMAND}
			-DLIB=${BUILDMASTER_INSTALL_LIBDIR}/x264.lib
			-DMEMBER=x264res.rc_x264res.res
			-P "${FFMPEG_STRIP_MSVC_RES}"
		COMMENT "MSVC: libx264.a → x264.lib (copy) + strip version .res"
	)
endif()

register_plugin(x264 "-Dlibx264=enabled -Dlibx264_encoder=enabled -Dlibx264rgb_encoder=enabled")
