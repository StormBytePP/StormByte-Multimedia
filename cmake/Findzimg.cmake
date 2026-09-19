include(FindPackageHandleStandardArgs)

if(NOT WIN32)
	find_package(PkgConfig)
	if(PKG_CONFIG_FOUND)
		pkg_check_modules(PC_ZIMG zimg)
	endif()
endif()

find_path(ZIMG_INCLUDE_DIRS
	NAMES zimg.h
	HINTS ${PC_ZIMG_INCLUDEDIR} ${PC_ZIMG_INCLUDE_DIRS}
)

find_library(ZIMG_LIBRARIES
	NAMES zimg
	HINTS ${PC_ZIMG_LIBDIR} ${PC_ZIMG_LIBRARY_DIRS}
)

if(PC_ZIMG_VERSION)
	set(ZIMG_VERSION ${PC_ZIMG_VERSION} CACHE STRING "zimg version")
endif()

mark_as_advanced(ZIMG_INCLUDE_DIRS ZIMG_LIBRARIES ZIMG_VERSION)

find_package_handle_standard_args(zimg
	REQUIRED_VARS ZIMG_LIBRARIES ZIMG_INCLUDE_DIRS
	VERSION_VAR ZIMG_VERSION
)

if(zimg_FOUND AND NOT TARGET zimg::zimg)
	list(GET ZIMG_LIBRARIES 0 _zimg_lib)
	add_library(zimg::zimg UNKNOWN IMPORTED)
	set_target_properties(zimg::zimg PROPERTIES
		IMPORTED_LOCATION "${_zimg_lib}"
		INTERFACE_INCLUDE_DIRECTORIES "${ZIMG_INCLUDE_DIRS}"
	)
endif()
