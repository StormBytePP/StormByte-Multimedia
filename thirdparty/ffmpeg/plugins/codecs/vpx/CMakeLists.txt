set(VPX_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
ensure_build_dir(VPX_BUILD_DIR)
set(VPX_COMPONENT "VPX codec")

# On x86_64, avoid picking ARM sources; on aarch64/arm64, do the opposite.
set(VPX_PATCH_FILE "${CMAKE_CURRENT_LIST_DIR}/0001-Only-add-arm-asm-files-when-target-is-arm.patch")
create_git_reset_file(VPX_RESET_REPO "VPX" "${VPX_SRC_DIR}")
create_git_patch_file(VPX_PATCH_REPO "VPX" "${VPX_SRC_DIR}" "${VPX_PATCH_FILE}")
include("${VPX_RESET_REPO}")
include("${VPX_PATCH_REPO}")

# Shared library builds are limited on some platforms (see upstream);
# Windows still needs static as before.
set(VPX_OPTIONS
	"-Dlibs=enabled"
	"-Dexamples=disabled"
	"-Dtools=disabled"
	"-Ddocs=disabled"
	"-Dinstall_docs=disabled"
	"-Ddecode_perf_tests=disabled"
	"-Dencode_perf_tests=disabled"
	"-Dunit_tests=disabled"
	"-Dvp8_encoder=enabled"
	"-Dvp8_decoder=enabled"
	"-Dvp9_encoder=enabled"
	"-Dvp9_decoder=enabled"
	"-Dvp9_highbitdepth=enabled"
	"-Dmultithread=enabled"
)

# Architecture-specific CPU / feature flags
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64|ARM64)$")
	list(APPEND VPX_OPTIONS
		"-Dcpu=aarch64"
	)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm|armv7)")
	list(APPEND VPX_OPTIONS
		"-Dcpu=arm"
	)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64)$")
	list(APPEND VPX_OPTIONS
		"-Dcpu=x86_64"
		# Force-disable non-x86 architecture features to avoid selecting ARM/NEON sources
		"-Ddisable_features=neon,neon_asm,sve,sve2,msa"
		"-Dforce_disable_features=neon,neon_asm,arm,arm64,sve,sve2,msa"
	)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(i[3-6]86|x86)$")
	list(APPEND VPX_OPTIONS
		"-Dcpu=x86"
		"-Ddisable_features=neon,neon_asm,sve,sve2,msa"
		"-Dforce_disable_features=neon,neon_asm,arm,arm64,sve,sve2,msa"
	)
else()
	message(STATUS "VPX: unknown processor '${CMAKE_SYSTEM_PROCESSOR}', letting Meson auto-detect cpu")
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
	${PLUGIN_LEVEL}
)
include("${VPX_LIBRARY_CREATE_FILE}")

if(MSVC)
	# On MSVC library is named libvpx.a so we need to rename it to vpx.lib
	add_custom_command(TARGET vpx_install POST_BUILD
		COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename "${BUILDMASTER_INSTALL_LIBDIR}/libvpx.a" "${BUILDMASTER_INSTALL_LIBDIR}/vpx.lib"
		COMMENT "Renaming libvpx.a to vpx.lib for MSVC compatibility"
	)
endif()

register_plugin(vpx "-Dlibvpx=enabled -Dlibvpx_vp8_encoder=enabled -Dlibvpx_vp8_decoder=enabled -Dlibvpx_vp9_encoder=enabled -Dlibvpx_vp9_decoder=enabled")