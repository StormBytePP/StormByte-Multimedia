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
	add_library(ffmpeg INTERFACE)
	target_link_libraries(ffmpeg INTERFACE
		FFmpeg::avutil
		FFmpeg::avcodec
		FFmpeg::avformat
		FFmpeg::swscale
		FFmpeg::swresample
		FFmpeg::avfilter
	)
else()
	buildmaster_message(STATUS "Setting up bundled FFmpeg" 1)

	set(FFMPEG_BUNDLED_TYPE "static" CACHE STRING "Build FFmpeg as static or shared library (static or shared)")
	if(MSVC)
		set(FFMPEG_BUNDLED_TYPE "static" CACHE STRING "Build FFmpeg as static or shared library (static or shared)" FORCE)
	endif()

	if(NOT FFMPEG_BUNDLED_TYPE STREQUAL "static" AND NOT FFMPEG_BUNDLED_TYPE STREQUAL "shared")
		buildmaster_message(FATAL "FFMPEG_BUNDLED_TYPE must be either 'static' or 'shared'")
	endif()

	set(FFMPEG_MESON_OPTIONS
		programs=disabled
		tests=disabled
		avdevice=disabled
		devices=disabled
		iconv=disabled
		alsa=disabled
		libpulse=disabled
		libjack=disabled
		libv4l2=disabled
		xlib=disabled
		libxcb=disabled
		libdrm=disabled
		vaapi=disabled
		vaapi_drm=disabled
		vaapi_x11=disabled
		deinterlace_vaapi_filter=disabled
		denoise_vaapi_filter=disabled
		overlay_vaapi_filter=disabled
		procamp_vaapi_filter=disabled
		scale_vaapi_filter=disabled
		sharpness_vaapi_filter=disabled
		tonemap_vaapi_filter=disabled
		transpose_vaapi_filter=disabled
		hstack_vaapi_filter=disabled
		vstack_vaapi_filter=disabled
		xstack_vaapi_filter=disabled
		pad_vaapi_filter=disabled
		drawbox_vaapi_filter=disabled
		vdpau=disabled
		vdpau_x11=disabled
		ffnvcodec=disabled
		nvenc=disabled
		nvdec=disabled
		cuda=disabled
		cuvid=disabled
		ff_cuda_nvcc=disabled
		libmfx=disabled
		libvpl=disabled
		rkmpp=disabled
		mmal=disabled
		omx=disabled
		omx_rpi=disabled
		mediandk=disabled
		mediafoundation=disabled
		opencl=disabled
		opengl=disabled
		sdl2=disabled
		vulkan=disabled
		libsrt=disabled
		librtmp=disabled
		libssh=disabled
		libtensorflow=disabled
		nanosleep=disabled
	)

	if(APPLE)
		list(APPEND FFMPEG_MESON_OPTIONS
			videotoolbox=enabled
			audiotoolbox=enabled
			avfoundation=disabled
			appkit=disabled
			metal=disabled
			securetransport=disabled
		)
	else()
		list(APPEND FFMPEG_MESON_OPTIONS
			appkit=disabled
			avfoundation=disabled
			videotoolbox=disabled
			audiotoolbox=disabled
			metal=disabled
			securetransport=disabled
		)
	endif()

	if(WITH_GPL)
		list(APPEND FFMPEG_MESON_OPTIONS gpl=enabled version3=enabled)
	else()
		list(APPEND FFMPEG_MESON_OPTIONS gpl=disabled version3=disabled)
	endif()

	if(WITH_NONFREE)
		list(APPEND FFMPEG_MESON_OPTIONS nonfree=enabled)
	else()
		list(APPEND FFMPEG_MESON_OPTIONS nonfree=disabled)
	endif()

	add_subdirectory(plugins)

	list(APPEND FFMPEG_MESON_OPTIONS ${FFMPEG_PLUGIN_OPTIONS})

	set(FFMPEG_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/src")

	set(FFMPEG_BM_OPTS "REQUIRE_TOOL=pkgconfig")
	if(WIN32)
		list(APPEND FFMPEG_BM_OPTS "TOOLCHAIN=msvc")
	endif()
	if(FFMPEG_BUNDLED_TYPE STREQUAL "static")
		list(APPEND FFMPEG_BM_OPTS "WHOLE")
		if(WIN32)
			list(APPEND FFMPEG_BM_OPTS "LINK={ws2_32;secur32;bcrypt;ncrypt;crypt32}")
		endif()
		if(APPLE)
			list(APPEND FFMPEG_BM_OPTS "LINK=iconv")
		endif()
	endif()
	if(CMAKE_INTERPROCEDURAL_OPTIMIZATION
			OR (CMAKE_BUILD_TYPE STREQUAL "Release"
				AND CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE))
		list(APPEND FFMPEG_BM_OPTS "IPO=fat")
	endif()
	set(FFMPEG_PATCHES
		"${CMAKE_CURRENT_LIST_DIR}/0001-meson-probe-cxx-archives-with-cpp-compiler.patch"
		"${CMAKE_CURRENT_LIST_DIR}/0002-meson-libwebp-conf-set.patch"
		"${CMAKE_CURRENT_LIST_DIR}/0003-meson-link-libm-only-when-lm-exists.patch"
	)
	list(APPEND FFMPEG_BM_OPTS
		"GIT={RESET;PATCH={${FFMPEG_PATCHES}};TITLE=Meson configure probes}"
	)
	list(JOIN FFMPEG_BM_OPTS ";" FFMPEG_BM_OPTSTR)

	buildmaster_component(
		ffmpeg
		"FFmpeg"
		"${FFMPEG_SRC_DIR}"
		"${FFMPEG_MESON_OPTIONS}"
		"${FFMPEG_BUNDLED_TYPE}"
		"avutil;avcodec;avformat;swscale;swresample;avfilter"
		"${FFMPEG_BM_OPTSTR}"
	)
	buildmaster_group_add(ff-plugins ffmpeg)
	buildmaster_link(ffmpeg ffmpeg-plugins)

	function(_stormbyte_ffmpeg_announce_build)
		buildmaster_message(STATUS "Setting up FFmpeg build" 1)
		if(NOT FFMPEG_PLUGIN_OPTIONS)
			return()
		endif()
		buildmaster_message(STATUS "Enabled codec options:" 2)
		list_to_columns(_ff_opts_fmt "\t\t\t" 36 ${FFMPEG_PLUGIN_OPTIONS})
		if(_ff_opts_fmt)
			string(REGEX REPLACE "\n$" "" _ff_opts_fmt "${_ff_opts_fmt}")
			string(REPLACE "\n" ";" _ff_opts_lines "${_ff_opts_fmt}")
			foreach(_ff_opts_line IN LISTS _ff_opts_lines)
				if(NOT _ff_opts_line STREQUAL "")
					message(STATUS "${_ff_opts_line}")
				endif()
			endforeach()
		endif()
	endfunction()
	buildmaster_hook_graph(_stormbyte_ffmpeg_announce_build zz-ffmpeg-announce
		CAPTURE FFMPEG_PLUGIN_OPTIONS
	)

	# Host DSO only. LINKFLAGS= on ffmpeg does not propagate (BM contract).
	# No gc-sections / -dead_strip / /OPT:REF — they can drop live objects.
	# -Bsymbolic: ELF. Invalid on ld64 and link.exe.
	# /FORCE:MULTIPLE: duplicate helpers (file_open) across avcodec/avfilter.
	# Frameworks: VideoToolbox/AudioToolbox + libass text stack. iconv via LINK=.
	# find_library: AppleClang + raw "-framework Foo" was flattened to a
	# bare token and the linker looked for a file named AudioToolbox.
	if(FFMPEG_BUNDLED_TYPE STREQUAL "static")
		if(WIN32)
			target_link_options(StormByte-Multimedia PRIVATE
				"/FORCE:MULTIPLE"
			)
		elseif(APPLE)
			foreach(_fw IN ITEMS
				VideoToolbox
				AudioToolbox
				CoreMedia
				CoreVideo
				CoreText
				CoreFoundation
			)
				find_library(_SB_MM_FW_${_fw} "${_fw}" REQUIRED)
				target_link_libraries(StormByte-Multimedia PRIVATE
					"${_SB_MM_FW_${_fw}}"
				)
			endforeach()
		elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
			target_link_options(StormByte-Multimedia PRIVATE
				"-Wl,-Bsymbolic"
			)
		endif()
	endif()
endif()

buildmaster_link(StormByte-Multimedia ffmpeg)
