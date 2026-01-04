set(VMAF_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/libvmaf")
ensure_build_dir(VMAF_BUILD_DIR)

# Need to patch to disable tool building
set(VMAF_WINDOWS_PATH_FILE "${CMAKE_CURRENT_LIST_DIR}/0001-MSVC-corrections.patch")
set(VMAF_TOOLS_PATH_FILE "${CMAKE_CURRENT_LIST_DIR}/0002-Disable-tool-building.patch")
create_git_reset_file(VMAF_RESET_REPO "VMAF Reset repo" "${VMAF_SRC_DIR}")
create_git_patch_file(VMAF_PATCH_REPO1 "VMAF Apply patch 1" "${VMAF_SRC_DIR}" "${VMAF_WINDOWS_PATH_FILE}")
create_git_patch_file(VMAF_PATCH_REPO2 "VMAF Apply patch 2" "${VMAF_SRC_DIR}" "${VMAF_TOOLS_PATH_FILE}")

include(${VMAF_RESET_REPO})
include(${VMAF_PATCH_REPO1})
include(${VMAF_PATCH_REPO2})

set(VMAF_COMPONENT "VMAF filter")
set(VMAF_OPTIONS
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

# In Windows we depend on pthread_win32
if(MSVC)
	create_meson_dependant_component(
		VMAF_LIBRARY_CREATE_FILE
		"vmaf"
		"${VMAF_COMPONENT}"
		"${VMAF_SRC_DIR}"
		"${VMAF_BUILD_DIR}"
		"${VMAF_OPTIONS}"
		"${FFMPEG_BUNDLED_TYPE}"
		"vmaf"
		"pthread-win32_install"
		2
	)
else()
	create_meson_component(
		VMAF_LIBRARY_CREATE_FILE
		"vmaf"
		"${VMAF_COMPONENT}"
		"${VMAF_SRC_DIR}"
		"${VMAF_BUILD_DIR}"
		"${VMAF_OPTIONS}"
		"${FFMPEG_BUNDLED_TYPE}"
		"vmaf"
		2
	)
endif()
include("${VMAF_LIBRARY_CREATE_FILE}")

if(FFMPEG_BUNDLED_TYPE STREQUAL "static" AND MSVC)
	# We have to repack the VMAF static library to link pthread_win32 into it
	set(VMAF_LIBRARY "${VMAF_BUILD_DIR}/src/libvmaf.a")
	library_import_static_hint(PTHREAD_WIN32_LIBRARY "pthreadVC3")
	create_bundle_static_libraries(VMAF_BUNDLE_FILE "vmaf" "${VMAF_LIBRARY};${PTHREAD_WIN32_LIBRARY}")
	add_custom_command(TARGET vmaf_install POST_BUILD
		# First remove the original static library (which is also badly named)
		COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E remove "${BUILDMASTER_INSTALL_LIBDIR}/libvmaf.a"
		COMMAND ${ENV_RUNNER_SILENT} ${VMAF_BUNDLE_FILE}
		COMMENT "Bundling VMAF with PthreadWin32"
	)
endif()

add_custom_command(TARGET vmaf_install POST_BUILD
	COMMAND ${ENV_CMAKE_SILENT_COMMAND} -P "${VMAF_RESET_REPO}"
	COMMENT "Resetting VMAF source tree after internal install"
)