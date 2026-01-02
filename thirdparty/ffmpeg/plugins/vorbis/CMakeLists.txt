set(VORBIS_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
ensure_build_dir(VORBIS_BUILD_DIR)
if(FFMPEG_BUNDLED STREQUAL "static")
	library_import_static_hint(VORBIS_LIBRARY "vorbis" "${FFMPEG_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}")
	library_import_static_hint(VORBIS_ENC_LIBRARY "vorbisenc" "${FFMPEG_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}")
else()
	library_import_hint(VORBIS_LIBRARY "vorbis" "${FFMPEG_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}")
	library_import_hint(VORBIS_ENC_LIBRARY "vorbisenc" "${FFMPEG_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}")
endif()

set(VORBIS_COMPONENT "Vorbis codec")
set(VORBIS_OPTIONS
	"-DBUILD_SHARED_LIBS=${CMAKE_SHARED_MODE}"
	"-DBUILD_TESTING=OFF"
	"-DCMAKE_POSITION_INDEPENDENT_CODE=ON"
	"-DCMAKE_ASM_NASM_FLAGS=-DPIC -w-pp-macro-params-legacy"
)
create_cmake_configure_file(
	VORBIS_CMAKE_CONFIGURE_FILE
	"${VORBIS_COMPONENT}"
	"${VORBIS_SRC_DIR}"
	"${VORBIS_BUILD_DIR}"
	"${FFMPEG_INSTALL_DIR}"
	"${VORBIS_OPTIONS}"
)

add_custom_target(vorbis_configure
	COMMAND ${ENV_CMAKE_SILENT_COMMAND} -P "${VORBIS_CMAKE_CONFIGURE_FILE}"
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
if(FFMPEG_BUNDLED STREQUAL "static")
	add_library(vorbisenc STATIC IMPORTED GLOBAL)
else()
	add_library(vorbisenc SHARED IMPORTED GLOBAL)
endif()
if(MSVC)
	if(FFMPEG_BUNDLED STREQUAL "static")
		set_target_properties(vorbisenc PROPERTIES
			IMPORTED_LOCATION "${VORBIS_ENC_LIBRARY}"
			IMPORTED_LOCATION_DEBUG "${VORBIS_ENC_LIBRARY}"
			IMPORTED_LOCATION_RELEASE "${VORBIS_ENC_LIBRARY}"
			IMPORTED_LOCATION_MINSIZEREL "${VORBIS_ENC_LIBRARY}"
			IMPORTED_LOCATION_RELWITHDEBINFO "${VORBIS_ENC_LIBRARY}"
		)
	else()
		set_target_properties(vorbisenc PROPERTIES
			IMPORTED_IMPLIB "${VORBIS_ENC_LIBRARY}"
			IMPORTED_IMPLIB_DEBUG "${VORBIS_ENC_LIBRARY}"
			IMPORTED_IMPLIB_RELEASE "${VORBIS_ENC_LIBRARY}"
			IMPORTED_IMPLIB_MINSIZEREL "${VORBIS_ENC_LIBRARY}"
			IMPORTED_IMPLIB_RELWITHDEBINFO "${VORBIS_ENC_LIBRARY}"
		)
	endif()
endif()
cmake_policy(SET CMP0079 NEW)
target_link_libraries(ffmpeg-plugins INTERFACE vorbisenc)
# Let the helper manage the outputs rule and target: it expects the
# `<plugin>_outputs` variable to exist and will create
# `<plugin>_outputs_target` plus the add_custom_command that produces the
# files. Export both produced files so the helper can declare the rule.
add_dependencies(ffmpeg-plugins-install vorbisenc)
add_dependencies(vorbisenc vorbis_outputs_target)
add_custom_command(OUTPUT ${VORBIS_ENC_LIBRARY} DEPENDS vorbis_install)
add_custom_target(vorbisenc_outputs_target DEPENDS ${VORBIS_ENC_LIBRARY})
add_dependencies(ffmpeg-plugins-install vorbisenc_outputs_target)
add_dependencies(vorbisenc vorbisenc_outputs_target)

# Export only the primary vorbis library to the helper; manage vorbisenc
# (the secondary import library) manually so the helper receives a single
# output value as expected.
set(vorbis_outputs ${VORBIS_LIBRARY} PARENT_SCOPE)