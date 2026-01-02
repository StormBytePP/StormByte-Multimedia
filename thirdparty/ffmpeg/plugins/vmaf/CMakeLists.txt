set(VMAF_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/libvmaf")
ensure_build_dir(VMAF_BUILD_DIR)
if(FFMPEG_BUNDLED STREQUAL "static")
	library_import_static_hint(VMAF_LIBRARY "vmaf" "${FFMPEG_INSTALL_LIB_DIR}")
else()
	library_import_hint(VMAF_LIBRARY "vmaf" "${FFMPEG_INSTALL_LIB_DIR}")
endif()

set(VMAF_MESON_OPTIONS
    "-Ddefault_library=${MESON_SHARED_MODE}"
    "-Denable_tests=false"
	"-Denable_docs=false"
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
	"${VMAF_SRC_DIR}"
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

add_custom_target(vmaf_build
	COMMAND_EXPAND_LISTS
	COMMAND ${ENV_NINJA_COMMAND} -C "${VMAF_BUILD_DIR}"
	WORKING_DIRECTORY ${VMAF_BUILD_DIR}
	COMMENT "Building VMAF library"
)

add_custom_target(vmaf_install
	COMMAND_EXPAND_LISTS
	COMMAND ${ENV_NINJA_SILENT_COMMAND} -C "${VMAF_BUILD_DIR}" install
	WORKING_DIRECTORY ${VMAF_BUILD_DIR}
	COMMENT "Installing VMAF library internally"
	DEPENDS vmaf_build
)

set(vmaf_outputs "${VMAF_LIBRARY}" PARENT_SCOPE)