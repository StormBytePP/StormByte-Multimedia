if(WITH_FFMPEG STREQUAL "BUNDLED")
	set(VPX_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
	set(VPX_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/meson-build")
	set(VPX_LIBRARY "${FFMPEG_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}/${CMAKE_SHARED_LIBRARY_PREFIX}vpx${CMAKE_SHARED_LIBRARY_SUFFIX}")
	file(MAKE_DIRECTORY "${VPX_BUILD_DIR}")

	set(VPX_MESON_OPTIONS
		"-Ddefault_library=shared"
		"-Dbuildtype=${CMAKE_BUILD_TYPE_LOWERCASE}"
		"-Dcpu=x86_64"
		"--prefix=${FFMPEG_INSTALL_DIR}"
		"--libdir=${CMAKE_INSTALL_LIBDIR}"
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
		run_meson("${VPX_BUILD_DIR}" "${VPX_SRC_DIR}" "${VPX_MESON_OPTIONS}")
		if(MESON_LAST_OUT)
			message(STATUS "${MESON_LAST_OUT}")
		endif()
		if(MESON_LAST_ERR)
			message(STATUS "${MESON_LAST_ERR}")
		endif()
		if(NOT MESON_LAST_RESULT EQUAL 0)
			message(FATAL_ERROR "Meson setup failed (exit ${MESON_LAST_RESULT}). See meson output above for details.")
		endif()
	else()
		message(STATUS "Skipping meson setup; build files already present for VPX")
	endif()

	add_custom_command(
		OUTPUT ${VPX_LIBRARY}
		COMMAND_EXPAND_LISTS
		COMMAND ${NINJA_RUNNER} -C "${VPX_BUILD_DIR}" install
		WORKING_DIRECTORY ${VPX_BUILD_DIR}
		COMMENT "Building bundled VPX (meson + ninja)"
		USES_TERMINAL
	)

	add_custom_target(vpx_bundled DEPENDS "${VPX_LIBRARY}")

	# Update FFmpeg options
	set(FFMPEG_OPTIONS
		"-Dlibvpx=enabled"
		PARENT_SCOPE
	)
endif()
