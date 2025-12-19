if(WITH_FFMPEG STREQUAL "BUNDLED")
	find_program(MESON_EXECUTABLE meson)
	find_program(NINJA_EXECUTABLE ninja)
	if(NOT MESON_EXECUTABLE)
		message(FATAL_ERROR "Meson not found: required to build bundled Opus. Install 'meson'.")
	endif()
	if(NOT NINJA_EXECUTABLE)
		message(FATAL_ERROR "Ninja not found: required to build bundled Opus. Install 'ninja'.")
	endif()

	set(VPX_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
	set(VPX_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/meson-build")
	set(VPX_LIBRARY "${FFMPEG_EXTERNAL_PATH}/${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}vpx${CMAKE_STATIC_LIBRARY_SUFFIX}")
	file(MAKE_DIRECTORY "${VPX_BUILD_DIR}")

	set(VPX_MESON_OPTIONS
		"-Ddefault_library=static"
		"-Dbuildtype=release"
		"-Dcpu=x86_64"
		"--prefix=${FFMPEG_EXTERNAL_PATH}"
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

	if (NOT EXISTS "${VPX_BUILD_DIR}/build.ninja")
		message(STATUS "Configuring bundled VPX with Meson")
		execute_process(
			COMMAND ${MESON_EXECUTABLE} setup ${VPX_BUILD_DIR} ${VPX_SRC_DIR} ${VPX_MESON_OPTIONS}
			RESULT_VARIABLE MESON_SETUP_RESULT
			OUTPUT_VARIABLE MESON_SETUP_OUT
			ERROR_VARIABLE MESON_SETUP_ERR
		)
		if(MESON_SETUP_OUT)
			message(STATUS "${MESON_SETUP_OUT}")
		endif()
		if(MESON_SETUP_ERR)
			message(STATUS "${MESON_SETUP_ERR}")
		endif()
		if(NOT MESON_SETUP_RESULT EQUAL 0)
			message(FATAL_ERROR "Meson setup failed (exit ${MESON_SETUP_RESULT}). See meson output above for details.")
		endif()
	else()
		message(STATUS "Skipping meson setup; build files already present for VPX")
	endif()

	add_custom_command(
		OUTPUT "${VPX_LIBRARY}"
		COMMAND ${NINJA_EXECUTABLE} -C ${VPX_BUILD_DIR} install
		WORKING_DIRECTORY ${VPX_BUILD_DIR}
		COMMENT "Building bundled VPX (meson + ninja)"
		USES_TERMINAL
	)

	add_custom_target(vpx_bundled DEPENDS "${VPX_LIBRARY}")

	# Update FFmpeg options
	set(FFMPEG_MESON_OPTIONS
		${FFMPEG_MESON_OPTIONS}
		"-Dlibvpx=enabled"
		PARENT_SCOPE
	)
endif()
