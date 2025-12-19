if(WITH_FFMPEG STREQUAL "BUNDLED")
	find_program(MESON_EXECUTABLE meson)
	find_program(NINJA_EXECUTABLE ninja)
	if(NOT MESON_EXECUTABLE)
		message(FATAL_ERROR "Meson not found: required to build bundled Opus. Install 'meson'.")
	endif()
	if(NOT NINJA_EXECUTABLE)
		message(FATAL_ERROR "Ninja not found: required to build bundled Opus. Install 'ninja'.")
	endif()

	set(X264_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
	set(X264_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/meson-build")
	set(X264_LIBRARY "${FFMPEG_EXTERNAL_PATH}/${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}x264${CMAKE_STATIC_LIBRARY_SUFFIX}")
	file(MAKE_DIRECTORY "${X264_BUILD_DIR}")

	set(X264_MESON_OPTIONS
		"-Ddefault_library=static"
		"-Dbuildtype=release"
		"--prefix=${FFMPEG_EXTERNAL_PATH}"
		"-Dbashcompletion=false"
		"-Dcli=false"
		"-Dopencl=false"
		"-Dbit-depth=all"
	)

	if (NOT EXISTS "${X264_BUILD_DIR}/build.ninja")
		message(STATUS "Configuring bundled X264 with Meson")
		execute_process(
			COMMAND ${MESON_EXECUTABLE} setup ${X264_BUILD_DIR} ${X264_SRC_DIR} ${X264_MESON_OPTIONS}
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
		OUTPUT ${X264_LIBRARY}
		COMMAND ${NINJA_EXECUTABLE} -C ${X264_BUILD_DIR} install
		WORKING_DIRECTORY ${X264_BUILD_DIR}
		COMMENT "Building bundled X264 (meson + ninja)"
		USES_TERMINAL
	)

	add_custom_target(x264_bundled DEPENDS "${X264_LIBRARY}")

	# Update FFmpeg options
	set(FFMPEG_MESON_OPTIONS
		${FFMPEG_MESON_OPTIONS}
		"-Dlibx264=enabled"
		"-Dlibx264_encoder=enabled"
		"-Dlibx264rgb_encoder=enabled"
		PARENT_SCOPE
	)
endif()
