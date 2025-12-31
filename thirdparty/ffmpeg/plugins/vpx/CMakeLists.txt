set(VPX_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
ensure_build_dir(VPX_BUILD_DIR)
if(FFMPEG_BUNDLED STREQUAL "static")
	library_import_static_hint(VPX_LIBRARY "vpx" "${FFMPEG_INSTALL_LIB_DIR}")
else()
	library_import_hint(VPX_LIBRARY "vpx" "${FFMPEG_INSTALL_LIB_DIR}")
endif()

# We need to patch sources so arm files are not picked when building on x86_64
set(VPX_PATCH_FILE "${CMAKE_CURRENT_LIST_DIR}/0001-Only-add-arm-asm-files-when-target-is-arm.patch")
create_git_reset_file(VPX_RESET_REPO "VPX" "${VPX_SRC_DIR}")
create_git_patch_file(VPX_PATCH_REPO "VPX" "${VPX_SRC_DIR}" "${VPX_PATCH_FILE}")
include("${VPX_RESET_REPO}")
include("${VPX_PATCH_REPO}")

# On Windows it is needed to force static build as of error:
# ERROR: Problem encountered: Shared library build is only supported
# on ELF and Darwin for now
set(VPX_MESON_OPTIONS
    "-Ddefault_library=${MESON_SHARED_MODE}"
    "-Dcpu=x86_64"
    "-Dlibs=enabled"
    "-Dexamples=disabled"
    "-Dtools=disabled"
    "-Ddocs=disabled"
    "-Dinstall_docs=disabled"
    "-Ddecode_perf_tests=disabled"
    "-Dencode_perf_tests=disabled"
    "-Dunit_tests=disabled"
    "-Dvp8=enabled"
    "-Dvp9=enabled"
	"-Dvp9_highbitdepth=enabled"
    # Force-disable non-x86 architecture features to avoid selecting ARM/NEON sources
    "-Ddisable_features=neon,neon_asm,sve,sve2,msa"
    "-Dforce_disable_features=neon,neon_asm,arm,arm64,sve,sve2,msa"
    "-Dmultithread=enabled"
)
if(FFMPEG_BUNDLED STREQUAL "static")
	list(APPEND VPX_MESON_OPTIONS "-Db_staticpic=true")
endif()

create_meson_setup_file(
	VPX_MESON_SETUP_FILE
	"VPX codec"
	"${VPX_SRC_DIR}"
	"${VPX_BUILD_DIR}"
	"${FFMPEG_INSTALL_DIR}"
	"${VPX_MESON_OPTIONS}"
	2
)

if (NOT EXISTS "${VPX_BUILD_DIR}/build.ninja")
	include("${VPX_MESON_SETUP_FILE}")
else()
	message(STATUS "Skipping meson setup; build files already present for vpx")
endif()

add_custom_target(vpx_build
	COMMAND_EXPAND_LISTS
	COMMAND ${ENV_NINJA_COMMAND} -C "${VPX_BUILD_DIR}"
	WORKING_DIRECTORY ${VPX_BUILD_DIR}
	COMMENT "Building VPX codec"
	USES_TERMINAL
)

add_custom_target(vpx_install
	COMMAND_EXPAND_LISTS
	COMMAND ${ENV_NINJA_SILENT_COMMAND} -C "${VPX_BUILD_DIR}" install
	WORKING_DIRECTORY ${VPX_BUILD_DIR}
	COMMENT "Installing VPX codec internally"
	DEPENDS vpx_build
)

add_custom_command(TARGET vpx_install POST_BUILD
	COMMAND ${ENV_CMAKE_SILENT_COMMAND} -P "${VPX_RESET_REPO}"
	WORKING_DIRECTORY "${VPX_SRC_DIR}"
	COMMENT "Reverting VPX source code changes"
)

# Windows creates libvpx.a when we expected vpx.lib
if (MSVC)
	add_custom_command(TARGET vpx_install POST_BUILD
		COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename "${FFMPEG_INSTALL_LIB_DIR}/libvpx.a" "${FFMPEG_INSTALL_LIB_DIR}/vpx.lib"
		COMMENT "Renaming libvpx.a to vpx.lib"
	)
endif()

set(vpx_outputs "${VPX_LIBRARY}" PARENT_SCOPE)