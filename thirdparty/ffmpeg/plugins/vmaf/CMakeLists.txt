set(VMAF_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
ensure_build_dir(VMAF_BUILD_DIR)
if(FFMPEG_BUNDLED STREQUAL "static")
	library_import_static_hint(VMAF_LIBRARY "vmaf" "${FFMPEG_INSTALL_LIB_DIR}")
else()
	library_import_hint(VMAF_LIBRARY "vmaf" "${FFMPEG_INSTALL_LIB_DIR}")
endif()

# Need to patch to disable tool building
set(VMAF_WINDOWS_PATH_FILE "${CMAKE_CURRENT_LIST_DIR}/0001-MSVC-corrections.patch")
set(VMAF_TOOLS_PATH_FILE "${CMAKE_CURRENT_LIST_DIR}/0002-Disable-tool-building.patch")
create_git_reset_file(VMAF_RESET_REPO "VMAF Reset repo" "${VMAF_SRC_DIR}")
create_git_patch_file(VMAF_PATCH_REPO1 "VMAF Apply patch 1" "${VMAF_SRC_DIR}" "${VMAF_WINDOWS_PATH_FILE}")
create_git_patch_file(VMAF_PATCH_REPO2 "VMAF Apply patch 2" "${VMAF_SRC_DIR}" "${VMAF_TOOLS_PATH_FILE}")

include(${VMAF_RESET_REPO})
include(${VMAF_PATCH_REPO1})
include(${VMAF_PATCH_REPO2})

set(VMAF_MESON_OPTIONS
    "-Ddefault_library=${MESON_SHARED_MODE}"
    "-Denable_tests=false"
	"-Denable_docs=false"
	"-Denable_tools=false"
	"-Denable_asm=true"
	"-Denable_avx512=true"
	"-Dbuilt_in_models=true"
	"-Denable_cuda=false"
	"-Denable_nvtx=false"
	"-Denable_nvcc=false"
)

create_meson_setup_file(
	VMAF_MESON_SETUP_FILE
	"VMAF library"
	"${VMAF_SRC_DIR}/libvmaf"
	"${VMAF_BUILD_DIR}"
	"${FFMPEG_INSTALL_DIR}"
	"${VMAF_MESON_OPTIONS}"
	2
)

if (NOT EXISTS "${VMAF_BUILD_DIR}/build.ninja")
	include("${VMAF_MESON_SETUP_FILE}")
else()
	message(STATUS "Skipping meson setup; build files already present for VMAF")
endif()

if(MSVC)
	add_custom_target(vmaf_build
		COMMAND_EXPAND_LISTS
		COMMAND ${ENV_NINJA_COMMAND} -C "${VMAF_BUILD_DIR}"
		WORKING_DIRECTORY ${VMAF_BUILD_DIR}
		COMMENT "Building VMAF library"
		DEPENDS pthread_win32_install
	)
else()
	add_custom_target(vmaf_build
		COMMAND_EXPAND_LISTS
		COMMAND ${ENV_NINJA_COMMAND} -C "${VMAF_BUILD_DIR}"
		WORKING_DIRECTORY ${VMAF_BUILD_DIR}
		COMMENT "Building VMAF library"
	)
endif()

add_custom_target(vmaf_install
	COMMAND_EXPAND_LISTS
	COMMAND ${ENV_NINJA_SILENT_COMMAND} -C "${VMAF_BUILD_DIR}" install
	WORKING_DIRECTORY ${VMAF_BUILD_DIR}
	COMMENT "Installing VMAF library internally"
	DEPENDS vmaf_build
)

if(FFMPEG_BUNDLED STREQUAL "static" AND MSVC)
	# On Windows, library is installed as libvmaf.a, we need to rename it to vmaf_original.lib (in next step we repack)
	add_custom_command(TARGET vmaf_install POST_BUILD
		COMMAND ${ENV_CMAKE_COMMAND} -E rename "${FFMPEG_INSTALL_LIB_DIR}/libvmaf.a" "${FFMPEG_INSTALL_LIB_DIR}/vmaf_original.lib"
		COMMENT "Renaming libvmaf.a to vmaf_original.lib for MSVC compatibility"
	)

	# After rename we need to create a bundle with pthread library
	library_import_static_hint(PTHREAD_WIN32_LIBRARY "pthreadVC3" "${FFMPEG_INSTALL_LIB_DIR}")
	add_custom_command(TARGET vmaf_install POST_BUILD
		COMMAND ${ENV_RUNNER_SILENT} lib /OUT:${VMAF_LIBRARY} "${FFMPEG_INSTALL_LIB_DIR}/vmaf_original.lib" "${PTHREAD_WIN32_LIBRARY}"
		COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E remove "${FFMPEG_INSTALL_LIB_DIR}/vmaf_original.lib"
		COMMENT "Copying pthread_win32.lib to install directory for VMAF"
	)
endif()
add_custom_command(TARGET vmaf_install POST_BUILD
	COMMAND ${ENV_CMAKE_SILENT_COMMAND} -P "${VMAF_RESET_REPO}"
	COMMENT "Resetting VMAF source tree after internal install"
)

set(vmaf_outputs "${VMAF_LIBRARY}" PARENT_SCOPE)
if(MSVC)
	set(vmaf_links pthread_win32 PARENT_SCOPE)
	set(vmaf_deps pthread_win32_install PARENT_SCOPE)
endif()