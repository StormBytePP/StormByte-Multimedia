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
	# System FFmpeg: normal shared/import linkage
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

	# Helpers for MSVC operations
	if(MSVC)
		include("${CMAKE_CURRENT_LIST_DIR}/helpers.cmake")
	endif()

	# GNUInstallDirs → CMAKE_INSTALL_LIBDIR, etc. (BuildMaster install layout)
	include(GNUInstallDirs)

	# ---------------------------------------------------------------------------
	# Compiler / linker env for plugins + FFmpeg meson (prefix includes/libs)
	# ---------------------------------------------------------------------------
	if(MSVC)
		set(FFMPEG_BUNDLED_TYPE "static" CACHE STRING "Build FFmpeg as static or shared library (static or shared)" FORCE)
		set(LIB "${BUILDMASTER_INSTALL_LIBDIR}")
		set(INCLUDE "${BUILDMASTER_INSTALL_INCLUDEDIR}")
		# /MD vs /MT according to shared vs static consumer build
		if(CMAKE_SHARED_MODE)
			string(REPLACE "/MT" "/MD" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
			string(REPLACE "/MT" "/MD" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
		else()
			string(REPLACE "/MD" "/MT" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
			string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
		endif()
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /I${BUILDMASTER_INSTALL_INCLUDEDIR}")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /I${BUILDMASTER_INSTALL_INCLUDEDIR}")
	else()
		set(CFLAGS "-I${BUILDMASTER_INSTALL_INCLUDEDIR}")
		set(CXXFLAGS "-I${BUILDMASTER_INSTALL_INCLUDEDIR}")
		set(LDFLAGS "-L${BUILDMASTER_INSTALL_LIBDIR}")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${BUILDMASTER_INSTALL_INCLUDEDIR}")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${BUILDMASTER_INSTALL_INCLUDEDIR}")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LDFLAGS}")
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${LDFLAGS}")
	endif()

	update_env_runner()

	if(NOT ${FFMPEG_BUNDLED_TYPE} STREQUAL "static" AND NOT ${FFMPEG_BUNDLED_TYPE} STREQUAL "shared")
		message(FATAL_ERROR "FFMPEG_BUNDLED_TYPE must be either 'static' or 'shared'")
	endif()

	# ---------------------------------------------------------------------------
	# FFmpeg meson options (headless library: no CLIs, host UI, or unused HW)
	# ---------------------------------------------------------------------------
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
		"-Dmmal=disabled"
		"-Domx=disabled"
		"-Domx_rpi=disabled"
		"-Dmediandk=disabled"
		"-Dmediafoundation=disabled"

		# --- GPU / graphics APIs (not used) ---
		"-Dopencl=disabled"
		"-Dopengl=disabled"
		"-Dsdl2=disabled"
		"-Dvulkan=disabled"

		# --- Optional network / ML libs ---
		"-Dlibsrt=disabled"
		"-Dlibrtmp=disabled"
		"-Dlibssh=disabled"
		"-Dlibtensorflow=disabled"

		# --- Misc ---
		"-Dnanosleep=disabled"
	)

	# --- Apple frameworks (system frameworks only; no extra installs) ---
	# Useful: VideoToolbox / AudioToolbox (HW path). AppKit/Metal/SecureTransport: skip for headless lib.
	if(APPLE)
		list(APPEND FFMPEG_MESON_OPTIONS
			"-Dvideotoolbox=enabled"
			"-Daudiotoolbox=enabled"
			"-Davfoundation=disabled"   # enable later if you need capture / high-level AV APIs
			"-Dappkit=disabled"
			"-Dmetal=disabled"
			"-Dsecuretransport=disabled"
		)
	else()
		list(APPEND FFMPEG_MESON_OPTIONS
			"-Dappkit=disabled"
			"-Davfoundation=disabled"
			"-Dvideotoolbox=disabled"
			"-Daudiotoolbox=disabled"
			"-Dmetal=disabled"
			"-Dsecuretransport=disabled"
		)
	endif()

	# GPL / LGPL-3 upgrade when requested
	if(WITH_GPL)
		list(APPEND FFMPEG_MESON_OPTIONS "-Dgpl=enabled")
		list(APPEND FFMPEG_MESON_OPTIONS "-Dversion3=enabled")
	else()
		list(APPEND FFMPEG_MESON_OPTIONS "-Dgpl=disabled")
		list(APPEND FFMPEG_MESON_OPTIONS "-Dversion3=disabled")
	endif()

	# Nonfree (e.g. FDK-AAC) when requested
	if(WITH_NONFREE)
		list(APPEND FFMPEG_MESON_OPTIONS "-Dnonfree=enabled")
	else()
		list(APPEND FFMPEG_MESON_OPTIONS "-Dnonfree=disabled")
	endif()

	# Plugins self-register into FFMPEG_PLUGIN_OPTIONS (and build deps)
	add_subdirectory(plugins)

	message(STATUS "\tSetting up FFmpeg build")
	list_to_columns(_ffmpeg_plugin_options_columns "\t\t\t" 32 ${FFMPEG_PLUGIN_OPTIONS})
	message(STATUS "\t\tEnabled codec options:\n${_ffmpeg_plugin_options_columns}")

	list(APPEND FFMPEG_MESON_OPTIONS ${FFMPEG_PLUGIN_OPTIONS})

	# ---------------------------------------------------------------------------
	# FFmpeg source tree
	# Branch is tracked in .gitmodules (meson-8.1)
	# ---------------------------------------------------------------------------
	set(FFMPEG_COMPONENT "ffmpeg")
	set(FFMPEG_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/src")
	ensure_build_dir(FFMPEG_BUILD_DIR)
	set(FFMPEG_OPTIONS "${FFMPEG_MESON_OPTIONS}")
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
		add_custom_command(TARGET ffmpeg_install POST_BUILD
			COMMAND ${CMAKE_COMMAND}
				-DDIR=${BUILDMASTER_INSTALL_LIBDIR}
				-DPATTERN=libavutil.a
				-DOUT=avutil.lib
				-DMODE=copy
				-P "${FFMPEG_RENAME_MSVC_LIB}"
			COMMAND ${CMAKE_COMMAND}
				-DDIR=${BUILDMASTER_INSTALL_LIBDIR}
				-DPATTERN=libavcodec.a
				-DOUT=avcodec.lib
				-DMODE=copy
				-P "${FFMPEG_RENAME_MSVC_LIB}"
			COMMAND ${CMAKE_COMMAND}
				-DDIR=${BUILDMASTER_INSTALL_LIBDIR}
				-DPATTERN=libavformat.a
				-DOUT=avformat.lib
				-DMODE=copy
				-P "${FFMPEG_RENAME_MSVC_LIB}"
			COMMAND ${CMAKE_COMMAND}
				-DDIR=${BUILDMASTER_INSTALL_LIBDIR}
				-DPATTERN=libswscale.a
				-DOUT=swscale.lib
				-DMODE=copy
				-P "${FFMPEG_RENAME_MSVC_LIB}"
			COMMAND ${CMAKE_COMMAND}
				-DDIR=${BUILDMASTER_INSTALL_LIBDIR}
				-DPATTERN=libswresample.a
				-DOUT=swresample.lib
				-DMODE=copy
				-P "${FFMPEG_RENAME_MSVC_LIB}"
			COMMAND ${CMAKE_COMMAND}
				-DDIR=${BUILDMASTER_INSTALL_LIBDIR}
				-DPATTERN=libavfilter.a
				-DOUT=avfilter.lib
				-DMODE=copy
				-P "${FFMPEG_RENAME_MSVC_LIB}"
			COMMAND ${CMAKE_COMMAND}
				-DLIB=${BUILDMASTER_INSTALL_LIBDIR}/avutil.lib
				-DMEMBER=avutilres.rc@cus_0_avutilres.res
				-P "${FFMPEG_STRIP_MSVC_RES}"
			COMMAND ${CMAKE_COMMAND}
				-DLIB=${BUILDMASTER_INSTALL_LIBDIR}/avcodec.lib
				-DMEMBER=avcodecres.rc@cus_0_avcodecres.res
				-P "${FFMPEG_STRIP_MSVC_RES}"
			COMMAND ${CMAKE_COMMAND}
				-DLIB=${BUILDMASTER_INSTALL_LIBDIR}/avformat.lib
				-DMEMBER=avformatres.rc@cus_0_avformatres.res
				-P "${FFMPEG_STRIP_MSVC_RES}"
			COMMAND ${CMAKE_COMMAND}
				-DLIB=${BUILDMASTER_INSTALL_LIBDIR}/avfilter.lib
				-DMEMBER=avfilterres.rc@cus_0_avfilterres.res
				-P "${FFMPEG_STRIP_MSVC_RES}"
			COMMAND ${CMAKE_COMMAND}
				-DLIB=${BUILDMASTER_INSTALL_LIBDIR}/swscale.lib
				-DMEMBER=swscaleres.rc@cus_0_swscaleres.res
				-P "${FFMPEG_STRIP_MSVC_RES}"
			COMMAND ${CMAKE_COMMAND}
				-DLIB=${BUILDMASTER_INSTALL_LIBDIR}/swresample.lib
				-DMEMBER=swresampleres.rc@cus_0_swresampleres.res
				-P "${FFMPEG_STRIP_MSVC_RES}"
			COMMENT "MSVC: FFmpeg .lib names (copy) + strip version .res"
		)
	endif()

	# Post-install git reset is handled automatically by BuildMaster (component id: ffmpeg)

	# ---------------------------------------------------------------------------
	# Imported targets + path list for final link
	# ---------------------------------------------------------------------------
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
				get_target_property(_reallib "${_lib}" IMPORTED_LOCATION)
				if(_reallib AND NOT _reallib STREQUAL "_reallib-NOTFOUND")
					list(APPEND FFMPEG_LINK_LIBS "${_reallib}")
				endif()
			endif()
		endforeach()
	endif()

	# ===========================================================================
	# Linkage
	#
	# Static FFmpeg + plugins: pull full archives so codec/filter registration
	# tables (ff_lib*, ff_vf_*, ff_af_*) and weakly referenced plugin objects
	# survive the final shared link.
	#
	# Skip webpdecoder (duplicate objects with libwebp).
	# Skip shared objects (.so/.dll/.dylib): link those normally if present.
	#
	# MSVC: /FORCE:MULTIPLE for known duplicate helpers (e.g. file_open across
	# avcodec/avfilter). Does not fix LNK1241 (.res); handle resources separately.
	#
	# Apple: VideoToolbox / AudioToolbox need system frameworks when those
	# meson options are enabled (see FFMPEG_MESON_OPTIONS above).
	# ===========================================================================

	if(FFMPEG_BUNDLED_TYPE STREQUAL "static" AND NOT MSVC AND NOT APPLE)
		target_link_options(StormByte-Multimedia PRIVATE -Wl,-Bsymbolic)
	endif()

	if(FFMPEG_BUNDLED_TYPE STREQUAL "static" AND MSVC)
		target_link_libraries(StormByte-Multimedia PRIVATE
			ws2_32
			secur32
			bcrypt
			ncrypt
			crypt32
		)
	endif()

	set(_FFMPEG_STATIC_LIBS "")
	foreach(_path IN LISTS FFMPEG_LINK_LIBS)
		if(_path MATCHES "webpdecoder")
			continue()
		endif()
		# Shared loader / DLLs: not whole-archive material
		if(_path MATCHES "\\.(so|dll|dylib)(\\.[0-9]+)*$")
			continue()
		endif()
		list(APPEND _FFMPEG_STATIC_LIBS "${_path}")
	endforeach()

	if(MSVC)
		set(_FFMPEG_LINK_CMDS "")
		foreach(_path IN LISTS _FFMPEG_STATIC_LIBS)
			list(APPEND _FFMPEG_LINK_CMDS "/WHOLEARCHIVE:${_path}")
		endforeach()
		target_link_options(StormByte-Multimedia PRIVATE
			${_FFMPEG_LINK_CMDS}
			"/FORCE:MULTIPLE"
		)
		target_link_libraries(StormByte-Multimedia PRIVATE
			${_FFMPEG_STATIC_LIBS}
			FFmpeg::avutil
			FFmpeg::avcodec
			FFmpeg::avformat
			FFmpeg::swscale
			FFmpeg::swresample
			FFmpeg::avfilter
			FFMpeg::plugins
		)
	elseif(APPLE)
		set(_FFMPEG_FORCE_LOAD "")
		foreach(_path IN LISTS _FFMPEG_STATIC_LIBS)
			list(APPEND _FFMPEG_FORCE_LOAD "-Wl,-force_load,${_path}")
		endforeach()
		target_link_libraries(StormByte-Multimedia PRIVATE
			${_FFMPEG_FORCE_LOAD}
			# FFmpeg VideoToolbox / AudioToolbox (enabled on APPLE above)
			"-framework VideoToolbox"
			"-framework AudioToolbox"
			"-framework CoreMedia"
			"-framework CoreVideo"
			# libass / text stack + CF
			"-framework CoreText"
			"-framework CoreFoundation"
			# system libiconv (FFmpeg -Diconv=disabled; still used by deps e.g. libass)
			iconv
		)
	else()
		# GNU ld / lld — same idea as MSVC WHOLEARCHIVE
		target_link_libraries(StormByte-Multimedia PRIVATE
			-Wl,--whole-archive
			${_FFMPEG_STATIC_LIBS}
			-Wl,--no-whole-archive
		)
	endif()

	# ===========================================================================
	# End Linkage
	# ===========================================================================

	target_include_directories(StormByte-Multimedia SYSTEM PRIVATE "${BUILDMASTER_INSTALL_INCLUDEDIR}")
	add_dependencies(StormByte-Multimedia ffmpeg_install)
endif()
