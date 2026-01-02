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
# VMAF is disabled on Windows as it does not compile (requires pthread)
if(MSVC)
	set(WITH_VMAF OFF)
	set(INDENT 3)
else()
	set(WITH_VMAF ON)
	set(INDENT 0)
endif()
# 12-bit build
ensure_build_dir(X265_BUILD_DIR_12 "12bit")
library_import_static_hint(X265_LIBRARY_12 "x265${X265_SUFFIX_STATIC}" "${X265_BUILD_DIR_12}")
set(X265_COMPONENT_12 "X265 codec (12-bit)")
set(X265_OPTIONS_12
	"-DENABLE_SHARED=OFF"
	"-DENABLE_PIC=ON"
	"-DENABLE_SVT_HEVC=OFF"
	"-DENABLE_LIBVMAF=${WITH_VMAF}"
	"-DENABLE_LIBNUMA=OFF"
	"-DGIT_ARCHETYPE=1"
	"-DENABLE_CLI=OFF"
	"-DHIGH_BIT_DEPTH=ON"
	"-DMAIN12=ON"
	"-DEXPORT_C_API=OFF"
	"-DLIB_INSTALL_DIR=${CMAKE_INSTALL_LIBDIR}"
	"-DCMAKE_ASM_NASM_FLAGS=-w-pp-macro-params-legacy"
)
create_cmake_configure_file(
	X265_CMAKE_CONFIGURE_FILE_12
	"${X265_COMPONENT_12}"
	"${X265_SRC_DIR}"
	"${X265_BUILD_DIR_12}"
	"${FFMPEG_INSTALL_DIR}"
	"${X265_OPTIONS_12}"
	${INDENT}
)
if(MSVC)
	include(${X265_CMAKE_CONFIGURE_FILE_12})

	add_custom_target(x265_12bit_build
		COMMAND ${ENV_CMAKE_COMMAND} --build "${X265_BUILD_DIR_12}"
		WORKING_DIRECTORY "${X265_BUILD_DIR_12}"
		COMMENT "Building X265 codec (12-bit)"
		VERBATIM
	)
else()
	add_custom_target(x265_12bit_configure
		COMMAND ${ENV_CMAKE_SILENT_COMMAND} -P "${X265_CMAKE_CONFIGURE_FILE_12}"
		WORKING_DIRECTORY "${X265_BUILD_DIR_12}"
		COMMENT "Configuring X265 codec (12-bit)"
		DEPENDS vmaf_install
	)

	add_custom_target(x265_12bit_build
		COMMAND ${ENV_CMAKE_COMMAND} --build "${X265_BUILD_DIR_12}"
		WORKING_DIRECTORY "${X265_BUILD_DIR_12}"
		COMMENT "Building X265 codec (12-bit)"
		VERBATIM
		DEPENDS x265_12bit_configure
	)
endif()

