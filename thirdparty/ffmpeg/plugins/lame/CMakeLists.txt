set(LAME_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/cmake")
ensure_build_dir(LAME_BUILD_DIR)
library_import_hint(LAME_LIBRARY "mp3lame" "${FFMPEG_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(LAME_COMPONENT "MP3 encoder LAME")
set(LAME_OPTIONS
	"-DBUILD_SHARED_LIBS=ON"
)

create_cmake_configure_file(
	LAME_CMAKE_CONFIGURE_FILE
	"${LAME_COMPONENT}"
	"${LAME_SRC_DIR}"
	"${LAME_BUILD_DIR}"
	"${FFMPEG_INSTALL_DIR}"
	"${LAME_OPTIONS}"
	2
)
include("${LAME_CMAKE_CONFIGURE_FILE}")

add_custom_target(_lame_build
	COMMAND ${ENV_CMAKE_COMMAND} --build "${LAME_BUILD_DIR}"
	WORKING_DIRECTORY "${LAME_BUILD_DIR}"
	COMMENT "Building MP3 encoder LAME"
	USES_TERMINAL
)

add_custom_target(_lame_install
	COMMAND ${ENV_RUNNER_SILENT} ${ENV_CMAKE_COMMAND} --build "${LAME_BUILD_DIR}" --target install
	WORKING_DIRECTORY "${LAME_BUILD_DIR}"
	COMMENT "Installing internally MP3 encoder LAME"
	DEPENDS _lame_build
)

add_custom_target(lame DEPENDS _lame_install)
