if(WITH_FFMPEG STREQUAL "BUNDLED")
	find_program(MESON_EXECUTABLE meson)
	find_program(NINJA_EXECUTABLE ninja)

	find_program(PYTHON3_EXECUTABLE python3)
	if(PYTHON3_EXECUTABLE AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../ninja-propagate.py")
		set(NINJA_RUNNER ${PYTHON3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/../ninja-propagate.py" ${NINJA_EXECUTABLE})
	else()
		set(NINJA_RUNNER ${NINJA_EXECUTABLE})
	endif()
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
		"-Dbuildtype=${CMAKE_BUILD_TYPE_LOWERCASE}"
		"--prefix=${FFMPEG_EXTERNAL_PATH}"
		"--libdir=${CMAKE_INSTALL_LIBDIR}"
		"-Dbashcompletion=false"
		"-Dcli=false"
		"-Dopencl=false"
		"-Dbit-depth=all"
	)

	if (NOT EXISTS "${X264_BUILD_DIR}/build.ninja")
		include(${CMAKE_SOURCE_DIR}/cmake/meson.cmake)
		message(STATUS "Configuring bundled X264 with Meson")
		# Use helper run_meson to ensure correct env on Windows
		run_meson("${X264_BUILD_DIR}" "${X264_SRC_DIR}" "${X264_MESON_OPTIONS}")
		# run_meson exports MESON_LAST_RESULT/MESON_LAST_OUT/MESON_LAST_ERR
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
		message(STATUS "Skipping meson setup; build files already present for x264")
	endif()

	add_custom_command(
		OUTPUT ${X264_LIBRARY}
		COMMAND ${NINJA_RUNNER} -C ${X264_BUILD_DIR} install
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
