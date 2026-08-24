set(VMAF_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/libvmaf")
ensure_build_dir(VMAF_BUILD_DIR)

# Need to patch to disable tool building / MSVC fixes
set(VMAF_WINDOWS_PATH_FILE "${CMAKE_CURRENT_LIST_DIR}/0001-MSVC-corrections.patch")
create_git_reset_file(VMAF_RESET_REPO vmaf "VMAF reset" "${VMAF_SRC_DIR}")
create_git_patch_file(VMAF_PATCH_REPO1 vmaf "VMAF MSVC patch" "${VMAF_SRC_DIR}" "${VMAF_WINDOWS_PATH_FILE}")

include(${VMAF_RESET_REPO})
include(${VMAF_PATCH_REPO1})

set(VMAF_COMPONENT "VMAF filter")
set(VMAF_OPTIONS
	"-Denable_tests=false"
	"-Denable_docs=false"
	"-Denable_tools=false"
	"-Denable_asm=true"
	"-Dbuilt_in_models=true"
	"-Denable_cuda=false"
	"-Denable_nvtx=false"
	"-Denable_nvcc=false"
	"-Dbundled_winpthreads=false"
)

# AVX-512 is x86_64-only; leave enabled only there
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64)$")
	list(APPEND VMAF_OPTIONS "-Denable_avx512=true")
else()
	list(APPEND VMAF_OPTIONS "-Denable_avx512=false")
endif()

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
		${PLUGIN_LEVEL}
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
		3
	)
endif()
include("${VMAF_LIBRARY_CREATE_FILE}")

#if(FFMPEG_BUNDLED_TYPE STREQUAL "static" AND MSVC)
#	# We have to repack the VMAF static library to link pthread_win32 into it
#	set(VMAF_LIBRARY "${VMAF_BUILD_DIR}/src/libvmaf.a")
#	library_import_static_hint(PTHREAD_WIN32_LIBRARY "pthreadVC3")
#	create_bundle_static_libraries(VMAF_BUNDLE_FILE "vmaf" "${VMAF_LIBRARY};${PTHREAD_WIN32_LIBRARY}")
#	add_custom_command(TARGET vmaf_install POST_BUILD
#		# First remove the original static library (which is also badly named)
#		COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E remove "${BUILDMASTER_INSTALL_LIBDIR}/libvmaf.a"
#		COMMAND ${ENV_RUNNER_SILENT} ${VMAF_BUNDLE_FILE}
#		COMMENT "Bundling VMAF with PthreadWin32"
#	)
#endif()
if(FFMPEG_BUNDLED_TYPE STREQUAL "static" AND MSVC)
	# We have to rename libvmaf.a to vmaf.lib
	add_custom_command(TARGET vmaf_install POST_BUILD
		# First remove the original static library (which is also badly named)
		COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename "${BUILDMASTER_INSTALL_LIBDIR}/libvmaf.a" "${BUILDMASTER_INSTALL_LIBDIR}/vmaf.lib"
		COMMENT "Renaming VMAF library"
	)
endif()

# Post-install git reset is handled automatically by BuildMaster (component id: vmaf)

register_plugin(vmaf "-Dlibvmaf=enabled -Dlibvmaf_filter=enabled -Dvmafmotion_filter=enabled")