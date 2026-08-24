message_indented("Setting up harfbuzz" ${PLUGIN_LEVEL})

set(HARFBUZZ_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
ensure_build_dir(HARFBUZZ_BUILD_DIR)
set(HARFBUZZ_COMPONENT "harfbuzz")

# Windows CreateProcess limit: gen-harfbuzzcc used to get hundreds of paths
# via @INPUT@. Portable listfile approach (all OSes).
create_git_patch_file(
	HARFBUZZ_PATCH_CREATE_FILE
	"harfbuzz"
	"harfbuzz long paths"
	"${HARFBUZZ_SRC_DIR}"
	"${CMAKE_CURRENT_LIST_DIR}/0001-harfbuz-fix-long-paths.patch"
)
include("${HARFBUZZ_PATCH_CREATE_FILE}")

set(HARFBUZZ_OPTIONS
	"-Dtests=disabled"
	"-Ddocs=disabled"
	"-Dbenchmark=disabled"
	"-Dicu=disabled"
	"-Dglib=disabled"
	"-Dcairo=disabled"
	"-Dfreetype=enabled"
)

create_meson_dependant_component(
	HARFBUZZ_LIBRARY_CREATE_FILE
	"harfbuzz"
	"${HARFBUZZ_COMPONENT}"
	"${HARFBUZZ_SRC_DIR}"
	"${HARFBUZZ_BUILD_DIR}"
	"${HARFBUZZ_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"harfbuzz"
	"freetype_install"
	${PLUGIN_LEVEL}
)
include("${HARFBUZZ_LIBRARY_CREATE_FILE}")

# Meson installs libharfbuzz*.a; MSVC consumers expect *.lib (no lib prefix).
# Rename all variants that this build installs (main is required by libass).
if(FFMPEG_BUNDLED_TYPE STREQUAL "static" AND MSVC)
	add_custom_command(TARGET harfbuzz_install POST_BUILD
		COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename
			"${BUILDMASTER_INSTALL_LIBDIR}/libharfbuzz.a"
			"${BUILDMASTER_INSTALL_LIBDIR}/harfbuzz.lib"
		COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename
			"${BUILDMASTER_INSTALL_LIBDIR}/libharfbuzz-subset.a"
			"${BUILDMASTER_INSTALL_LIBDIR}/harfbuzz-subset.lib"
		COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename
			"${BUILDMASTER_INSTALL_LIBDIR}/libharfbuzz-raster.a"
			"${BUILDMASTER_INSTALL_LIBDIR}/harfbuzz-raster.lib"
		COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename
			"${BUILDMASTER_INSTALL_LIBDIR}/libharfbuzz-vector.a"
			"${BUILDMASTER_INSTALL_LIBDIR}/harfbuzz-vector.lib"
		COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename
			"${BUILDMASTER_INSTALL_LIBDIR}/libharfbuzz-gpu.a"
			"${BUILDMASTER_INSTALL_LIBDIR}/harfbuzz-gpu.lib"
		COMMENT "Renaming harfbuzz libraries for MSVC"
	)
endif()

register_dependency(harfbuzz)