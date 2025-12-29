set(VVENC_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
ensure_build_dir(VVENC_BUILD_DIR)
library_import_hint(VVENC_LIBRARY "vvenc" "${FFMPEG_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}")

set(VVENC_COMPONENT "VVenc (H266) codec")
set(VVENC_OPTIONS
	"-DBUILD_SHARED_LIBS=ON"
	"-DVVENC_LIBRARY_ONLY=ON"
)
create_cmake_configure_file(
	VVENC_CMAKE_CONFIGURE_FILE
	"${VVENC_COMPONENT}"
	"${VVENC_SRC_DIR}"
	"${VVENC_BUILD_DIR}"
	"${FFMPEG_INSTALL_DIR}"
	"${VVENC_OPTIONS}"
)
include("${VVENC_CMAKE_CONFIGURE_FILE}")

add_custom_target(_vvenc_build
	COMMAND ${ENV_CMAKE_COMMAND} --build "${VVENC_BUILD_DIR}"
	WORKING_DIRECTORY "${VVENC_BUILD_DIR}"
	COMMENT "Building VVenc codec"
	USES_TERMINAL
)

add_custom_target(_vvenc_install
	COMMAND ${ENV_RUNNER_SILENT} ${ENV_CMAKE_COMMAND} --build "${VVENC_BUILD_DIR}" --target install
	WORKING_DIRECTORY "${VVENC_BUILD_DIR}"
	COMMENT "Installing VVenc codec internally"
	DEPENDS _vvenc_build
)
add_custom_target(vvenc DEPENDS _vvenc_install)