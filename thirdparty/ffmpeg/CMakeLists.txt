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

if(WITH_FFMPEG STREQUAL "SYSTEM")
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
	find_package(FFmpeg REQUIRED)
	# If ffmpeg is not bundled, link normally with found libraries
	target_link_libraries(StormByte-Multimedia PRIVATE
		FFmpeg::avutil
		FFmpeg::avcodec
		FFmpeg::avformat
		FFmpeg::swscale
		FFmpeg::swresample
		FFmpeg::avfilter
	)
else()
	message(STATUS "Setting up bundled FFmpeg")

	set(FFMPEG_BUNDLED_TYPE "static" CACHE STRING "Build FFmpeg as static or shared library (static or shared)")

	# Add GNUInstallDirs for CMAKE_INSTALL_LIBDIR, etc
	include(GNUInstallDirs)

	# Set env variables
	if(WIN32)
		set(FFMPEG_BUNDLED_TYPE "static" CACHE STRING "Build FFmpeg as static or shared library (static or shared)" FORCE)
		set(LIB "${BUILDMASTER_INSTALL_LIBDIR}")
		set(INCLUDE "${BUILDMASTER_INSTALL_INCLUDEDIR}")
		# Use /MD or /MT depending on shared/static build
		if(CMAKE_SHARED_MODE)
			string(REPLACE "/MT" "/MD" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
			string(REPLACE "/MT" "/MD" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
		else()
			string(REPLACE "/MD" "/MT" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
			string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
		endif()
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /I${BUILDMASTER_INSTALL_INCLUDEDIR}")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /I${BUILDMASTER_INSTALL_INCLUDEDIR}")
		# Enable COMDAT folding-friendly packaging so the final link can drop unreferenced sections
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Gy")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Gy")
	else()
		set(CFLAGS "-I${BUILDMASTER_INSTALL_INCLUDEDIR}")
		set(CXXFLAGS "-I${BUILDMASTER_INSTALL_INCLUDEDIR}")
		set(LDFLAGS "-L${BUILDMASTER_INSTALL_LIBDIR}")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${BUILDMASTER_INSTALL_INCLUDEDIR}")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${BUILDMASTER_INSTALL_INCLUDEDIR}")
		# Per-function/data sections so the linker can GC unused FFmpeg objects into Multimedia
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections")
		set(CFLAGS "${CFLAGS} -ffunction-sections -fdata-sections")
		set(CXXFLAGS "${CXXFLAGS} -ffunction-sections -fdata-sections")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LDFLAGS}")
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${LDFLAGS}")
	endif()

	# Update runner with new env vars
	update_env_runner()

	# Check build mode
	if(NOT ${FFMPEG_BUNDLED_TYPE} STREQUAL "static" AND NOT ${FFMPEG_BUNDLED_TYPE} STREQUAL "shared")
		message(FATAL_ERROR "FFMPEG_BUNDLED_TYPE must be either 'static' or 'shared'")
	endif()

	# Configure FFmpeg meson options (headless library use: no CLIs, HW stacks, or host UI/device deps)
	set(FFMPEG_MESON_OPTIONS
		# --- Build surface: library-only, no tools/tests ---
		"-Dprograms=disabled"
		"-Dtests=disabled"
		"-Davdevice=disabled"
		"-Ddevices=disabled"

		# --- Text / charset (avoid pulling libiconv into static consumers) ---
		"-Diconv=disabled"

		# --- Linux audio / capture devices ---
		"-Dalsa=disabled"
		"-Dlibpulse=disabled"
		"-Dlibjack=disabled"
		"-Dlibv4l2=disabled"

		# --- Linux display / X11 / DRM ---
		"-Dxlib=disabled"
		"-Dlibxcb=disabled"
		"-Dlibdrm=disabled"

		# --- VA-API (and filters that only make sense with it) ---
		"-Dvaapi=disabled"
		"-Dvaapi_drm=disabled"
		"-Dvaapi_x11=disabled"
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

		# --- VDPAU ---
		"-Dvdpau=disabled"
		"-Dvdpau_x11=disabled"

		# --- NVIDIA / CUDA ---
		"-Dffnvcodec=disabled"
		"-Dnvenc=disabled"
		"-Dnvdec=disabled"
		"-Dcuda=disabled"
		"-Dcuvid=disabled"
		"-Dff_cuda_nvcc=disabled"

		# --- Other HW encode/decode stacks ---
		"-Dlibmfx=disabled"
		"-Dlibvpl=disabled"
		"-Drkmpp=disabled"
		"-Drockchip_mpp=disabled"
		"-Dmmal=disabled"
		"-Domx=disabled"
		"-Domx_rpi=disabled"
		"-Dmediandk=disabled"
		"-Dmediafoundation=disabled"

		# --- GPU / graphics APIs we do not use ---
		"-Dvulkan=disabled"
		"-Dopencl=disabled"
		"-Dopengl=disabled"
		"-Dsdl2=disabled"

		# --- Apple frameworks (static link would otherwise require system frameworks) ---
		"-Dappkit=disabled"
		"-Davfoundation=disabled"
		"-Dvideotoolbox=disabled"
		"-Daudiotoolbox=disabled"
		"-Dmetal=disabled"
		"-Dsecuretransport=disabled"

		# --- Optional network / ML libs (not part of our codec plugin set) ---
		"-Dlibsrt=disabled"
		"-Dlibrtmp=disabled"
		"-Dlibssh=disabled"
		"-Dlibtensorflow=disabled"

		# --- Misc platform probes we force off for a deterministic minimal build ---
		"-Dnanosleep=disabled"
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

	# Plugins (self-register; fill FFMPEG_PLUGIN_OPTIONS)
	add_subdirectory(plugins)

	message(STATUS "\tSetting up FFmpeg build")
	list_to_columns(_ffmpeg_plugin_options_columns "\t\t\t" 32 ${FFMPEG_PLUGIN_OPTIONS})
	message(STATUS "\t\tEnabled codec options:\n${_ffmpeg_plugin_options_columns}")

	# Append values added by plugins
	list(APPEND FFMPEG_MESON_OPTIONS ${FFMPEG_PLUGIN_OPTIONS})

	# Configure FFmpeg build
	set(FFMPEG_COMPONENT "ffmpeg")
	set(FFMPEG_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/src")
	ensure_build_dir(FFMPEG_BUILD_DIR)
	set(FFMPEG_OPTIONS "${FFMPEG_MESON_OPTIONS}")

	# Fetch latest data (component id must match create_meson_* id: ffmpeg)
	create_git_fetch(
		FFMPEG_GIT_FETCH_FILE
		ffmpeg
		"FFmpeg fetch"
		"${FFMPEG_SRC_DIR}"
	)
	include("${FFMPEG_GIT_FETCH_FILE}")

	# Reset changes
	create_git_reset_file(
		FFMPEG_GIT_RESET_FILE
		ffmpeg
		"FFmpeg reset"
		"${FFMPEG_SRC_DIR}"
	)
	include("${FFMPEG_GIT_RESET_FILE}")

	# Switch to branch meson-8.0.1 which is more recent than default master
	create_git_switch_branch(
		FFMPEG_GIT_SWITCH_FILE
		ffmpeg
		"FFmpeg switch to meson-8.0.1"
		"${FFMPEG_SRC_DIR}"
		"meson-8.0.1"
	)
	include("${FFMPEG_GIT_SWITCH_FILE}")

	# Local workaround until upstream MR lands (error message shows library name)
	set(FFMPEG_PATCH_EXPLICIT_MSG
		"${CMAKE_CURRENT_LIST_DIR}/fix-compressors-pkconfig-usage.patch")
	create_git_patch_file(
		FFMPEG_GIT_PATCH_FILE
		ffmpeg
		"FFmpeg explicit missing-lib error messages"
		"${FFMPEG_SRC_DIR}"
		"${FFMPEG_PATCH_EXPLICIT_MSG}"
	)
	include("${FFMPEG_GIT_PATCH_FILE}")

	set(FFMPEG_LIBRARIES "avutil;avcodec;avformat;swscale;swresample;avfilter")

	create_meson_dependant_component(
		FFMPEG_LIBRARY_CREATE_FILE
		"ffmpeg"
		"FFmpeg"
		"${FFMPEG_SRC_DIR}"
		"${FFMPEG_BUILD_DIR}"
		"${FFMPEG_OPTIONS}"
		"${FFMPEG_BUNDLED_TYPE}"
		"${FFMPEG_LIBRARIES}"
		"ffmpeg-plugins"
		1
	)
	include("${FFMPEG_LIBRARY_CREATE_FILE}")

	add_dependencies(ffmpeg_configure ffmpeg-plugins)

	if(FFMPEG_BUNDLED_TYPE STREQUAL "static" AND MSVC)
		# On Windows, libraries are named lib*.a; rename them to *.lib
		add_custom_command(TARGET ffmpeg_install POST_BUILD
			COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename "${BUILDMASTER_INSTALL_LIBDIR}/libavutil.a" "${BUILDMASTER_INSTALL_LIBDIR}/avutil.lib"
			COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename "${BUILDMASTER_INSTALL_LIBDIR}/libavcodec.a" "${BUILDMASTER_INSTALL_LIBDIR}/avcodec.lib"
			COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename "${BUILDMASTER_INSTALL_LIBDIR}/libavformat.a" "${BUILDMASTER_INSTALL_LIBDIR}/avformat.lib"
			COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename "${BUILDMASTER_INSTALL_LIBDIR}/libswscale.a" "${BUILDMASTER_INSTALL_LIBDIR}/swscale.lib"
			COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename "${BUILDMASTER_INSTALL_LIBDIR}/libswresample.a" "${BUILDMASTER_INSTALL_LIBDIR}/swresample.lib"
			COMMAND ${ENV_CMAKE_SILENT_COMMAND} -E rename "${BUILDMASTER_INSTALL_LIBDIR}/libavfilter.a" "${BUILDMASTER_INSTALL_LIBDIR}/avfilter.lib"
			COMMENT "Renaming FFmpeg libraries to .lib extensions"
		)
	endif()

	# Post-install git reset is handled automatically by BuildMaster (component id: ffmpeg)

	# Create library aliases and gather link libraries
	add_library(FFMpeg::plugins ALIAS ffmpeg-plugins)
	set(FFMPEG_LINK_LIBS "")
	foreach(_lib IN LISTS FFMPEG_LIBRARIES)
		target_include_directories(${_lib}_component SYSTEM INTERFACE ${BUILDMASTER_INSTALL_INCLUDEDIR})
		add_library(FFmpeg::${_lib} ALIAS ${_lib}_component)
		get_target_property(_FFMPEG_LIB FFmpeg::${_lib} IMPORTED_LOCATION)
		if(_FFMPEG_LIB AND NOT _FFMPEG_LIB STREQUAL "_FFMPEG_LIB-NOTFOUND")
			list(APPEND FFMPEG_LINK_LIBS "${_FFMPEG_LIB}")
		endif()
	endforeach()

	# Walk ffmpeg-plugins INTERFACE graph for real plugin .a/.lib paths.
	# Skip non-targets (generator expressions, etc.) — avoids get_target_property on "::@…".
	get_target_property(_FFMPEG_ALL_PLUGINS_INTERFACE ffmpeg-plugins INTERFACE_LINK_LIBRARIES)
	if(_FFMPEG_ALL_PLUGINS_INTERFACE AND NOT _FFMPEG_ALL_PLUGINS_INTERFACE STREQUAL "_FFMPEG_ALL_PLUGINS_INTERFACE-NOTFOUND")
		foreach(_lib IN LISTS _FFMPEG_ALL_PLUGINS_INTERFACE)
			if(NOT TARGET "${_lib}")
				continue()
			endif()
			get_target_property(_interfacelibs "${_lib}" INTERFACE_LINK_LIBRARIES)
			if(_interfacelibs AND NOT _interfacelibs STREQUAL "_interfacelibs-NOTFOUND")
				foreach(_interfacelib IN LISTS _interfacelibs)
					if(NOT TARGET "${_interfacelib}")
						continue()
					endif()
					get_target_property(_reallib "${_interfacelib}" IMPORTED_LOCATION)
					if(_reallib AND NOT _reallib STREQUAL "_reallib-NOTFOUND")
						list(APPEND FFMPEG_LINK_LIBS "${_reallib}")
					endif()
				endforeach()
			else()
				# Target may itself be an imported library
				get_target_property(_reallib "${_lib}" IMPORTED_LOCATION)
				if(_reallib AND NOT _reallib STREQUAL "_reallib-NOTFOUND")
					list(APPEND FFMPEG_LINK_LIBS "${_reallib}")
				endif()
			endif()
		endforeach()
	endif()

	# According to FFmpeg documentation, when linking statically on GNU ld,
	# -Wl,-Bsymbolic helps avoid symbol conflicts. Not valid on Apple ld64.
	if(FFMPEG_BUNDLED_TYPE STREQUAL "static" AND NOT MSVC AND NOT APPLE)
		target_link_options(StormByte-Multimedia PRIVATE -Wl,-Bsymbolic)
	endif()

	# Drop unreferenced sections from static FFmpeg/plugins when linking Multimedia
	# (requires -ffunction-sections/-fdata-sections or /Gy on the object files above).
	if(MSVC)
		target_link_options(StormByte-Multimedia PRIVATE /OPT:REF /OPT:ICF)
	elseif(APPLE)
		target_link_options(StormByte-Multimedia PRIVATE -Wl,-dead_strip)
	else()
		target_link_options(StormByte-Multimedia PRIVATE -Wl,--gc-sections)
	endif()

	if(FFMPEG_BUNDLED_TYPE STREQUAL "static" AND MSVC)
		target_link_libraries(StormByte-Multimedia PRIVATE
			ws2_32     # Windows Sockets API: socket, bind, connect, recv, send, shutdown, WSAStartup, etc.
			secur32    # Security Support Provider Interface (SSPI): TLS handshake, credentials, EncryptMessage, etc.
			bcrypt     # Windows CNG (Cryptography API: Next Generation): random generation, algorithm providers
			ncrypt     # Windows CNG key storage: persisted keys, self-signed certs, key export/import
			crypt32    # Windows CryptoAPI: certificate store, encoding, self-signing, PEM/DER conversion
		)
	endif()

	# Linking: static FFmpeg has circular deps between libs.
	# - MSVC: normal imported targets + plugins
	# - Linux (GNU ld): LINK_GROUP RESCAN (--start-group / --end-group)
	# - Apple ld64: RESCAN unsupported — link archive list as-is
	if(MSVC)
		target_link_libraries(StormByte-Multimedia PRIVATE
			FFmpeg::avutil
			FFmpeg::avcodec
			FFmpeg::avformat
			FFmpeg::swscale
			FFmpeg::swresample
			FFmpeg::avfilter
			FFMpeg::plugins
		)
	elseif(APPLE)
		target_link_libraries(StormByte-Multimedia PRIVATE
			${FFMPEG_LINK_LIBS}
		)
	else()
		target_link_libraries(StormByte-Multimedia PRIVATE
			"$<LINK_GROUP:RESCAN,${FFMPEG_LINK_LIBS}>"
		)
	endif()

	# Include directories and dependencies
	target_include_directories(StormByte-Multimedia SYSTEM PRIVATE "${BUILDMASTER_INSTALL_INCLUDEDIR}")
	add_dependencies(StormByte-Multimedia ffmpeg_install)
endif()
