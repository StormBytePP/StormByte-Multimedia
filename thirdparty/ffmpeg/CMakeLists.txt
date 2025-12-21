if(NOT DEFINED WITH_FFMPEG)
	if(WIN32)
		set(WITH_FFMPEG "BUNDLED" CACHE STRING "Use FFmpeg libraries (SYSTEM or BUNDLED)" FORCE)
	else()
		set(WITH_FFMPEG "BUNDLED" CACHE STRING "Use FFmpeg libraries (SYSTEM or BUNDLED)")
	endif()
endif()

if (WITH_FFMPEG STREQUAL "SYSTEM")
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
	find_package(FFmpeg REQUIRED)
else()
	# Configure FFMPEG meson build options
	set(FFMPEG_MESON_OPTIONS
		"-Ddefault_library=static"
		"-Dbuildtype=release"
		"-Dprograms=enabled"
		"-Dtests=disabled"
		"-Ddevices=disabled"
		"-Dvaapi=disabled"
		"-Dvaapi_drm=disabled"
		"-Ddeinterlace_vaapi_filter=disabled"
		"-Ddenoise_vaapi_filter=disabled"
		"-Doverlay_vaapi_filter=disabled"
		"-Dprocamp_vaapi_filter=disabled"
		"-Dscale_vaapi_filter=disabled"
		"-Dsharpness_vaapi_filter=disabled"
		"-Dtonemap_vaapi_filter=disabled"
		"-Dtranspose_vaapi_filter=disabled"
		"-Dhstack_vaapi_filter=disabled"
		"-Dvstack_vaapi_filter=disabled"
		"-Dxstack_vaapi_filter=disabled"
		"-Dpad_vaapi_filter=disabled"
		"-Ddrawbox_vaapi_filter=disabled"
		"-Dvaapi_x11=disabled"
		"-Dgpl=enabled"
		"-Dversion3=enabled"
	)

	# Create a path to gather all external dependencies
	set(FFMPEG_EXTERNAL_PATH "${CMAKE_CURRENT_BINARY_DIR}/external")
	file(MAKE_DIRECTORY "${FFMPEG_EXTERNAL_PATH}")
	# file(MAKE_DIRECTORY "${FFMPEG_EXTERNAL_PATH}/include")
	# file(MAKE_DIRECTORY "${FFMPEG_EXTERNAL_PATH}/lib")

	# Create pkgconfig path
	set(PKGCONFIG_PATH "${FFMPEG_EXTERNAL_PATH}/${CMAKE_INSTALL_LIBDIR}/pkgconfig")

	# Include dependencies
	add_subdirectory(containers) # Containers are needed before codecs
	add_subdirectory(codecs)

	# Build FFmpeg from the readonly `src/` tree using Meson + Ninja and
	# expose static IMPORTED targets in the `FFmpeg::` namespace.
	find_program(MESON_EXECUTABLE meson)
	find_program(NINJA_EXECUTABLE ninja)
	if (NOT MESON_EXECUTABLE)
		message(FATAL_ERROR "Meson not found: required to build bundled FFmpeg. Install 'meson'.")
	endif()
	if (NOT NINJA_EXECUTABLE)
		message(FATAL_ERROR "Ninja not found: required to build bundled FFmpeg. Install 'ninja'.")
	endif()

	set(FFMPEG_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
	set(FFMPEG_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/meson-build")
	file(MAKE_DIRECTORY "${FFMPEG_BUILD_DIR}")

	# Meson configure at build time; set PKG_CONFIG_PATH to staged pkgconfig dir
	set(_meson_setup_out "${FFMPEG_BUILD_DIR}/build.ninja")
	# Export pkg-config path and staged include/lib dirs so Meson's compile/link
	# probes can find bundled codec headers and libraries (e.g. x264).
	# Use a CMake list so each assignment is passed as a separate arg to
	# `cmake -E env` (avoids concatenating into PKG_CONFIG_PATH).
	set(_meson_env
		"PKG_CONFIG_PATH=${PKGCONFIG_PATH}"
		"CPPFLAGS=-I${FFMPEG_EXTERNAL_PATH}/include"
		"CFLAGS=-I${FFMPEG_EXTERNAL_PATH}/include"
		"CXXFLAGS=-I${FFMPEG_EXTERNAL_PATH}/include"
		"LDFLAGS=-L${FFMPEG_EXTERNAL_PATH}/${CMAKE_INSTALL_LIBDIR}"
	)

	# Ensure Meson configure re-runs when codec staging targets are updated.
	# Collect codec bundled targets we want to be ready before Meson runs.
	set(_ffmpeg_codec_bundles "opus_bundled;vorbis_bundled;ogg_bundled;x264_bundled;x265_bundled")
	if(WITH_NONFREE)
		list(APPEND _ffmpeg_codec_bundles fdk-aac_bundled)
	endif()

	add_custom_command(
		OUTPUT ${_meson_setup_out}
		COMMAND ${CMAKE_COMMAND} -E env ${_meson_env} ${MESON_EXECUTABLE} setup ${FFMPEG_BUILD_DIR} ${FFMPEG_SRC_DIR} ${FFMPEG_MESON_OPTIONS}
		WORKING_DIRECTORY ${FFMPEG_SRC_DIR}
		COMMENT "Configuring bundled FFmpeg with Meson"
		USES_TERMINAL
		DEPENDS ${_ffmpeg_codec_bundles}
	)

	add_custom_target(meson_configure_ffmpeg DEPENDS ${_meson_setup_out})

	# # Add codec and container dependencies
	# if(WITH_NONFREE)
	# 	add_dependencies(meson_configure_ffmpeg fdk-aac_bundled)
	# endif()
	# add_dependencies(meson_configure_ffmpeg opus_bundled)
	# add_dependencies(meson_configure_ffmpeg vorbis_bundled)
	# add_dependencies(meson_configure_ffmpeg ogg_bundled)
	# add_dependencies(meson_configure_ffmpeg x264_bundled)
	# #add_dependencies(meson_configure_ffmpeg x265_bundled)
	# add_dependencies(meson_configure_ffmpeg vpx_bundled)

	# Ninja build target for FFmpeg
	set(FFMPEG_LIB_DIR "${FFMPEG_BUILD_DIR}")
	set(_ffmpeg_libs avutil avcodec avformat swscale swresample avfilter)
	set(_ffmpeg_outputs "")
	foreach(_lib IN LISTS _ffmpeg_libs)
		list(APPEND _ffmpeg_outputs "${FFMPEG_LIB_DIR}/lib${_lib}.a")
	endforeach()

	add_custom_command(
		OUTPUT ${_ffmpeg_outputs}
		COMMAND ${NINJA_EXECUTABLE} -C ${FFMPEG_BUILD_DIR}
		WORKING_DIRECTORY ${FFMPEG_BUILD_DIR}
		COMMENT "Building bundled FFmpeg (meson + ninja)"
		USES_TERMINAL
		DEPENDS ${_meson_setup_out}
		USES_TERMINAL
	)

	add_custom_target(ffmpeg_bundled DEPENDS ${_ffmpeg_outputs})
	add_dependencies(ffmpeg_bundled meson_configure_ffmpeg)

	message(STATUS "MESONOPTIONS: ${FFMPEG_MESON_OPTIONS}")

	# Expose imported targets for consumers
	foreach(_lib IN LISTS _ffmpeg_libs)
		set(_libpath "${FFMPEG_LIB_DIR}/lib${_lib}.a")
		set(_tgt "FFmpeg::${_lib}")
		if(NOT TARGET ${_tgt})
			add_library(${_tgt} STATIC IMPORTED GLOBAL)
			set_target_properties(${_tgt} PROPERTIES IMPORTED_LOCATION "${_libpath}")
			add_dependencies(${_tgt} ffmpeg_bundled)
			set_target_properties(${_tgt} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${FFMPEG_SRC_DIR}")
		endif()
	endforeach()
endif()

cmake_policy(SET CMP0079 NEW)
target_link_libraries(StormByte-Multimedia PRIVATE FFmpeg::avutil FFmpeg::avcodec FFmpeg::avformat FFmpeg::swscale FFmpeg::swresample FFmpeg::avfilter)