set(VPX_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
ensure_build_dir(VPX_BUILD_DIR)
set(VPX_COMPONENT "VPX codec")
# We need to patch sources so arm files are not picked when building on x86_64
set(VPX_PATCH_FILE "${CMAKE_CURRENT_LIST_DIR}/0001-Only-add-arm-asm-files-when-target-is-arm.patch")
create_git_reset_file(VPX_RESET_REPO "VPX" "${VPX_SRC_DIR}")
create_git_patch_file(VPX_PATCH_REPO "VPX" "${VPX_SRC_DIR}" "${VPX_PATCH_FILE}")
include("${VPX_RESET_REPO}")
include("${VPX_PATCH_REPO}")

# On Windows it is needed to force static build as of error:
# ERROR: Problem encountered: Shared library build is only supported
# on ELF and Darwin for now
set(VPX_OPTIONS
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
if(FFMPEG_BUNDLED_TYPE STREQUAL "static")
	list(APPEND VPX_OPTIONS "-Db_staticpic=true")
endif()

create_meson_component(
	VPX_LIBRARY_CREATE_FILE
	"vpx"
	"${VPX_COMPONENT}"
	"${VPX_SRC_DIR}"
	"${VPX_BUILD_DIR}"
	"${VPX_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"vpx"
	2
)
include("${VPX_LIBRARY_CREATE_FILE}")

if(MSVC)
	# On MSVC library is named libvpx.a so we need to rename it to vpx.lib
	add_custom_command(TARGET vpx_install POST_BUILD
		COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename "${BUILDMASTER_INSTALL_LIBDIR}/libvpx.a" "${BUILDMASTER_INSTALL_LIBDIR}/vpx.lib"
		COMMENT "Renaming libvpx.a to vpx.lib for MSVC compatibility"
	)
endif()