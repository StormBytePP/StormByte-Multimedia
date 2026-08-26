# cmake -DLIB=... -DMEMBER=... -P strip_msvc_res.cmake
# Removes one archive member if present. Always exits 0 if member already gone.

if(NOT LIB OR NOT MEMBER)
	message(FATAL_ERROR "strip_msvc_res: need -DLIB= and -DMEMBER=")
endif()

if(NOT EXISTS "${LIB}")
	return()
endif()

# Prefer CMAKE_AR, with compiler-based fallbacks
if(CMAKE_AR)
	set(_LIB_TOOL "${CMAKE_AR}")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID MATCHES "Clang")
	find_program(_LIB_TOOL NAMES llvm-lib)
else()
	find_program(_LIB_TOOL NAMES lib)
endif()

if(NOT _LIB_TOOL)
	message(FATAL_ERROR "strip_msvc_res: could not find lib/llvm-lib")
endif()

# LIST
execute_process(
	COMMAND "${_LIB_TOOL}" /NOLOGO /LIST "${LIB}"
	OUTPUT_VARIABLE _list
	ERROR_VARIABLE _err
	RESULT_VARIABLE _rc
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(NOT _rc EQUAL 0)
	return()
endif()

string(FIND "${_list}" "${MEMBER}" _pos)
if(_pos EQUAL -1)
	return()
endif()

# REMOVE
execute_process(
	COMMAND "${_LIB_TOOL}" /NOLOGO "/REMOVE:${MEMBER}" "${LIB}"
	RESULT_VARIABLE _rc2
	OUTPUT_VARIABLE _out2
	ERROR_VARIABLE _err2
)
