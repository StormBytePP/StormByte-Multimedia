set(VORBIS_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
ensure_build_dir(VORBIS_BUILD_DIR)
library_import_hint(VORBIS_LIBRARY "vorbis" "${FFMPEG_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}")

set(VORBIS_COMPONENT "Vorbis codec")
set(VORBIS_OPTIONS
	"-DBUILD_SHARED_LIBS=ON"
	"-DBUILD_TESTING=OFF"
)
create_cmake_configure_file(
	VORBIS_CMAKE_CONFIGURE_FILE
	"${VORBIS_COMPONENT}"
	"${VORBIS_SRC_DIR}"
	"${VORBIS_BUILD_DIR}"
	"${FFMPEG_INSTALL_DIR}"
	"${VORBIS_OPTIONS}"
	2
)

add_custom_target(_vorbis_configure
	COMMAND ${ENV_CMAKE_COMMAND} -P "${VORBIS_CMAKE_CONFIGURE_FILE}"
	WORKING_DIRECTORY "${VORBIS_SRC_DIR}"
	COMMENT "Configuring Vorbis codec"
	USES_TERMINAL
	DEPENDS ogg
)

add_custom_target(_vorbis_build
	COMMAND ${ENV_CMAKE_COMMAND} --build "${VORBIS_BUILD_DIR}"
	WORKING_DIRECTORY "${VORBIS_BUILD_DIR}"
	COMMENT "Building Vorbis codec"
	USES_TERMINAL
	DEPENDS _vorbis_configure
)

add_custom_target(_vorbis_install
	COMMAND ${ENV_RUNNER_SILENT} ${ENV_CMAKE_COMMAND} --build "${VORBIS_BUILD_DIR}" --target install
	WORKING_DIRECTORY "${VORBIS_BUILD_DIR}"
	COMMENT "Installing Vorbis codec internally"
	DEPENDS _vorbis_build
)
add_custom_target(vorbis DEPENDS _vorbis_install)
