set(X264_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/src")
ensure_build_dir(X264_BUILD_DIR)
if(FFMPEG_BUNDLED STREQUAL "static")
	library_import_static_hint(X264_LIBRARY "x264" "${FFMPEG_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}")
else()
	library_import_hint(X264_LIBRARY "x264" "${FFMPEG_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}")
endif()

set(X264_COMPONENT "X264 codec")
set(X264_OPTIONS
	"-Ddefault_library=${MESON_SHARED_MODE}"
	"-Dasm=enabled"
	"-Dbashcompletion=false"
	"-Dcli=false"
	"-Dopencl=false"
	"-Dbit-depth=all"
)
create_meson_setup_file(
	X264_MESON_SETUP_FILE
	"${X264_COMPONENT}"
	"${X264_SRC_DIR}"
	"${X264_BUILD_DIR}"
	"${FFMPEG_INSTALL_DIR}"
	"${X264_OPTIONS}"
	2
)

if (NOT EXISTS "${X264_BUILD_DIR}/build.ninja")
	include("${X264_MESON_SETUP_FILE}")
else()
	message(STATUS "Skipping meson setup; build files already present for x264")
endif()

add_custom_target(x264_build
	COMMAND_EXPAND_LISTS
	COMMAND ${ENV_NINJA_COMMAND} -C "${X264_BUILD_DIR}"
	WORKING_DIRECTORY ${X264_BUILD_DIR}
	COMMENT "Building X264 codec"
	USES_TERMINAL
)

add_custom_target(x264_install
	COMMAND_EXPAND_LISTS
	COMMAND ${ENV_NINJA_SILENT_COMMAND} -C "${X264_BUILD_DIR}" install
	WORKING_DIRECTORY ${X264_BUILD_DIR}
	COMMENT "Installing X264 codec internally"
	DEPENDS x264_build
)
if(MSVC)
	# On Windows, library is called libx264.a, we need to rename to x264.lib
	add_custom_command(TARGET x264_install POST_BUILD
		COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename "${FFMPEG_INSTALL_LIB_DIR}/libx264.a" "${FFMPEG_INSTALL_LIB_DIR}/x264.lib"
		COMMENT "Renaming libx264.a to x264.lib"
	)
endif()
set(x264_outputs "${X264_LIBRARY}" PARENT_SCOPE)