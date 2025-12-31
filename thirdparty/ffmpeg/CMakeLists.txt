# Configure options
option(WITH_NONFREE "Enable nonfree codecs in FFmpeg build" OFF)
option(WITH_GPL "Enable GPL components in FFmpeg build" OFF)
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
	message(STATUS "Setting up bundled FFmpeg")

	# Add GNUInstallDirs for CMAKE_INSTALL_LIBDIR, etc
	include(GNUInstallDirs)

	set(FFMPEG_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/install")
	set(FFMPEG_INSTALL_LIB_DIR "${FFMPEG_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}")
	set(FFMPEG_INSTALL_BIN_DIR "${FFMPEG_INSTALL_DIR}/${CMAKE_INSTALL_BINDIR}")
	set(FFMPEG_INSTALL_INCLUDE_DIR "${FFMPEG_INSTALL_DIR}/${CMAKE_INSTALL_INCLUDEDIR}")

	file(MAKE_DIRECTORY "${FFMPEG_INSTALL_DIR}")
	file(MAKE_DIRECTORY "${FFMPEG_INSTALL_LIB_DIR}")
	file(MAKE_DIRECTORY "${FFMPEG_INSTALL_LIB_DIR}/pkgconfig")
	file(MAKE_DIRECTORY "${FFMPEG_INSTALL_BIN_DIR}")
	file(MAKE_DIRECTORY "${FFMPEG_INSTALL_INCLUDE_DIR}")

	# Set env variables
	if(WIN32)
		set(LIB "${FFMPEG_INSTALL_LIB_DIR}")
		set(INCLUDE "${FFMPEG_INSTALL_INCLUDE_DIR}")
	else()
		set(CFLAGS "-I${FFMPEG_INSTALL_INCLUDE_DIR}")
		set(CXXFLAGS "-I${FFMPEG_INSTALL_INCLUDE_DIR}")
		set(LDFLAGS "-L${FFMPEG_INSTALL_LIB_DIR}")
	endif()

	# Set PKG_CONFIG_PATH to include our internal pkgconfig files
	set(PKG_CONFIG_PATH "${FFMPEG_INSTALL_LIB_DIR}/pkgconfig")

	# Update runner with new env vars
	update_env_runner()

	# Configure FFMPEG meson build options
	set(FFMPEG_MESON_OPTIONS
		"-Ddefault_library=shared"
		# Disabled features we don't need / want
		"-Dprograms=disabled"
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
		"-Dalsa=disabled"
		"-Dlibpulse=disabled"
		"-Dlibjack=disabled"
		"-Dffnvcodec=disabled"
		"-Dnvenc=disabled"
		"-Dnvdec=disabled"
		"-Dcuda=disabled"
		"-Dcuvid=disabled"
		"-Dff_cuda_nvcc=disabled"
		"-Dlibmfx=disabled"
		"-Drkmpp=disabled"
		"-Drockchip_mpp=disabled"
		"-Dmmal=disabled"
		"-Domx=disabled"
		"-Domx_rpi=disabled"
		"-Dvaapi=disabled"
		"-Dvaapi_drm=disabled"
		"-Dvaapi_x11=disabled"
		"-Dvdpau=disabled"
		"-Dvdpau_x11=disabled"
		"-Dvulkan=disabled"
		"-Dopencl=disabled"
		"-Dopengl=disabled"
		"-Dsdl2=disabled"
		"-Dappkit=disabled"
		"-Davfoundation=disabled"
		"-Dvideotoolbox=disabled"
		"-Dmetal=disabled"
		"-Dxlib=disabled"
		"-Dlibxcb=disabled"
		"-Dlibdrm=disabled"
		"-Dlibv4l2=disabled"
		"-Dmediandk=disabled"
		"-Dmediafoundation=disabled"
		"-Dlibsrt=disabled"
		"-Dlibrtmp=disabled"
		"-Dlibssh=disabled"
		"-Dlibtensorflow=disabled"
		"-Dnanosleep=disabled"
		"-Dlibvpl=disabled"
	)

	# Enable GPL components if requested
	if(WITH_GPL)
		list(APPEND FFMPEG_MESON_OPTIONS "-Dgpl=enabled")
		list(APPEND FFMPEG_MESON_OPTIONS "-Dversion3=enabled")
	else()
		list(APPEND FFMPEG_MESON_OPTIONS "-Dgpl=disabled")
		list(APPEND FFMPEG_MESON_OPTIONS "-Dversion3=disabled")
	endif()

	# Enable nonfree codecs if requested
	if(WITH_NONFREE)
		list(APPEND FFMPEG_MESON_OPTIONS "-Dnonfree=enabled")
	else()
		list(APPEND FFMPEG_MESON_OPTIONS "-Dnonfree=disabled")
	endif()

	# Add plugins
	add_subdirectory(plugins)

	message(STATUS "\tSetting up FFmpeg build")
	# Show final meson options for FFmpeg (print list with spaces instead of ';')
	string(REPLACE ";" " " _ffmpeg_plugin_options_str "${FFMPEG_PLUGIN_OPTIONS}")
	list_to_columns(_ffmpeg_plugin_options_columns "\t\t\t" 32 ${FFMPEG_PLUGIN_OPTIONS})
	message(STATUS "\t\tEnabled codec options:\n${_ffmpeg_plugin_options_columns}")

	# Append values added by codecs
	list(APPEND FFMPEG_MESON_OPTIONS ${FFMPEG_PLUGIN_OPTIONS})

	# Configure FFmpeg build
	set(FFMPEG_COMPONENT "ffmpeg")
	set(FFMPEG_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/src")
	ensure_build_dir(FFMPEG_BUILD_DIR)
	set(FFMPEG_OPTIONS "${FFMPEG_MESON_OPTIONS}")

	# Fetch latest data
	create_git_fetch(
		FFMPEG_GIT_FETCH_FILE
		"${FFMPEG_COMPONENT}"
		"${FFMPEG_SRC_DIR}"
	)
	include("${FFMPEG_GIT_FETCH_FILE}")

	# Reset changes
	create_git_reset_file(
		FFMPEG_GIT_RESET_FILE
		"${FFMPEG_COMPONENT}"
		"${FFMPEG_SRC_DIR}"
	)
	include("${FFMPEG_GIT_RESET_FILE}")

	# Switch to branch meson-8.0.1 which is more recent than default master
	create_git_switch_branch(
		FFMPEG_GIT_SWITCH_FILE
		"${FFMPEG_COMPONENT}"
		"${FFMPEG_SRC_DIR}"
		"meson-8.0.1"
	)
	include("${FFMPEG_GIT_SWITCH_FILE}")

	# Patch source to fix libvpx detection issue
	set(FFMPEG_VPX_PATCH_FILES
		"${CMAKE_CURRENT_LIST_DIR}/0001-Fix-vpx-Add-top-level-flag-if-libvpx-is-present.patch"
		"${CMAKE_CURRENT_LIST_DIR}/0002-Fix-libvvenc-Don-t-require-vpl-probably-a-typo-bad-p.patch"
		"${CMAKE_CURRENT_LIST_DIR}/0003-Only-set-vpx-as-enabled-if-used-enabled-it-or-header.patch"
	)
	create_git_patch_file(
		FFMPEG_VPX_PATCH_REPO
		"${FFMPEG_COMPONENT}"
		"${FFMPEG_SRC_DIR}"
		"${FFMPEG_VPX_PATCH_FILES}"
	)
	include("${FFMPEG_VPX_PATCH_REPO}")

	# Creates a reset for postbuild stage
	create_git_reset_file(
		FFMPEG_GIT_RESET_FILE
		"${FFMPEG_COMPONENT}"
		"${FFMPEG_SRC_DIR}"
	)

	create_meson_setup_file(
		FFMPEG_MESON_SETUP_FILE
		"${FFMPEG_COMPONENT}"
		"${FFMPEG_SRC_DIR}"
		"${FFMPEG_BUILD_DIR}"
		"${FFMPEG_INSTALL_DIR}"
		"${FFMPEG_OPTIONS}"
	)

	set(FFMPEG_CONFIGURED "${FFMPEG_BUILD_DIR}/.configured")
	add_custom_command(
		OUTPUT ${FFMPEG_CONFIGURED}
		COMMAND ${ENV_CMAKE_COMMAND} -P ${FFMPEG_MESON_SETUP_FILE}
		COMMAND ${ENV_CMAKE_COMMAND} -E touch ${FFMPEG_CONFIGURED}
		WORKING_DIRECTORY ${FFMPEG_BUILD_DIR}
		COMMENT "Configuring FFmpeg"
		DEPENDS ffmpeg-plugins
		USES_TERMINAL
	)

	add_custom_target(_ffmpeg_build
		COMMAND_EXPAND_LISTS
		COMMAND ${ENV_NINJA_COMMAND} -C "${FFMPEG_BUILD_DIR}"
		WORKING_DIRECTORY ${FFMPEG_BUILD_DIR}
		COMMENT "Building FFmpeg"
		USES_TERMINAL
		DEPENDS ${FFMPEG_CONFIGURED}
	)

	add_custom_target(_ffmpeg_install
		COMMAND_EXPAND_LISTS
		COMMAND ${ENV_RUNNER_SILENT} ${ENV_NINJA_COMMAND} -C "${FFMPEG_BUILD_DIR}" install
		WORKING_DIRECTORY ${FFMPEG_BUILD_DIR}
		COMMENT "Installing FFmpeg internally"
		DEPENDS _ffmpeg_build
	)

	# add_custom_command(TARGET _ffmpeg_install
	# 	POST_BUILD
	# 	COMMAND ${ENV_CMAKE_COMMAND} -P ${FFMPEG_GIT_RESET_FILE}
	# 	WORKING_DIRECTORY ${FFMPEG_SRC_DIR}
	# 	COMMENT "Resetting FFmpeg source directory to clean state"
	# )

	# Set a list or artifacts that will be installed
	set(_ffmpeg_libs avutil avcodec avformat swscale swresample avfilter)
	set(_ffmpeg_outputs "")
	foreach(_lib IN LISTS _ffmpeg_libs)
		library_import_hint(_libpath "${_lib}" "${FFMPEG_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}")
		list(APPEND _ffmpeg_outputs "${_libpath}")
	endforeach()

	# This is a dummy needed so cmake knows the rules for the ffmpeg libraries
	add_custom_command(
		OUTPUT ${_ffmpeg_outputs}
		DEPENDS _ffmpeg_install
	)

	# Added so FFmpeg libraries know which files to wait for
	add_custom_target(ffmpeg DEPENDS ${_ffmpeg_outputs})

	# Expose imported targets for consumers
	set(FFMPEG_LIB_INSTALL_DIR "${FFMPEG_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}")
	set(_ffmpeg_libs avutil avcodec avformat swscale swresample avfilter)
	foreach(_lib IN LISTS _ffmpeg_libs)
		library_import_hint(_libpath "${_lib}" "${FFMPEG_LIB_INSTALL_DIR}")
		set(_tgt "FFmpeg::${_lib}")
		if(NOT TARGET ${_tgt})
			add_library(${_tgt} SHARED IMPORTED GLOBAL)
			if(WIN32)
				set_target_properties(${_tgt} PROPERTIES
					IMPORTED_IMPLIB "${_libpath}"
				)
			else()
				set_target_properties(${_tgt} PROPERTIES
					IMPORTED_LOCATION "${_libpath}"
				)
			endif()
			add_dependencies(${_tgt} ffmpeg)
			# Expose include dir and ensure linker can find transitive shared deps
			set_target_properties(${_tgt} PROPERTIES
				INTERFACE_INCLUDE_DIRECTORIES "${FFMPEG_INSTALL_INCLUDE_DIR}"
			)
		endif()
	endforeach()
endif()

cmake_policy(SET CMP0079 NEW)
target_link_libraries(StormByte-Multimedia PRIVATE FFmpeg::avutil FFmpeg::avcodec FFmpeg::avformat FFmpeg::swscale FFmpeg::swresample FFmpeg::avfilter)
