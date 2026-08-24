set(VMAF_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/libvmaf")
ensure_build_dir(VMAF_BUILD_DIR)

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

if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64)$")
	list(APPEND VMAF_OPTIONS "-Denable_avx512=true")
else()
	list(APPEND VMAF_OPTIONS "-Denable_avx512=false")
endif()

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
		${PLUGIN_LEVEL}
	)
endif()
include("${VMAF_LIBRARY_CREATE_FILE}")

if(MSVC AND FFMPEG_BUNDLED_TYPE STREQUAL "static")
	add_custom_command(TARGET vmaf_install POST_BUILD
		COMMAND ${CMAKE_COMMAND}
			-DDIR=${BUILDMASTER_INSTALL_LIBDIR}
			-DPATTERN=libvmaf.a
			-DOUT=vmaf.lib
			-DMODE=copy
			-P "${FFMPEG_RENAME_MSVC_LIB}"
		COMMENT "MSVC: libvmaf.a → vmaf.lib (copy)"
	)
endif()

register_plugin(vmaf "-Dlibvmaf=enabled -Dlibvmaf_filter=enabled -Dvmafmotion_filter=enabled")