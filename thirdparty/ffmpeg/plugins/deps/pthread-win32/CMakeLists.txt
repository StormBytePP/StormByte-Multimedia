set(PTHREAD_WIN32_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
ensure_build_dir(PTHREAD_WIN32_BUILD_DIR)
set(PTHREAD_WIN32_COMPONENT "pthread Win32 library")
set(PTHREAD_WIN32_OPTIONS "")

# Canonical name for DSL / pkg-config / VMAF (no debug suffix)
if(FFMPEG_BUNDLED_TYPE STREQUAL "static")
	set(PTHREAD_WIN32_LIBS "libpthreadVC3")
else()
	set(PTHREAD_WIN32_LIBS "pthreadVC3")
endif()

create_cmake_component(
	PTHREAD_WIN32_LIBRARY_CREATE_FILE
	"pthread-win32"
	"${PTHREAD_WIN32_COMPONENT}"
	"${PTHREAD_WIN32_SRC_DIR}"
	"${PTHREAD_WIN32_BUILD_DIR}"
	"${PTHREAD_WIN32_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"${PTHREAD_WIN32_LIBS}"
	${PLUGIN_LEVEL}
)
include("${PTHREAD_WIN32_LIBRARY_CREATE_FILE}")

if(MSVC AND FFMPEG_BUNDLED_TYPE STREQUAL "static")
	# Debug: libpthreadVC3d.lib → also provide libpthreadVC3.lib (keep original for safety)
	# Release: already libpthreadVC3.lib
	add_custom_command(TARGET pthread-win32_install POST_BUILD
		COMMAND ${CMAKE_COMMAND}
			-DDIR=${BUILDMASTER_INSTALL_LIBDIR}
			-DPATTERN=libpthreadVC3*.lib
			-DOUT=libpthreadVC3.lib
			-DMODE=copy
			-P "${FFMPEG_RENAME_MSVC_LIB}"
		COMMENT "Normalize libpthreadVC3.lib (Debug/Release)"
	)
endif()

register_dependency(pthread-win32)