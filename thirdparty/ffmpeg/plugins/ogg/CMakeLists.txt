set(OGG_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/src")
ensure_build_dir(OGG_BUILD_DIR)
if(FFMPEG_BUNDLED STREQUAL "static")
	library_import_static_hint(OGG_LIBRARY "ogg" "${FFMPEG_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}")
else()
	library_import_hint(OGG_LIBRARY "ogg" "${FFMPEG_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}")
endif()
set(OGG_COMPONENT "OGG container")
set(OGG_OPTIONS
	"-DBUILD_SHARED_LIBS=${CMAKE_SHARED_MODE}"
	"-DBUILD_TESTING=OFF"
	"-DINSTALL_DOCS=OFF"
	"-DCMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_LIBDIR}"
	"-DCMAKE_POSITION_INDEPENDENT_CODE=ON"
	"-DCMAKE_ASM_NASM_FLAGS=-DPIC -w-pp-macro-params-legacy"
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

add_custom_target(ogg_build
	COMMAND ${ENV_CMAKE_COMMAND} --build "${OGG_BUILD_DIR}"
	WORKING_DIRECTORY "${OGG_BUILD_DIR}"
	COMMENT "Building OGG container"
	USES_TERMINAL
)

add_custom_target(ogg_install
	COMMAND ${ENV_CMAKE_SILENT_COMMAND} --build "${OGG_BUILD_DIR}" --target install
	WORKING_DIRECTORY "${OGG_BUILD_DIR}"
	COMMENT "Installing internally OGG container"
	DEPENDS ogg_build
)

set(ogg_outputs "${OGG_LIBRARY}" PARENT_SCOPE)