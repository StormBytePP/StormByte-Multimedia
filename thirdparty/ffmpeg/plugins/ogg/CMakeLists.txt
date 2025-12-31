set(OGG_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/src")
ensure_build_dir(OGG_BUILD_DIR)
library_import_hint(OGG_LIBRARY "ogg" "${FFMPEG_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(OGG_COMPONENT "OGG container")
set(OGG_OPTIONS
	"-DBUILD_SHARED_LIBS=ON"
	"-DBUILD_TESTING=OFF"
	"-DINSTALL_DOCS=OFF"
	"-DCMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_LIBDIR}"
)

create_cmake_configure_file(
	OGG_CMAKE_CONFIGURE_FILE
	"${OGG_COMPONENT}"
	"${OGG_SRC_DIR}"
	"${OGG_BUILD_DIR}"
	"${FFMPEG_INSTALL_DIR}"
	"${OGG_OPTIONS}"
	2
)
include("${OGG_CMAKE_CONFIGURE_FILE}")

add_custom_target(_ogg_build
	COMMAND ${ENV_CMAKE_COMMAND} --build "${OGG_BUILD_DIR}"
	WORKING_DIRECTORY "${OGG_BUILD_DIR}"
	COMMENT "Building OGG container"
	USES_TERMINAL
)

add_custom_target(_ogg_install
	COMMAND ${ENV_RUNNER_SILENT} ${ENV_CMAKE_COMMAND} --build "${OGG_BUILD_DIR}" --target install
	WORKING_DIRECTORY "${OGG_BUILD_DIR}"
	COMMENT "Installing internally OGG container"
	DEPENDS _ogg_build
)

add_custom_target(ogg DEPENDS _ogg_install)
