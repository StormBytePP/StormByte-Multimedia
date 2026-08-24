set(KVAZAAR_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
ensure_build_dir(KVAZAAR_BUILD_DIR)
set(KVAZAAR_COMPONENT "kvazaar")

set(KVAZAAR_PATCH_FILE "${CMAKE_CURRENT_LIST_DIR}/0001-kvazaar-external-pthread-msvc.patch")
create_git_reset_file(KVAZAAR_RESET_REPO kvazaar "kvazaar reset" "${KVAZAAR_SRC_DIR}")
create_git_patch_file(KVAZAAR_PATCH_REPO kvazaar "kvazaar external winpthreads" "${KVAZAAR_SRC_DIR}" "${KVAZAAR_PATCH_FILE}")
include("${KVAZAAR_RESET_REPO}")
include("${KVAZAAR_PATCH_REPO}")

set(KVAZAAR_OPTIONS
	"-DBUILD_SHARED_LIBS=OFF"
	"-DBUILD_TESTS=OFF"
	"-DBUILD_KVAZAAR_BINARY=OFF"
)

if(FFMPEG_BUNDLED_TYPE STREQUAL "shared")
	list(APPEND KVAZAAR_OPTIONS "-DBUILD_SHARED_LIBS=ON")
endif()

if(MSVC)
	create_cmake_dependant_component(
		KVAZAAR_LIBRARY_CREATE_FILE
		"kvazaar"
		"${KVAZAAR_COMPONENT}"
		"${KVAZAAR_SRC_DIR}"
		"${KVAZAAR_BUILD_DIR}"
		"${KVAZAAR_OPTIONS}"
		"${FFMPEG_BUNDLED_TYPE}"
		"kvazaar"
		"pthread-win32_install"
		${PLUGIN_LEVEL}
	)
else()
	create_cmake_component(
		KVAZAAR_LIBRARY_CREATE_FILE
		"kvazaar"
		"${KVAZAAR_COMPONENT}"
		"${KVAZAAR_SRC_DIR}"
		"${KVAZAAR_BUILD_DIR}"
		"${KVAZAAR_OPTIONS}"
		"${FFMPEG_BUNDLED_TYPE}"
		"kvazaar"
		${PLUGIN_LEVEL}
	)
endif()
include("${KVAZAAR_LIBRARY_CREATE_FILE}")

if(MSVC AND FFMPEG_BUNDLED_TYPE STREQUAL "static")
	add_custom_command(TARGET kvazaar_install POST_BUILD
		COMMAND ${CMAKE_COMMAND}
			-DDIR=${BUILDMASTER_INSTALL_LIBDIR}
			-DPATTERN=libkvazaar.lib
			-DOUT=kvazaar.lib
			-DMODE=copy
			-P "${FFMPEG_RENAME_MSVC_LIB}"
		COMMENT "MSVC: libkvazaar.lib → kvazaar.lib (copy)"
	)
endif()

register_plugin(kvazaar "-Dlibkvazaar=enabled")