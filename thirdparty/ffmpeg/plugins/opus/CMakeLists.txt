set(OPUS_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/src")
ensure_build_dir(OPUS_BUILD_DIR)
library_import_hint(OPUS_LIBRARY "opus" "${FFMPEG_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}")

# If generated DNN data files are missing, try to download the official
# opus_data tarball used by upstream autogen.sh. This creates files such
# as dnn/fargan_data.h which Meson expects.
if(NOT EXISTS "${OPUS_BUILD_DIR}/dnn/fargan_data.h")
	# Try to extract expected hash from autogen.sh, fallback to known hash
	set(OPUS_DATA_HASH "a5177ec6fb7d15058e99e57029746100121f68e4890b1467d4094aa336b6013e")
	if(EXISTS "${OPUS_SRC_DIR}/autogen.sh")
		file(READ "${OPUS_SRC_DIR}/autogen.sh" _autogen_contents)
		string(REGEX MATCH "[0-9a-f]{64}" _match "${_autogen_contents}")
		if(_match)
			set(OPUS_DATA_HASH "${CMAKE_MATCH_0}")
		endif()
	endif()

	message(STATUS "Downloading Opus DNN data with hash ${OPUS_DATA_HASH}")
	file(MAKE_DIRECTORY "${OPUS_BUILD_DIR}/tmp")
	set(OPUS_MODEL_TAR "${OPUS_BUILD_DIR}/tmp/opus_data-${OPUS_DATA_HASH}.tar.gz")
	if(NOT EXISTS "${OPUS_BUILD_DIR}/dnn")
		file(MAKE_DIRECTORY "${OPUS_BUILD_DIR}/dnn")
	endif()
	if(NOT EXISTS "${OPUS_MODEL_TAR}")
		file(DOWNLOAD
			"https://media.xiph.org/opus/models/opus_data-${OPUS_DATA_HASH}.tar.gz"
			"${OPUS_MODEL_TAR}"
			#SHOW_PROGRESS
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
			COMMAND ${ENV_CMAKE_COMMAND} -E tar xzf "${OPUS_MODEL_TAR}"
			WORKING_DIRECTORY "${OPUS_BUILD_DIR}"
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

# Setup CFLAGS and CXXFLAGS to include build dir to use downloaded models
if(MSVC)
  	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /I${OPUS_BUILD_DIR}/dnn")
else()
  	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${OPUS_BUILD_DIR}/dnn")
endif()

# Ensure no leading/trailing whitespace that would break command-line -D parsing
string(STRIP "${CMAKE_C_FLAGS}" CMAKE_C_FLAGS)

set(OPUS_COMPONENT "Opus codec")
set(OPUS_OPTIONS
	"-DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}"
	"-DOPUS_BUILD_SHARED_LIBRARY=ON"
	"-DOPUS_BUILD_TESTING=OFF"
	"-DOPUS_CUSTOM_MODES=ON"
	"-DOPUS_BUILD_PROGRAMS=OFF"
	"-DOPUS_FUZZING=ON"
	"-DOPUS_CHECK_ASM=ON"
	"-DOPUS_INSTALL_PKG_CONFIG_MODULE=ON"
	"-DOPUS_INSTALL_CMAKE_CONFIG_MODULE=OFF"
	"-DOPUS_DRED=ON"
	"-DOPUS_OSCE=ON"
)
create_cmake_configure_file(
	OPUS_CMAKE_CONFIGURE_FILE
	"${OPUS_COMPONENT}"
	"${OPUS_SRC_DIR}"
	"${OPUS_BUILD_DIR}"
	"${FFMPEG_PLUGINS_INSTALL_DIR}"
	"${OPUS_OPTIONS}"
)
include("${OPUS_CMAKE_CONFIGURE_FILE}")

add_custom_target(_opus_build
	COMMAND ${ENV_CMAKE_COMMAND} --build "${OPUS_BUILD_DIR}"
	WORKING_DIRECTORY "${OPUS_BUILD_DIR}"
	COMMENT "Building Opus codec"
	USES_TERMINAL
)
add_custom_target(_opus_install
	COMMAND ${ENV_RUNNER_SILENT} ${ENV_CMAKE_COMMAND} --build "${OPUS_BUILD_DIR}" --target install
	WORKING_DIRECTORY "${OPUS_BUILD_DIR}"
	COMMENT "Installing Opus codec internally"
	DEPENDS _opus_build
)
add_custom_target(opus DEPENDS _opus_install)
