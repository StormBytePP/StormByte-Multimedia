set(VVENC_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
ensure_build_dir(VVENC_BUILD_DIR)
if(FFMPEG_BUNDLED STREQUAL "static")
	library_import_static_hint(VVENC_LIBRARY "vvenc" "${FFMPEG_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}")
else()
	library_import_hint(VVENC_LIBRARY "vvenc" "${FFMPEG_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}")
endif()

set(VVENC_COMPONENT "VVenc (H266) codec")
set(VVENC_OPTIONS
	"-DBUILD_SHARED_LIBS=${CMAKE_SHARED_MODE}"
	"-DVVENC_LIBRARY_ONLY=ON"
	"-DCMAKE_POSITION_INDEPENDENT_CODE=ON"
	"-DCMAKE_ASM_NASM_FLAGS=-DPIC -w-pp-macro-params-legacy"
)
create_cmake_configure_file(
	VVENC_CMAKE_CONFIGURE_FILE
	"${VVENC_COMPONENT}"
	"${VVENC_SRC_DIR}"
	"${VVENC_BUILD_DIR}"
	"${FFMPEG_INSTALL_DIR}"
	"${VVENC_OPTIONS}"
	2
)
include("${VVENC_CMAKE_CONFIGURE_FILE}")

add_custom_target(vvenc_build
	COMMAND ${ENV_CMAKE_COMMAND} --build "${VVENC_BUILD_DIR}"
	WORKING_DIRECTORY "${VVENC_BUILD_DIR}"
	COMMENT "Building VVenc codec"
)

add_custom_target(vvenc_install
	COMMAND ${ENV_CMAKE_SILENT_COMMAND} --build "${VVENC_BUILD_DIR}" --target install
	WORKING_DIRECTORY "${VVENC_BUILD_DIR}"
	COMMENT "Installing VVenc codec internally"
	DEPENDS vvenc_build
)
set(vvenc_outputs "${VVENC_LIBRARY}" PARENT_SCOPE)