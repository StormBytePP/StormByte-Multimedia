set(XVID_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake-port")
ensure_build_dir(XVID_BUILD_DIR)
if(FFMPEG_BUNDLED STREQUAL "static")
	library_import_static_hint(XVID_LIBRARY "xvidcore" "${FFMPEG_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}")
else()
	library_import_hint(XVID_LIBRARY "xvidcore" "${FFMPEG_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}")
endif()

set(XVID_COMPONENT "XVID codec")
set(XVID_OPTIONS
	"-DBUILD_SHARED_LIBS=${CMAKE_SHARED_MODE}"
	"-DCMAKE_POSITION_INDEPENDENT_CODE=ON"
	"-DCMAKE_ASM_NASM_FLAGS=-DPIC -w-pp-macro-params-legacy"
)
create_cmake_configure_file(
	XVID_CMAKE_CONFIGURE_FILE
	"${XVID_COMPONENT}"
	"${XVID_SRC_DIR}"
	"${XVID_BUILD_DIR}"
	"${FFMPEG_INSTALL_DIR}"
	"${XVID_OPTIONS}"
	2
)
include("${XVID_CMAKE_CONFIGURE_FILE}")

add_custom_target(xvid_build
	COMMAND ${ENV_CMAKE_COMMAND} --build "${XVID_BUILD_DIR}"
	WORKING_DIRECTORY "${XVID_BUILD_DIR}"
	COMMENT "Building XVID codec"
)

add_custom_target(xvid_install
	COMMAND ${ENV_CMAKE_SILENT_COMMAND} --build "${XVID_BUILD_DIR}" --target install
	WORKING_DIRECTORY "${XVID_BUILD_DIR}"
	COMMENT "Installing XVID codec internally"
	DEPENDS xvid_build
)
set(xvid_outputs "${XVID_LIBRARY}" PARENT_SCOPE)