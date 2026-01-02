# Configure options
option(WITH_NONFREE "Enable nonfree codecs in FFmpeg build" OFF)
option(WITH_GPL "Enable GPL components in FFmpeg build" OFF)
set(FFMPEG_BUNDLED "static" CACHE STRING "Build FFmpeg as static or shared library (static or shared)")
if(NOT DEFINED WITH_FFMPEG)
	if(WIN32)
		set(WITH_FFMPEG "BUNDLED" CACHE STRING "Use FFmpeg libraries (SYSTEM or BUNDLED)" FORCE)
		set(FFMPEG_BUNDLED "static" CACHE STRING "Build FFmpeg as static or shared library (static or shared)" FORCE)
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

	# Configure build mode
	if(FFMPEG_BUNDLED STREQUAL "static")
		set(CMAKE_SHARED_MODE OFF)
		set(MESON_SHARED_MODE "static")
	else()
		set(CMAKE_SHARED_MODE ON)
		set(MESON_SHARED_MODE "shared")
	endif()

	# Configure compile flags
	if(MSVC)
		# Use /MD or /MT depending on shared/static build
		if(CMAKE_SHARED_MODE)
			string(REPLACE "/MT" "/MD" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
			string(REPLACE "/MT" "/MD" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
		else()
			string(REPLACE "/MD" "/MT" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
			string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
		endif()
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /I${FFMPEG_INSTALL_INCLUDE_DIR}")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /I${FFMPEG_INSTALL_INCLUDE_DIR}")
	else()
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${FFMPEG_INSTALL_INCLUDE_DIR}")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${FFMPEG_INSTALL_INCLUDE_DIR}")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LDFLAGS}")
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${LDFLAGS}")
	endif()

	# Configure FFMPEG meson build options
	set(FFMPEG_MESON_OPTIONS
		"-Ddefault_library=${MESON_SHARED_MODE}"
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
		"-Davdevice=disabled"
		'-Dzlib=disabled'
		'-Dbzlib=disabled'
		'-Dlzma=disabled'
	)
	if(FFMPEG_BUNDLED STREQUAL "static")
		list(APPEND FFMPEG_MESON_OPTIONS "-Db_staticpic=true")
	endif()

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
		COMMAND ${ENV_CMAKE_SILENT_COMMAND} -P ${FFMPEG_MESON_SETUP_FILE}
		COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E touch ${FFMPEG_CONFIGURED}
		WORKING_DIRECTORY ${FFMPEG_BUILD_DIR}
		COMMENT "Configuring FFmpeg"
		DEPENDS ffmpeg-plugins-install
	)

	add_custom_target(ffmpeg_build
		COMMAND_EXPAND_LISTS
		COMMAND ${ENV_NINJA_COMMAND} -C "${FFMPEG_BUILD_DIR}"
		WORKING_DIRECTORY ${FFMPEG_BUILD_DIR}
		COMMENT "Building FFmpeg"
		DEPENDS ${FFMPEG_CONFIGURED}
		USES_TERMINAL
	)

	add_custom_target(ffmpeg_install
		COMMAND_EXPAND_LISTS
		COMMAND ${ENV_NINJA_SILENT_COMMAND} -C "${FFMPEG_BUILD_DIR}" install
		WORKING_DIRECTORY ${FFMPEG_BUILD_DIR}
		COMMENT "Installing FFmpeg internally"
		DEPENDS ffmpeg_build
	)

	if(FFMPEG_BUNDLED STREQUAL "static" AND MSVC)
		# On Windows, libraries are named lib*.a; rename them to *.lib
		add_custom_command(TARGET ffmpeg_install POST_BUILD
			COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename "${FFMPEG_INSTALL_LIB_DIR}/libavutil.a" "${FFMPEG_INSTALL_LIB_DIR}/avutil.lib"
			COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename "${FFMPEG_INSTALL_LIB_DIR}/libavcodec.a" "${FFMPEG_INSTALL_LIB_DIR}/avcodec.lib"
			COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename "${FFMPEG_INSTALL_LIB_DIR}/libavformat.a" "${FFMPEG_INSTALL_LIB_DIR}/avformat.lib"
			COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename "${FFMPEG_INSTALL_LIB_DIR}/libswscale.a" "${FFMPEG_INSTALL_LIB_DIR}/swscale.lib"
			COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename "${FFMPEG_INSTALL_LIB_DIR}/libswresample.a" "${FFMPEG_INSTALL_LIB_DIR}/swresample.lib"
			COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename "${FFMPEG_INSTALL_LIB_DIR}/libavfilter.a" "${FFMPEG_INSTALL_LIB_DIR}/avfilter.lib"
			COMMENT "Renaming FFmpeg libraries to .lib extensions"
		)
	endif()

	# Set a list or artifacts that will be installed
	set(_ffmpeg_libs avutil avcodec avformat swscale swresample avfilter)
	set(_ffmpeg_outputs "")

	# First pass: compute the expected output library paths
	foreach(_lib IN LISTS _ffmpeg_libs)
		if(FFMPEG_BUNDLED STREQUAL "static")
			library_import_static_hint(_libpath "${_lib}" "${FFMPEG_INSTALL_LIB_DIR}")
		else()
			library_import_hint(_libpath "${_lib}" "${FFMPEG_INSTALL_LIB_DIR}")
		endif()
		list(APPEND _ffmpeg_outputs "${_libpath}")
	endforeach()

	# Rules so cmake knows how to build the artifacts: declare the custom
	# command that produces the library files, then create the `ffmpeg`
	# helper target depending on those files so other targets can depend on it.
	add_custom_command(OUTPUT ${_ffmpeg_outputs} DEPENDS ffmpeg_install)
	add_custom_target(ffmpeg DEPENDS ${_ffmpeg_outputs} ffmpeg-plugins-install)

	# Second pass: create the imported targets that reference the generated files
	foreach(_lib IN LISTS _ffmpeg_libs)
		set(_tgt "FFmpeg::${_lib}")
		if (FFMPEG_BUNDLED STREQUAL "static")
			library_import_static_hint(_libpath "${_lib}" "${FFMPEG_INSTALL_LIB_DIR}")
			add_library(${_tgt} STATIC IMPORTED GLOBAL)
		else()
			library_import_hint(_libpath "${_lib}" "${FFMPEG_INSTALL_LIB_DIR}")
			add_library(${_tgt} SHARED IMPORTED GLOBAL)
		endif()
		if(MSVC)
			if(FFMPEG_BUNDLED STREQUAL "static")
				set_target_properties(${_tgt} PROPERTIES
					IMPORTED_LOCATION "${_libpath}"
					IMPORTED_LOCATION_DEBUG "${_libpath}"
					IMPORTED_LOCATION_RELEASE "${_libpath}"
					IMPORTED_LOCATION_MINSIZEREL "${_libpath}"
					IMPORTED_LOCATION_RELWITHDEBINFO "${_libpath}"
				)
				target_link_libraries(${_tgt}
					INTERFACE
						crypt32
						ws2_32
						bcrypt
						secur32
						ncrypt
				)
			else()
				set_target_properties(${_tgt} PROPERTIES
					IMPORTED_IMPLIB "${_libpath}"
					IMPORTED_IMPLIB_DEBUG "${_libpath}"
					IMPORTED_IMPLIB_RELEASE "${_libpath}"
					IMPORTED_IMPLIB_MINSIZEREL "${_libpath}"
					IMPORTED_IMPLIB_RELWITHDEBINFO "${_libpath}"
				)
			endif()
		else()
			set_target_properties(${_tgt} PROPERTIES
				IMPORTED_LOCATION "${_libpath}"
				IMPORTED_LOCATION_DEBUG "${_libpath}"
				IMPORTED_LOCATION_RELEASE "${_libpath}"
				IMPORTED_LOCATION_MINSIZEREL "${_libpath}"
				IMPORTED_LOCATION_RELWITHDEBINFO "${_libpath}"
			)
		endif()
		add_dependencies(${_tgt} ffmpeg)
		# Ensure consumers also link the plugin target transitively
		target_link_libraries(${_tgt} INTERFACE ffmpeg-plugins)
		set_target_properties(${_tgt} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${FFMPEG_INSTALL_INCLUDE_DIR}")
	endforeach()

	cmake_policy(SET CMP0079 NEW)
	# Ensure the main target builds FFmpeg and plugin installation first so
	# the imported library files exist when the linker runs. Some build
	# generators (ninja) may attempt to link the main target before the
	# plugin install step is executed; adding explicit dependencies ensures
	# the correct ordering.
	add_dependencies(StormByte-Multimedia ffmpeg)

	# According to FFMmpeg documentation, when linking statically,
	# -Wl,-Bsymbolic should be used to avoid symbol conflicts.
	if(FFMPEG_BUNDLED STREQUAL "static" AND NOT MSVC)
		target_link_options(StormByte-Multimedia PRIVATE -Wl,-Bsymbolic)
	endif()
	
endif()

cmake_policy(SET CMP0079 NEW)
# ASAN for GCC needs the explicit linkage of FFmpeg libraries even if they are bundled into StormByte-Multimedia
if(ENABLE_ASAN)
	target_link_libraries(StormByte-Multimedia PUBLIC FFmpeg::avutil FFmpeg::avcodec FFmpeg::avformat FFmpeg::swscale FFmpeg::swresample FFmpeg::avfilter)
else()
	target_link_libraries(StormByte-Multimedia PRIVATE FFmpeg::avutil FFmpeg::avcodec FFmpeg::avformat FFmpeg::swscale FFmpeg::swresample FFmpeg::avfilter)
endif()
