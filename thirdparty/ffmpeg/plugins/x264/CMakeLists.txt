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
	2
)
include("${X264_LIBRARY_CREATE_FILE}")

if(MSVC)
	# On Windows, library is called libx264.a, we need to rename to x264.lib
	add_custom_command(TARGET x264_install POST_BUILD
		COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename "${BUILDMASTER_INSTALL_LIBDIR}/libx264.a" "${BUILDMASTER_INSTALL_LIBDIR}/x264.lib"
		COMMENT "Renaming libx264.a to x264.lib"
	)
endif()