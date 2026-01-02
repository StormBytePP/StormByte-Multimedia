set(PTHREAD_WIN32_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
ensure_build_dir(PTHREAD_WIN32_BUILD_DIR)
if(FFMPEG_BUNDLED STREQUAL "static")
	library_import_static_hint(PTHREAD_WIN32_LIBRARY "pthreadVCE3" "${FFMPEG_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}")
else()
	library_import_hint(PTHREAD_WIN32_LIBRARY "pthreadVCE3" "${FFMPEG_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}")
endif()

set(PTHREAD_WIN32_COMPONENT "pthread Win32 library")
set(PTHREAD_WIN32_OPTIONS
	"-DBUILD_SHARED_LIBS=${CMAKE_SHARED_MODE}"
	"-DCMAKE_POSITION_INDEPENDENT_CODE=ON"
)
create_cmake_configure_file(
	PTHREAD_WIN32_CMAKE_CONFIGURE_FILE
	"${PTHREAD_WIN32_COMPONENT}"
	"${PTHREAD_WIN32_SRC_DIR}"
	"${PTHREAD_WIN32_BUILD_DIR}"
	"${FFMPEG_INSTALL_DIR}"
	"${PTHREAD_WIN32_OPTIONS}"
	2
)
include(${PTHREAD_WIN32_CMAKE_CONFIGURE_FILE})

add_custom_target(pthread_win32_build
	COMMAND ${ENV_CMAKE_COMMAND} --build "${PTHREAD_WIN32_BUILD_DIR}"
	WORKING_DIRECTORY "${PTHREAD_WIN32_BUILD_DIR}"
	COMMENT "Building pthread Win32 library"
)

add_custom_target(pthread_win32_install
	COMMAND ${ENV_CMAKE_SILENT_COMMAND} --build "${PTHREAD_WIN32_BUILD_DIR}" --target install
	WORKING_DIRECTORY "${PTHREAD_WIN32_BUILD_DIR}"
	COMMENT "Installing pthread Win32 library"
	DEPENDS pthread_win32_build
)

if(FFMPEG_BUNDLED STREQUAL "static")
	add_library(pthread_win32 STATIC IMPORTED GLOBAL)
else()
	add_library(pthread_win32 SHARED IMPORTED GLOBAL)
endif()
if(MSVC AND NOT FFMPEG_BUNDLED STREQUAL "static")
	set_target_properties(pthread_win32 PROPERTIES
		IMPORTED_IMPLIB "${PTHREAD_WIN32_LIBRARY}"
		IMPORTED_IMPLIB_DEBUG "${PTHREAD_WIN32_LIBRARY}"
		IMPORTED_IMPLIB_RELEASE "${PTHREAD_WIN32_LIBRARY}"
		IMPORTED_IMPLIB_MINSIZEREL "${PTHREAD_WIN32_LIBRARY}"
		IMPORTED_IMPLIB_RELWITHDEBINFO "${PTHREAD_WIN32_LIBRARY}"
	)
else()
	set_target_properties(pthread_win32 PROPERTIES
		IMPORTED_LOCATION "${PTHREAD_WIN32_LIBRARY}"
		IMPORTED_LOCATION_DEBUG "${PTHREAD_WIN32_LIBRARY}"
		IMPORTED_LOCATION_RELEASE "${PTHREAD_WIN32_LIBRARY}"
		IMPORTED_LOCATION_MINSIZEREL "${PTHREAD_WIN32_LIBRARY}"
		IMPORTED_LOCATION_RELWITHDEBINFO "${PTHREAD_WIN32_LIBRARY}"
	)
endif()
add_dependencies(pthread_win32 pthread_win32_install)

cmake_policy(SET CMP0079 NEW)
target_link_libraries(ffmpeg-plugins INTERFACE pthread_win32)

# Declare the concrete output file(s) produced by the install target so
# build generators (ninja) know which rule generates the library file and
# can schedule the install before linking.
set(pthread_win32_outputs "${PTHREAD_WIN32_LIBRARY}")

add_custom_command(OUTPUT ${pthread_win32_outputs}
	COMMAND ${CMAKE_COMMAND} -E touch "${PTHREAD_WIN32_LIBRARY}"
	DEPENDS pthread_win32_install
)

add_custom_target(pthread_win32_outputs_target DEPENDS ${pthread_win32_outputs})
add_dependencies(pthread_win32 pthread_win32_outputs_target)
add_dependencies(ffmpeg-plugins-install pthread_win32_install)
add_dependencies(ffmpeg-plugins-install pthread_win32_outputs_target)