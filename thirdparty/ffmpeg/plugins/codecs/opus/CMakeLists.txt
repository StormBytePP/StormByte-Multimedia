message(STATUS "\t\t\tSetting up Opus codec extra data")
set(OPUS_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/src")
ensure_build_dir(OPUS_BUILD_DIR)

# Always let BuildMaster handle the download (it is cache-aware).
# The official opus_data tarball is required by upstream autogen.sh
# and produces files such as dnn/fargan_data.h.
set(OPUS_DATA_HASH "a5177ec6fb7d15058e99e57029746100121f68e4890b1467d4094aa336b6013e")
if(EXISTS "${OPUS_SRC_DIR}/autogen.sh")
	file(READ "${OPUS_SRC_DIR}/autogen.sh" _autogen_contents)
	string(REGEX MATCH "[0-9a-f]{64}" _match "${_autogen_contents}")
	if(_match)
		set(OPUS_DATA_HASH "${CMAKE_MATCH_0}")
	endif()
endif()

# Cache-aware download (destination is automatic under BUILDMASTER_DOWNLOADSDIR)
file_download_cached(OPUS_DNN_DOWNLOAD
	"https://media.xiph.org/opus/models/opus_data-${OPUS_DATA_HASH}.tar.gz"
	TITLE "Opus DNN data"
	EXPECTED_HASH "SHA256=${OPUS_DATA_HASH}"
	4
)
include(${OPUS_DNN_DOWNLOAD})

# The file is now at ${BUILDMASTER_DOWNLOADSDIR}/opus_data-${OPUS_DATA_HASH}.tar.gz
set(OPUS_MODEL_TAR "${BUILDMASTER_DOWNLOADSDIR}/opus_data-${OPUS_DATA_HASH}.tar.gz")

# Decompress into the build directory
file_decompress(OPUS_DNN_UNPACK
	"${OPUS_MODEL_TAR}"
	"${OPUS_BUILD_DIR}"
	TITLE "Opus DNN data"
	4
)
include(${OPUS_DNN_UNPACK})

# Include downloaded DNN headers
if(MSVC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /I${OPUS_BUILD_DIR}/dnn")
else()
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${OPUS_BUILD_DIR}/dnn")
endif()
string(STRIP "${CMAKE_C_FLAGS}" CMAKE_C_FLAGS)

if(FFMPEG_BUNDLED_TYPE STREQUAL "static")
	set(OPUS_SHARED_MODE OFF)
else()
	set(OPUS_SHARED_MODE ON)
endif()

set(OPUS_COMPONENT "Opus codec")
set(OPUS_OPTIONS
	"-DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}"
	"-DOPUS_BUILD_SHARED_LIBRARY=${OPUS_SHARED_MODE}"
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

# NASM flags only make sense on x86/x86_64 (ARM uses intrinsics/NEON, not NASM)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64|i[3-6]86|x86)$")
	list(APPEND OPUS_OPTIONS "-DCMAKE_ASM_NASM_FLAGS=-DPIC -w-pp-macro-params-legacy")
endif()

create_cmake_component(
	OPUS_LIBRARY_CREATE_FILE
	"opus"
	"${OPUS_COMPONENT}"
	"${OPUS_SRC_DIR}"
	"${OPUS_BUILD_DIR}"
	"${OPUS_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"opus"
	3
)
include("${OPUS_LIBRARY_CREATE_FILE}")

register_plugin(opus "-Dlibopus=enabled")