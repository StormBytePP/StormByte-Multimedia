# rename_msvc_lib.cmake
# cmake -DDIR=... -DPATTERN=... -DOUT=... -DMODE=copy|rename -P rename_msvc_lib.cmake

if(NOT DIR OR NOT PATTERN OR NOT OUT)
	message(FATAL_ERROR "rename_msvc_lib: need -DDIR= -DPATTERN= -DOUT=")
endif()

if(NOT MODE)
	set(MODE "copy")
endif()

if(NOT MODE STREQUAL "copy" AND NOT MODE STREQUAL "rename")
	message(FATAL_ERROR "rename_msvc_lib: MODE must be copy or rename (got ${MODE})")
endif()

if(NOT EXISTS "${DIR}")
	message(FATAL_ERROR "rename_msvc_lib: DIR does not exist: ${DIR}")
endif()

get_filename_component(_out_bn "${OUT}" NAME)
set(_out_path "${DIR}/${_out_bn}")

# Already canonical and non-empty → done (typical Release)
if(EXISTS "${_out_path}")
	file(SIZE "${_out_path}" _out_sz)
	if(_out_sz GREATER 0)
		message(STATUS "rename_msvc_lib: ${_out_path} already OK (${_out_sz} bytes)")
		return()
	endif()
	# 0-byte stub → remove and continue
	file(REMOVE "${_out_path}")
endif()

file(GLOB _cands "${DIR}/${PATTERN}")
set(_src "")

foreach(f IN LISTS _cands)
	if(f MATCHES "\\.pdb$")
		continue()
	endif()
	get_filename_component(_bn "${f}" NAME)
	if(_bn STREQUAL _out_bn)
		continue()
	endif()
	file(SIZE "${f}" _sz)
	if(_sz EQUAL 0)
		continue()
	endif()
	set(_src "${f}")
	break()
endforeach()

if(_src STREQUAL "")
	message(FATAL_ERROR "rename_msvc_lib: no non-empty match for '${PATTERN}' in ${DIR}")
endif()

if(MODE STREQUAL "copy")
	execute_process(
		COMMAND "${CMAKE_COMMAND}" -E copy "${_src}" "${_out_path}"
		RESULT_VARIABLE _rc
	)
	if(NOT _rc EQUAL 0)
		message(FATAL_ERROR "rename_msvc_lib: copy failed ${_src} → ${_out_path}")
	endif()
	message(STATUS "MSVC copy: ${_src} → ${_out_path} (original kept)")
else()
	file(RENAME "${_src}" "${_out_path}")
	message(STATUS "MSVC rename: ${_src} → ${_out_path}")
endif()
