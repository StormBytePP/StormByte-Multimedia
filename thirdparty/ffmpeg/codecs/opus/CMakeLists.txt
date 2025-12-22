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

	add_custom_command(
		OUTPUT "${OPUS_LIBRARY}"
		COMMAND ${CMAKE_COMMAND} -S ${OPUS_SRC_DIR} -B ${OPUS_BUILD_DIR} -G Ninja
			-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
			-DOPUS_BUILD_SHARED_LIBRARY=OFF
			-DOPUS_BUILD_TESTING=OFF
			-DOPUS_CUSTOM_MODES=ON
			-DOPUS_BUILD_PROGRAMS=OFF
			-DOPUS_FUZZING=ON
			-DOPUS_CHECK_ASM=ON
			-DOPUS_INSTALL_PKG_CONFIG_MODULE=ON
			-DOPUS_INSTALL_CMAKE_CONFIG_MODULE=OFF
			-DOPUS_DRED=ON
			-DOPUS_OSCE=ON
			-DCMAKE_INSTALL_PREFIX=${FFMPEG_EXTERNAL_PATH}
		COMMAND ${CMAKE_COMMAND} --build ${OPUS_BUILD_DIR} --target install
		WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
		COMMENT "Configuring and installing opus to ${FFMPEG_EXTERNAL_PATH}"
		VERBATIM
	)

	add_custom_target(opus_bundled DEPENDS "${OPUS_LIBRARY}")

	# Update FFmpeg options
	set(FFMPEG_MESON_OPTIONS
		${FFMPEG_MESON_OPTIONS}
		"-Dlibopus=enabled"
		PARENT_SCOPE
	)
endif()
