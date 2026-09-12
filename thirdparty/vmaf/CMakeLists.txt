set(VMAF_GIT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(VMAF_WINDOWS_PATH_FILE "${CMAKE_CURRENT_LIST_DIR}/0001-MSVC-corrections.patch")

set(VMAF_COMPONENT "VMAF library")
set(VMAF_OPTIONS
	"-Denable_tests=false"
	"-Denable_docs=false"
	"-Denable_tools=false"
	"-Denable_asm=true"
	"-Dbuilt_in_models=true"
	"-Denable_cuda=false"
	"-Denable_nvtx=false"
	"-Denable_nvcc=false"
)
if(WIN32)
	list(APPEND VMAF_OPTIONS "-Dbundled_winpthreads=false")
endif()
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64)$")
	list(APPEND VMAF_OPTIONS "-Denable_avx512=true")
else()
	list(APPEND VMAF_OPTIONS "-Denable_avx512=false")
endif()

buildmaster_component(
	vmaf
	"${VMAF_COMPONENT}"
	"${VMAF_GIT_DIR}"
	"${VMAF_OPTIONS}"
	"static"
	"vmaf"
	"SOURCE=libvmaf;GIT={RESET;PATCH=${VMAF_WINDOWS_PATH_FILE};TITLE=VMAF MSVC}"
)
if(MSVC)
	buildmaster_link(vmaf pthread-win32)
endif()

