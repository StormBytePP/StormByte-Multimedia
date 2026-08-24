# cmake -DLIB=... -DMEMBER=... -P strip_msvc_res.cmake
# Removes one archive member if present. Always exits 0 if member already gone.

if(NOT LIB OR NOT MEMBER)
	message(FATAL_ERROR "strip_msvc_res: need -DLIB= and -DMEMBER=")
endif()

if(NOT EXISTS "${LIB}")
	message(STATUS "strip_msvc_res: skip (missing ${LIB})")
	return()
endif()

# Is the member present?
execute_process(
	COMMAND lib /NOLOGO /LIST "${LIB}"
	OUTPUT_VARIABLE _list
	ERROR_VARIABLE _err
	RESULT_VARIABLE _rc
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(NOT _rc EQUAL 0)
	message(STATUS "strip_msvc_res: lib /LIST failed on ${LIB} (${_rc})")
	return()
endif()

string(FIND "${_list}" "${MEMBER}" _pos)
if(_pos EQUAL -1)
	message(STATUS "strip_msvc_res: ${MEMBER} not in ${LIB} (ok)")
	return()
endif()

execute_process(
	COMMAND lib /NOLOGO "/REMOVE:${MEMBER}" "${LIB}"
	RESULT_VARIABLE _rc2
	OUTPUT_VARIABLE _out2
	ERROR_VARIABLE _err2
)
if(NOT _rc2 EQUAL 0)
	message(WARNING "strip_msvc_res: /REMOVE failed (${_rc2}): ${_err2}")
else()
	message(STATUS "strip_msvc_res: removed ${MEMBER} from ${LIB}")
endif()