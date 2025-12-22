set(X264_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/src")
ensure_build_dir(X264_BUILD_DIR)
library_import_hint(X264_LIBRARY "x264" "${FFMPEG_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}")

set(X264_COMPONENT "X264 codec")
set(X264_OPTIONS
	"-Ddefault_library=shared"
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
	"${FFMPEG_PLUGINS_INSTALL_DIR}"
	"${X264_OPTIONS}"
)

if (NOT EXISTS "${X264_BUILD_DIR}/build.ninja")
	include("${X264_MESON_SETUP_FILE}")
else()
	message(STATUS "Skipping meson setup; build files already present for x264")
endif()

add_custom_target(_x264_build
	COMMAND_EXPAND_LISTS
	COMMAND ${NINJA_EXECUTABLE} -C "${X264_BUILD_DIR}"
	WORKING_DIRECTORY ${X264_BUILD_DIR}
	COMMENT "Building X264 codec"
	USES_TERMINAL
)

add_custom_target(_x264_install
	COMMAND_EXPAND_LISTS
	COMMAND ${ENV_RUNNER_SILENT} ${NINJA_EXECUTABLE} -C "${X264_BUILD_DIR}" install
	WORKING_DIRECTORY ${X264_BUILD_DIR}
	COMMENT "Installing X264 codec internally"
)

add_custom_target(x264 DEPENDS _x264_install)