# 10-bit build
ensure_build_dir(X265_BUILD_DIR_10 "10bit")
library_import_static_hint(X265_LIBRARY_10 "x265${X265_SUFFIX_STATIC}" "${X265_BUILD_DIR_10}")
library_import_static_hint(X265_LIBRARY_HDR10_PLUS "hdr10plus" "${X265_BUILD_DIR_10}")
set(X265_COMPONENT_10 "X265 codec (10-bit)")
set(X265_OPTIONS_10
	"-DENABLE_SHARED=OFF"
	"-DENABLE_PIC=ON"
	"-DENABLE_SVT_HEVC=OFF"
	"-DENABLE_LIBVMAF=${WITH_VMAF}"
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
create_cmake_configure_file(
	X265_CMAKE_CONFIGURE_FILE_10
	"${X265_COMPONENT_10}"
	"${X265_SRC_DIR}"
	"${X265_BUILD_DIR_10}"
	"${FFMPEG_INSTALL_DIR}"
	"${X265_OPTIONS_10}"
	${INDENT}
)
if(MSVC)
	include(${X265_CMAKE_CONFIGURE_FILE_10})

	add_custom_target(x265_10bit_build
		COMMAND ${ENV_CMAKE_COMMAND} --build "${X265_BUILD_DIR_10}"
		WORKING_DIRECTORY "${X265_BUILD_DIR_10}"
		COMMENT "Building X265 codec (10-bit)"
		VERBATIM
	)
else()
	add_custom_target(x265_10bit_configure
		COMMAND ${ENV_CMAKE_SILENT_COMMAND} -P "${X265_CMAKE_CONFIGURE_FILE_10}"
		WORKING_DIRECTORY "${X265_BUILD_DIR_10}"
		COMMENT "Configuring X265 codec (10-bit)"
		DEPENDS vmaf_install
	)

	add_custom_target(x265_10bit_build
		COMMAND ${ENV_CMAKE_COMMAND} --build "${X265_BUILD_DIR_10}"
		WORKING_DIRECTORY "${X265_BUILD_DIR_10}"
		COMMENT "Building X265 codec (10-bit)"
		VERBATIM
		DEPENDS x265_10bit_configure
	)
endif()

# 8-bit build
ensure_build_dir(X265_BUILD_DIR_8 "8bit")
if(FFMPEG_BUNDLED STREQUAL "static")
	library_import_static_hint(X265_LIBRARY_8 "x265${X265_SUFFIX_STATIC}" "${X265_BUILD_DIR_8}")
else()
	library_import_hint(X265_LIBRARY_8 "x265${X265_SUFFIX_STATIC}" "${X265_BUILD_DIR_8}")
endif()
set(X265_COMPONENT_8 "X265 codec (8-bit)")
set(X265_EXTRA_LIBS "${X265_LIBRARY_12};${X265_LIBRARY_10}")
set(X265_OPTIONS_8
	"-DENABLE_SHARED=${CMAKE_SHARED_MODE}"
	"-DENABLE_PIC=ON"
	"-DENABLE_SVT_HEVC=OFF"
	"-DENABLE_LIBVMAF=${WITH_VMAF}"
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
create_cmake_configure_file(
	X265_CMAKE_CONFIGURE_FILE_8
	"${X265_COMPONENT_8}"
	"${X265_SRC_DIR}"
	"${X265_BUILD_DIR_8}"
	"${FFMPEG_INSTALL_DIR}"
	"${X265_OPTIONS_8}"
)

add_custom_target(x265_8bit_configure
	COMMAND ${ENV_CMAKE_SILENT_COMMAND} -P "${X265_CMAKE_CONFIGURE_FILE_8}"
	WORKING_DIRECTORY "${X265_BUILD_DIR_8}"
	COMMENT "Configuring x265 codec (8-bit)"
	DEPENDS x265_10bit_build x265_12bit_build
)

add_custom_target(x265_8bit_build
	COMMAND ${ENV_CMAKE_COMMAND} --build "${X265_BUILD_DIR_8}"
	WORKING_DIRECTORY "${X265_BUILD_DIR_8}"
	COMMENT "Building x265 codec (8-bit)"
	VERBATIM
	DEPENDS x265_8bit_configure
)
# Hack: In Windows, under "not release" builds, the x265 8-bit target
# tries to install x265.pdb to the install directory, but this fails
# because its name is libx265.pdb instead. To avoid this, we build the
# target without the install step, and manually copy the library file
# to the expected filename.
if(NOT FFMPEG_BUNDLED STREQUAL "static" AND MSVC)
	if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
		add_custom_command(TARGET x265_8bit_build POST_BUILD
			COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E copy "${X265_BUILD_DIR_8}/libx265.pdb" "${X265_BUILD_DIR_8}/x265.pdb"
			WORKING_DIRECTORY "${X265_BUILD_DIR_8}"
			COMMENT "Correcting x265 codec (8-bit) debug library filenames"
			VERBATIM
		)
	endif()
	add_custom_command(TARGET x265_8bit_build POST_BUILD
		COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename "${X265_BUILD_DIR_8}/libx265.dll" "${X265_BUILD_DIR_8}/x265.dll"
		COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename "${X265_BUILD_DIR_8}/libx265.lib" "${X265_BUILD_DIR_8}/x265.lib"
		WORKING_DIRECTORY "${X265_BUILD_DIR_8}"
		COMMENT "Correcting x265 codec (8-bit) library filenames"
		VERBATIM
	)
endif()

# Install 8-bit build (with 10 and 12 bit merged in)
add_custom_target(x265_8bit_install
	COMMAND ${ENV_CMAKE_SILENT_COMMAND} --build "${X265_BUILD_DIR_8}" --target install
	WORKING_DIRECTORY "${X265_BUILD_DIR_8}"
	COMMENT "Installing x265 codec all bit internally"
	VERBATIM
	DEPENDS x265_8bit_build
)

# Windows creates x265-static.lib when we expected x265.lib
if(MSVC)
	add_custom_command(TARGET x265_8bit_install POST_BUILD
		COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename "${FFMPEG_INSTALL_LIB_DIR}/x265-static.lib" "${FFMPEG_INSTALL_LIB_DIR}/x265.lib"
		COMMENT "Renaming x265-static.lib to x265.lib"
	)
endif()

# Restore x265 CMakeLists.txt after the final x265 (8-bit) target has
# been built. `x265_8bit` depends (via `x265_static_copy`) on the 10/12-bit
# builds, so a POST_BUILD hook on `x265_8bit` will run once all phases are
# complete.
add_custom_command(TARGET x265_8bit_install POST_BUILD
	COMMAND ${ENV_CMAKE_SILENT_COMMAND} -P "${X265_RESET_REPO}"
	WORKING_DIRECTORY "${X265_SRC_DIR}"
	COMMENT "Restoring x265 sources"
	VERBATIM
)

if(FFMPEG_BUNDLED STREQUAL "static")
	library_import_static_hint(X265_LIBRARY "x265" "${FFMPEG_INSTALL_LIB_DIR}")
	if(MSVC)
		# Since we build static in Windows, we need to repack all the three libraries into one
		add_custom_command(TARGET x265_8bit_install POST_BUILD
			COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E remove "${FFMPEG_INSTALL_LIB_DIR}/x265.lib"
			COMMAND ${ENV_RUNNER_SILENT} lib /OUT:${X265_LIBRARY} "${X265_LIBRARY_8}" "${X265_LIBRARY_10}" "${X265_LIBRARY_HDR10_PLUS}" "${X265_LIBRARY_12}"
			COMMENT "Repacking x265 static libraries into a single x265.lib"
		)
	else()
		# We need to repack all the object files into a single static library
		configure_file(
			${CMAKE_CURRENT_SOURCE_DIR}/x265_repack.sh.in
			${CMAKE_CURRENT_BINARY_DIR}/x265_repack.sh
			@ONLY
		)
		# Give execution permissions to the script
		execute_process(
			COMMAND ${ENV_RUNNER_SILENT} chmod +x "${CMAKE_CURRENT_BINARY_DIR}/x265_repack.sh"
		)
		add_custom_command(
			TARGET x265_8bit_install POST_BUILD
			COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E remove "${FFMPEG_INSTALL_LIB_DIR}/libx265.a"
			COMMAND ${ENV_RUNNER_SILENT} "${CMAKE_CURRENT_BINARY_DIR}/x265_repack.sh"
			COMMENT "Repacking x265 static libraries into a single libx265.a (renamed objects)"
		)
	endif()
else()
	library_import_hint(X265_LIBRARY "x265" "${FFMPEG_INSTALL_LIB_DIR}")
endif()

# Add a dummy target for dependencies
add_custom_target(x265_install DEPENDS x265_8bit_install)
set(x265_outputs "${X265_LIBRARY}" PARENT_SCOPE)
