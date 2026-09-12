# vim: ts=2 sw=2
# - Try to find libvmaf (Netflix VMAF).
#
# Once done this will define
#  VMAF_FOUND         - System has libvmaf
#  VMAF_INCLUDE_DIRS  - Directory that contains libvmaf/libvmaf.h
#  VMAF_LIBRARIES     - Link these to use libvmaf
#  VMAF_DEFINITIONS   - Extra CFLAGS from pkg-config
#  VMAF_VERSION       - Version string when pkg-config knows it
#
# Imported target:
#  VMAF::vmaf

include(FindPackageHandleStandardArgs)

if(NOT WIN32)
	find_package(PkgConfig)
	if(PKG_CONFIG_FOUND)
		pkg_check_modules(PC_VMAF libvmaf)
	endif()
endif()

find_path(VMAF_INCLUDE_DIRS
	NAMES
		libvmaf/libvmaf.h
		libvmaf.h
	HINTS
		${PC_VMAF_INCLUDEDIR}
		${PC_VMAF_INCLUDE_DIRS}
	PATH_SUFFIXES
		libvmaf
)

find_library(VMAF_LIBRARIES
	NAMES vmaf libvmaf
	HINTS
		${PC_VMAF_LIBDIR}
		${PC_VMAF_LIBRARY_DIRS}
)

set(VMAF_DEFINITIONS ${PC_VMAF_CFLAGS_OTHER} CACHE STRING "The VMAF CFLAGS.")
if(PC_VMAF_VERSION)
	set(VMAF_VERSION ${PC_VMAF_VERSION} CACHE STRING "The VMAF version number.")
endif()

mark_as_advanced(VMAF_INCLUDE_DIRS VMAF_LIBRARIES VMAF_DEFINITIONS VMAF_VERSION)

find_package_handle_standard_args(Vmaf
	REQUIRED_VARS VMAF_LIBRARIES VMAF_INCLUDE_DIRS
	VERSION_VAR VMAF_VERSION
)

if(VMAF_FOUND AND NOT TARGET VMAF::vmaf)
	list(GET VMAF_LIBRARIES 0 _vmaf_lib)
	add_library(VMAF::vmaf UNKNOWN IMPORTED)
	set_target_properties(VMAF::vmaf PROPERTIES
		IMPORTED_LOCATION "${_vmaf_lib}"
		INTERFACE_INCLUDE_DIRECTORIES "${VMAF_INCLUDE_DIRS}"
	)
	if(VMAF_DEFINITIONS)
		set_property(TARGET VMAF::vmaf PROPERTY
			INTERFACE_COMPILE_OPTIONS "${VMAF_DEFINITIONS}")
	endif()
endif()
