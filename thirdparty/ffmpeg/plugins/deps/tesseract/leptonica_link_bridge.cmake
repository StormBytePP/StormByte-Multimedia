# Loaded via -DCMAKE_PROJECT_INCLUDE so try_compile / try_run see the same
# imported targets that LeptonicaTargets.cmake references (ZLIB::ZLIB, JPEG::JPEG, …).

if(NOT DEFINED BM_INSTALL_PREFIX OR BM_INSTALL_PREFIX STREQUAL "")
	if(DEFINED CMAKE_PREFIX_PATH AND CMAKE_PREFIX_PATH)
		list(GET CMAKE_PREFIX_PATH 0 BM_INSTALL_PREFIX)
	endif()
endif()

if(BM_INSTALL_PREFIX)
	list(APPEND CMAKE_PREFIX_PATH "${BM_INSTALL_PREFIX}")
	list(REMOVE_DUPLICATES CMAKE_PREFIX_PATH)
	set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" CACHE STRING "" FORCE)
endif()

# --- ZLIB ---
# Package exports ZLIB::ZLIBSTATIC (+ zs.lib); Leptonica asks for ZLIB::ZLIB.
find_package(ZLIB CONFIG QUIET)
if(NOT TARGET ZLIB::ZLIB AND TARGET ZLIB::ZLIBSTATIC)
	add_library(ZLIB::ZLIB ALIAS ZLIB::ZLIBSTATIC)
endif()
if(NOT TARGET ZLIB::ZLIB)
	find_package(ZLIB MODULE QUIET)
endif()
if(NOT TARGET ZLIB::ZLIB AND BM_INSTALL_PREFIX)
	foreach(_cand IN ITEMS zs.lib zlib.lib z.lib)
		set(_z "${BM_INSTALL_PREFIX}/lib/${_cand}")
		if(EXISTS "${_z}")
			add_library(ZLIB::ZLIB UNKNOWN IMPORTED)
			set_target_properties(ZLIB::ZLIB PROPERTIES
				IMPORTED_LOCATION "${_z}"
				INTERFACE_INCLUDE_DIRECTORIES "${BM_INSTALL_PREFIX}/include"
			)
			break()
		endif()
	endforeach()
endif()

# --- PNG ---
if(NOT TARGET PNG::PNG)
	find_package(PNG QUIET)
endif()
if(NOT TARGET PNG::PNG AND BM_INSTALL_PREFIX)
	foreach(_cand IN ITEMS png.lib libpng16_static.lib)
		set(_p "${BM_INSTALL_PREFIX}/lib/${_cand}")
		if(EXISTS "${_p}")
			add_library(PNG::PNG UNKNOWN IMPORTED)
			set_target_properties(PNG::PNG PROPERTIES
				IMPORTED_LOCATION "${_p}"
				INTERFACE_INCLUDE_DIRECTORIES "${BM_INSTALL_PREFIX}/include"
			)
			break()
		endif()
	endforeach()
endif()

# --- JPEG ---
if(NOT TARGET JPEG::JPEG)
	find_package(JPEG QUIET)
endif()
if(NOT TARGET JPEG::JPEG AND BM_INSTALL_PREFIX)
	foreach(_cand IN ITEMS jpeg.lib jpeg-static.lib)
		set(_j "${BM_INSTALL_PREFIX}/lib/${_cand}")
		if(EXISTS "${_j}")
			add_library(JPEG::JPEG UNKNOWN IMPORTED)
			set_target_properties(JPEG::JPEG PROPERTIES
				IMPORTED_LOCATION "${_j}"
				INTERFACE_INCLUDE_DIRECTORIES "${BM_INSTALL_PREFIX}/include"
			)
			break()
		endif()
	endforeach()
endif()

# --- TIFF ---
if(NOT TARGET TIFF::TIFF)
	find_package(TIFF QUIET)
endif()
if(NOT TARGET TIFF::TIFF AND BM_INSTALL_PREFIX)
	set(_t "${BM_INSTALL_PREFIX}/lib/tiff.lib")
	if(EXISTS "${_t}")
		add_library(TIFF::TIFF UNKNOWN IMPORTED)
		set_target_properties(TIFF::TIFF PROPERTIES
			IMPORTED_LOCATION "${_t}"
			INTERFACE_INCLUDE_DIRECTORIES "${BM_INSTALL_PREFIX}/include"
		)
	endif()
endif()
