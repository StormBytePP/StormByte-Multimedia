if(WITH_FFMPEG STREQUAL "BUNDLED")
	find_program(MESON_EXECUTABLE meson)
	find_program(NINJA_EXECUTABLE ninja)
	if(NOT MESON_EXECUTABLE)
		message(FATAL_ERROR "Meson not found: required to build bundled Opus. Install 'meson'.")
	endif()
	if(NOT NINJA_EXECUTABLE)
		message(FATAL_ERROR "Ninja not found: required to build bundled Opus. Install 'ninja'.")
	endif()

	set(OPUS_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
	set(OPUS_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/meson-build")
	set(OPUS_LIBRARY "${FFMPEG_EXTERNAL_PATH}/${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}opus${CMAKE_STATIC_LIBRARY_SUFFIX}")
	file(MAKE_DIRECTORY "${OPUS_BUILD_DIR}")

	# If generated DNN data files are missing, try to download the official
	# opus_data tarball used by upstream autogen.sh. This creates files such
	# as dnn/fargan_data.h which Meson expects.
	if(NOT EXISTS "${OPUS_SRC_DIR}/dnn/fargan_data.h")
		# Try to extract expected hash from autogen.sh, fallback to known hash
		set(OPUS_DATA_HASH "a5177ec6fb7d15058e99e57029746100121f68e4890b1467d4094aa336b6013e")
		if(EXISTS "${OPUS_SRC_DIR}/autogen.sh")
			file(READ "${OPUS_SRC_DIR}/autogen.sh" _autogen_contents)
			string(REGEX MATCH "[0-9a-f]{64}" _match "${_autogen_contents}")
			if(_match)
				set(OPUS_DATA_HASH "${CMAKE_MATCH_0}")
			endif()
		endif()

		message(STATUS "Opus DNN data not found; attempting download with hash ${OPUS_DATA_HASH}")
		# Use CMake's file(DOWNLOAD) for portability instead of shell scripts.
		set(OPUS_MODEL_TAR "${OPUS_SRC_DIR}/dnn/opus_data-${OPUS_DATA_HASH}.tar.gz")
		if(NOT EXISTS "${OPUS_SRC_DIR}/dnn")
			file(MAKE_DIRECTORY "${OPUS_SRC_DIR}/dnn")
		endif()
		if(NOT EXISTS "${OPUS_MODEL_TAR}")
			file(DOWNLOAD
				"https://media.xiph.org/opus/models/opus_data-${OPUS_DATA_HASH}.tar.gz"
				"${OPUS_MODEL_TAR}"
				SHOW_PROGRESS
				EXPECTED_HASH SHA256=${OPUS_DATA_HASH}
				STATUS _opus_dl_status
				LOG _opus_dl_log
			)
			list(GET _opus_dl_status 0 _opus_dl_code)
			if(NOT _opus_dl_code EQUAL 0)
				message(WARNING "Failed to download Opus data: ${_opus_dl_log}")
			endif()
		endif()
		if(EXISTS "${OPUS_MODEL_TAR}")
			execute_process(
				COMMAND ${CMAKE_COMMAND} -E tar xzf "${OPUS_MODEL_TAR}"
				WORKING_DIRECTORY "${OPUS_SRC_DIR}"
				RESULT_VARIABLE _opus_tar_res
				OUTPUT_VARIABLE _opus_tar_out
				ERROR_VARIABLE _opus_tar_err
			)
			if(NOT _opus_tar_res EQUAL 0)
				message(WARNING "Failed to extract ${OPUS_MODEL_TAR}: ${_opus_tar_err}")
			endif()
		else()
			message(WARNING "Opus DNN model archive not available; Meson may fail if generated DNN files are required.")
		endif()
	endif()

	set(OPUS_MESON_OPTIONS
		"--prefix=${FFMPEG_EXTERNAL_PATH}"
		"-Ddefault_library=static"
		"-Dbuildtype=release"
		"-Dtests=disabled"
		"-Ddocs=disabled"
		"-Dextra-programs=disabled"
		"-Db_pch=false"
		"-Dcustom-modes=true"
		"-Dassertions=false"
		"-Ddeep-plc=disabled"
		"-Ddred=disabled"
		"-Dosce=disabled"
		"-Dintrinsics=enabled"
	)

	if (NOT EXISTS "${OPUS_BUILD_DIR}/build.ninja")
		message(STATUS "Configuring bundled Opus with Meson")
		execute_process(
			COMMAND ${MESON_EXECUTABLE} setup ${OPUS_BUILD_DIR} ${OPUS_SRC_DIR} ${OPUS_MESON_OPTIONS}
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
		message(STATUS "Skipping meson setup; build files already present for Opus")
	endif()

	add_custom_command(
		OUTPUT "${OPUS_LIBRARY}"
		COMMAND ${NINJA_EXECUTABLE} -C ${OPUS_BUILD_DIR} install
		WORKING_DIRECTORY ${OPUS_BUILD_DIR}
		COMMENT "Building bundled Opus (meson + ninja)"
		USES_TERMINAL
	)

	add_custom_target(opus_bundled DEPENDS "${OPUS_LIBRARY}")

	# Update FFmpeg options
	set(FFMPEG_MESON_OPTIONS
		${FFMPEG_MESON_OPTIONS}
		"-Dlibopus=enabled"
		PARENT_SCOPE
	)
endif()
