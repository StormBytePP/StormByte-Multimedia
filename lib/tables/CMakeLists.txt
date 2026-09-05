file(GLOB_RECURSE STORMBYTE_MULTIMEDIA_TABLE_SOURCES CONFIGURE_DEPENDS
	"${CMAKE_CURRENT_LIST_DIR}/*.cxx"
)

set(_tables_unity "${CMAKE_CURRENT_BINARY_DIR}/tables_unity.cxx")
set(_tables_unity_body "")
foreach(_src IN LISTS STORMBYTE_MULTIMEDIA_TABLE_SOURCES)
	file(RELATIVE_PATH _rel "${CMAKE_CURRENT_LIST_DIR}" "${_src}")
	string(REGEX REPLACE "[^A-Za-z0-9]" "_" _id "${_rel}")
	string(APPEND _tables_unity_body
		"#define table table_${_id}\n#include \"${_src}\"\n#undef table\n")
endforeach()
file(WRITE "${_tables_unity}.in" "${_tables_unity_body}")
configure_file("${_tables_unity}.in" "${_tables_unity}" COPYONLY)

target_sources(StormByte-Multimedia PRIVATE "${_tables_unity}")

target_include_directories(StormByte-Multimedia
	SYSTEM BEFORE PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/.."
)
