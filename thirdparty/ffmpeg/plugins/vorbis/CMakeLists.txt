set(VORBIS_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
ensure_build_dir(VORBIS_BUILD_DIR)
library_import_hint(VORBIS_LIBRARY "vorbis" "${FFMPEG_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}")
library_import_hint(VORBIS_ENC_LIBRARY "vorbisenc" "${FFMPEG_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}")

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

add_custom_target(vorbis_configure
	COMMAND ${ENV_CMAKE_COMMAND} -P "${VORBIS_CMAKE_CONFIGURE_FILE}"
	WORKING_DIRECTORY "${VORBIS_SRC_DIR}"
	COMMENT "Configuring Vorbis codec"
	USES_TERMINAL
	DEPENDS ogg_install
)

add_custom_target(vorbis_build
	COMMAND ${ENV_CMAKE_COMMAND} --build "${VORBIS_BUILD_DIR}"
	WORKING_DIRECTORY "${VORBIS_BUILD_DIR}"
	COMMENT "Building Vorbis codec"
	USES_TERMINAL
	DEPENDS vorbis_configure
)

add_custom_target(vorbis_install
	COMMAND ${ENV_CMAKE_SILENT_COMMAND} --build "${VORBIS_BUILD_DIR}" --target install
	WORKING_DIRECTORY "${VORBIS_BUILD_DIR}"
	COMMENT "Installing Vorbis codec internally"
	DEPENDS vorbis_build
)

# Since this library generates two outputs, we need to create this imported target
# which is not created automatically by register_plugin
add_library(vorbisenc SHARED IMPORTED GLOBAL)
if(MSVC)
	set_property(TARGET vorbisenc PROPERTY IMPORTED_IMPLIB "${VORBIS_ENC_LIBRARY}")
else()
	set_property(TARGET vorbisenc PROPERTY IMPORTED_LOCATION "${VORBIS_ENC_LIBRARY}")
endif()
cmake_policy(SET CMP0079 NEW)
target_link_libraries(ffmpeg-plugins INTERFACE vorbisenc)

add_custom_command(
	OUTPUT "${VORBIS_LIBRARY}" "${VORBIS_ENC_LIBRARY}"
	DEPENDS vorbis_install
)
set(vorbis_outputs "${VORBIS_LIBRARY}" PARENT_SCOPE)