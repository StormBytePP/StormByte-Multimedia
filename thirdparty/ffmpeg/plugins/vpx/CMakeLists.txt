set(VPX_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
ensure_build_dir(VPX_BUILD_DIR)
library_import_hint(VPX_LIBRARY "vpx" "${FFMPEG_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}")

set(VPX_MESON_OPTIONS
	"-Ddefault_library=shared"
	"-Dcpu=x86_64"
	"--prefix=${FFMPEG_INSTALL_DIR}"
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
	"-Dvp9_highbitdepth=disabled"
	# Force-disable non-x86 architecture features to avoid selecting ARM/NEON sources
	"-Dforce_disable_features=neon,neon_asm,arm,arm64,sve,sve2,msa"
	"-Dmultithread=enabled"
)

create_meson_setup_file(
	VPX_MESON_SETUP_FILE
	"VPX codec"
	"${VPX_SRC_DIR}"
	"${VPX_BUILD_DIR}"
	"${FFMPEG_PLUGINS_INSTALL_DIR}"
	"${VPX_MESON_OPTIONS}"
)

if (NOT EXISTS "${VPX_BUILD_DIR}/build.ninja")
	include("${VPX_MESON_SETUP_FILE}")
else()
	message(STATUS "Skipping meson setup; build files already present for vpx")
endif()

add_custom_target(_vpx_build
	COMMAND_EXPAND_LISTS
	COMMAND ${ENV_NINJA_COMMAND} -C "${VPX_BUILD_DIR}"
	WORKING_DIRECTORY ${VPX_BUILD_DIR}
	COMMENT "Building VPX codec"
	USES_TERMINAL
)

add_custom_target(_vpx_install
	COMMAND_EXPAND_LISTS
	COMMAND ${ENV_RUNNER_SILENT} ${ENV_NINJA_COMMAND} -C "${VPX_BUILD_DIR}" install
	WORKING_DIRECTORY ${VPX_BUILD_DIR}
	COMMENT "Installing VPX codec internally"
	DEPENDS _vpx_build
)

add_custom_target(vpx DEPENDS _vpx_install)
