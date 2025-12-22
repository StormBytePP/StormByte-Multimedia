if(WITH_FFMPEG STREQUAL "BUNDLED")
	message(STATUS "Enabling OGG bundled in FFmpeg build")
	
	include(GNUInstallDirs)

	set(OGG_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
	set(OGG_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/ogg-build")
	set(OGG_LIBRARY "${FFMPEG_EXTERNAL_PATH}/${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}ogg${CMAKE_STATIC_LIBRARY_SUFFIX}")
	file(MAKE_DIRECTORY "${OGG_BUILD_DIR}")

	add_custom_command(
		OUTPUT "${OGG_LIBRARY}"
		COMMAND ${CMAKE_COMMAND} -S ${OGG_SRC_DIR} -B ${OGG_BUILD_DIR} -G Ninja
			-DBUILD_SHARED_LIBS=OFF
			-DBUILD_TESTING=OFF
			-DINSTALL_DOCS=OFF
			-DCMAKE_INSTALL_PREFIX=${FFMPEG_EXTERNAL_PATH}
		COMMAND ${CMAKE_COMMAND} --build ${OGG_BUILD_DIR} --target install
		WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
		COMMENT "Configuring and installing ogg to ${FFMPEG_EXTERNAL_PATH}"
		VERBATIM
	)

	add_custom_target(ogg_bundled DEPENDS "${OGG_LIBRARY}")

	# Update FFmpeg options
	set(FFMPEG_MESON_OPTIONS
		${FFMPEG_MESON_OPTIONS}
		"-Doga_muxer=enabled"
		"-Dogg_muxer=enabled"
		"-Dogv_muxer=enabled"
		"-Dogg_demuxer=enabled"
		PARENT_SCOPE
	)
endif()
