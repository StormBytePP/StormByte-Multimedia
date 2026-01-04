message(STATUS "\t\tSetting up x265 codec")

# Set main source
set(X265_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/src/source")

# In Windows, this project creates the suffix "-static" for static libraries
if(WIN32)
	set(X265_SUFFIX_STATIC "-static")
else()
	set(X265_SUFFIX_STATIC "")
endif()

# Need to patch CMakeLists to delete invalid POLICY settings for CMake < 3.5
# and add an include to make it compile with GCC 15+.
set(X265_PATCH_FILE "${CMAKE_CURRENT_LIST_DIR}/0001-Various-patches-to-make-it-work.patch")
set(X265_CUSTOM_PATCHES ${X265_PATCH_FILE})
create_git_reset_file(X265_RESET_REPO "X265" "${X265_SRC_DIR}")
create_git_patch_file(X265_PATCH_REPO "X265" "${X265_SRC_DIR}" "${X265_CUSTOM_PATCHES}")

# First reset the repo to a clean state
include("${X265_RESET_REPO}")

# Patch the x265 sources
include("${X265_PATCH_REPO}")

# Build 3 phases: 12bit, 10bit+hdr10+ and 8bit (this last one exposing C API)
# 12-bit build
ensure_build_dir(X265_BUILD_DIR_12 "12bit")
# Can't use library_import_static_hint here as the 12-bit build will not be installed
set(X265_LIBRARY_12 "${X265_BUILD_DIR_12}/${CMAKE_STATIC_LIBRARY_PREFIX}x265${X265_SUFFIX_STATIC}${CMAKE_STATIC_LIBRARY_SUFFIX}")
set(X265_COMPONENT_12 "X265 codec (12-bit)")
set(X265_OPTIONS_12
	"-DENABLE_SHARED=OFF"
	"-DENABLE_PIC=ON"
	"-DENABLE_SVT_HEVC=OFF"
	"-DENABLE_LIBVMAF=ON"
	"-DENABLE_LIBNUMA=OFF"
	"-DGIT_ARCHETYPE=1"
	"-DENABLE_CLI=OFF"
	"-DHIGH_BIT_DEPTH=ON"
	"-DMAIN12=ON"
	"-DEXPORT_C_API=OFF"
	"-DLIB_INSTALL_DIR=${CMAKE_INSTALL_LIBDIR}"
	"-DCMAKE_ASM_NASM_FLAGS=-w-pp-macro-params-legacy"
)
create_cmake_stages(
	X265_CONFIGURE_12
	X265_BUILD_12
	X265_INSTALL_12
	x265_12bit
	"${X265_COMPONENT_12}"
	"${X265_SRC_DIR}"
	"${X265_BUILD_DIR_12}"
	"${X265_OPTIONS_12}"
	"static"
	"${X265_LIBRARY_12}"
)
add_custom_target(x265_12bit_configure
	COMMAND ${ENV_CMAKE_SILENT_COMMAND} -P "${X265_CONFIGURE_12}"
	WORKING_DIRECTORY "${X265_BUILD_DIR_12}"
	COMMENT "Configuring x265 codec (12-bit)"
	DEPENDS vmaf_install
)
include("${X265_BUILD_12}")
add_dependencies(x265_12bit_build x265_12bit_configure)
# Not installing 12-bit separately, as it will be merged into 8-bit build

# 10-bit build
ensure_build_dir(X265_BUILD_DIR_10 "10bit")
set(X265_LIBRARY_10 "${X265_BUILD_DIR_10}/${CMAKE_STATIC_LIBRARY_PREFIX}x265${X265_SUFFIX_STATIC}${CMAKE_STATIC_LIBRARY_SUFFIX}")
set(X265_LIBRARY_HDR10_PLUS "${X265_BUILD_DIR_10}/${CMAKE_STATIC_LIBRARY_PREFIX}hdr10plus${CMAKE_STATIC_LIBRARY_SUFFIX}")
set(X265_COMPONENT_10 "X265 codec (10-bit)")
set(X265_OPTIONS_10
	"-DENABLE_SHARED=OFF"
	"-DENABLE_PIC=ON"
	"-DENABLE_SVT_HEVC=OFF"
	"-DENABLE_LIBVMAF=ON"
	"-DENABLE_LIBNUMA=OFF"
	"-DGIT_ARCHETYPE=1"
	"-DENABLE_CLI=OFF"
	"-DENABLE_HDR10_PLUS=ON"
	"-DHIGH_BIT_DEPTH=ON"
	"-DMAIN12=OFF"
	"-DEXPORT_C_API=OFF" # Static build needs this whilst static don't
	"-DLIB_INSTALL_DIR=${CMAKE_INSTALL_LIBDIR}"
	"-DCMAKE_ASM_NASM_FLAGS=-w-pp-macro-params-legacy"
)
create_cmake_stages(
	X265_CONFIGURE_10
	X265_BUILD_10
	X265_INSTALL_10
	x265_10bit
	"${X265_COMPONENT_10}"
	"${X265_SRC_DIR}"
	"${X265_BUILD_DIR_10}"
	"${X265_OPTIONS_10}"
	"static"
	"${X265_LIBRARY_10}"
	3
)
add_custom_target(x265_10bit_configure
	COMMAND ${ENV_CMAKE_SILENT_COMMAND} -P "${X265_CONFIGURE_10}"
	WORKING_DIRECTORY "${X265_BUILD_DIR_10}"
	COMMENT "Configuring x265 codec (10-bit)"
	DEPENDS vmaf_install
)
include("${X265_BUILD_10}")
add_dependencies(x265_10bit_build x265_10bit_configure)
# Not installing 10-bit separately, as it will be merged into 8-bit build

# 8-bit build
if(FFMPEG_BUNDLED_TYPE STREQUAL "static")
	set(CMAKE_SHARED_MODE "OFF")
else()
	set(CMAKE_SHARED_MODE "ON")
endif()
ensure_build_dir(X265_BUILD_DIR_8 "8bit")
set(X265_LIBRARY_8 "${X265_BUILD_DIR_8}/${CMAKE_STATIC_LIBRARY_PREFIX}x265${X265_SUFFIX_STATIC}${CMAKE_STATIC_LIBRARY_SUFFIX}")
set(X265_COMPONENT_8 "X265 codec (8-bit)")
set(X265_EXTRA_LIBS "${X265_LIBRARY_12};${X265_LIBRARY_10}")
set(X265_OPTIONS_8
	"-DENABLE_SHARED=${CMAKE_SHARED_MODE}"
	"-DENABLE_PIC=ON"
	"-DENABLE_SVT_HEVC=OFF"
	"-DENABLE_LIBVMAF=ON"
	"-DENABLE_LIBNUMA=OFF"
	"-DGIT_ARCHETYPE=1"
	"-DENABLE_CLI=OFF"
	"-DHIGH_BIT_DEPTH=OFF"
	"-DEXPORT_C_API=ON"
	"-DLIB_INSTALL_DIR=${CMAKE_INSTALL_LIBDIR}"
	"-DCMAKE_ASM_NASM_FLAGS=-DPIC -w-pp-macro-params-legacy"
	"-DEXTRA_LIB='${X265_EXTRA_LIBS}'"
	"-DLINKED_10BIT=ON"
	"-DLINKED_12BIT=ON"
)
create_cmake_dependant_component(
	X265_LIBRARY_CREATE_FILE
	"x265"
	"${X265_COMPONENT_8}"
	"${X265_SRC_DIR}"
	"${X265_BUILD_DIR_8}"
	"${X265_OPTIONS_8}"
	"${FFMPEG_BUNDLED_TYPE}"
	"x265"
	"x265_12bit_build;x265_10bit_build"
)
include("${X265_LIBRARY_CREATE_FILE}")

# Hack: In Windows, under "not release" builds, the x265 8-bit target
# tries to install x265.pdb to the install directory, but this fails
# because its name is libx265.pdb instead. To avoid this, we build the
# target without the install step, and manually copy the library file
# to the expected filename.

# Ensure the main 8-bit `x265_build` target runs the 12-bit and 10-bit
# builds first so their configure/build stages are executed.
add_dependencies(x265_build x265_12bit_build x265_10bit_build)
if(MSVC)
	if(FFMPEG_BUNDLED_TYPE STREQUAL "shared")
		if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
			add_custom_command(TARGET x265_build POST_BUILD
				COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E copy "${X265_BUILD_DIR_8}/libx265.pdb" "${X265_BUILD_DIR_8}/x265.pdb"
				WORKING_DIRECTORY "${X265_BUILD_DIR_8}"
				COMMENT "Correcting x265 codec (8-bit) debug library filenames"
				VERBATIM
			)
		endif()
		add_custom_command(TARGET x265_build POST_BUILD
			COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename "${X265_BUILD_DIR_8}/libx265.dll" "${X265_BUILD_DIR_8}/x265.dll"
			COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename "${X265_BUILD_DIR_8}/libx265.lib" "${X265_BUILD_DIR_8}/x265.lib"
			WORKING_DIRECTORY "${X265_BUILD_DIR_8}"
			COMMENT "Correcting x265 codec (8-bit) library filenames"
			VERBATIM
		)
	else()
		# Windows creates x265-static.lib when we expected x265.lib
		add_custom_command(TARGET x265_install POST_BUILD
			COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename "${BUILDMASTER_INSTALL_LIBDIR}/x265-static.lib" "${BUILDMASTER_INSTALL_LIBDIR}/x265.lib"
			COMMENT "Renaming x265-static.lib to x265.lib"
		)
	endif()
endif()

# Restore x265 CMakeLists.txt after the final x265 (8-bit) target has
# been built.
add_custom_command(TARGET x265_install POST_BUILD
	COMMAND ${ENV_CMAKE_SILENT_COMMAND} -P "${X265_RESET_REPO}"
	WORKING_DIRECTORY "${X265_SRC_DIR}"
	COMMENT "Restoring x265 sources"
	VERBATIM
)

if(FFMPEG_BUNDLED_TYPE STREQUAL "static")
	# Repack all the libraries altogether into a single x265.lib
	library_import_static_hint(X265_LIBRARY "x265")
	# VMAF needs to be included in the static repack for ffmpeg configure to detect it
	library_import_static_hint(VMAF_LIBRARY "vmaf")
	create_bundle_static_libraries(
		BUNDLE_X265_LIBRARIES
		"x265"
		"${X265_LIBRARY_8};${X265_LIBRARY_10};${X265_LIBRARY_HDR10_PLUS};${X265_LIBRARY_12};${VMAF_LIBRARY}"
	)

	add_custom_command(TARGET x265_install POST_BUILD
		COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E remove "${X265_LIBRARY}"
		COMMAND ${BUNDLE_X265_LIBRARIES}
		COMMENT "Repacking x265 static libraries into a single x265"
	)
endif()
