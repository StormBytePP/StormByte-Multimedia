include(GNUInstallDirs)

file(GLOB_RECURSE STORMBYTE_MULTIMEDIA_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/*.cxx")

# Generate implemented backends file
set(IMPLEMENTED_BACKENDS "Implementation::FFmpeg") # Improve when more backends exist
set(SOURCE_IMPLEMENTED_BACKENDS_FILE "${CMAKE_CURRENT_LIST_DIR}/private/StormByte/multimedia/engine/implementation.cxx.in")
set(GENERATED_IMPLEMENTED_BACKENDS_FILE "${CMAKE_CURRENT_BINARY_DIR}/implementation.cxx")
configure_file(
	${SOURCE_IMPLEMENTED_BACKENDS_FILE}
	${GENERATED_IMPLEMENTED_BACKENDS_FILE}
	@ONLY
)
list(APPEND STORMBYTE_MULTIMEDIA_SOURCES "${GENERATED_IMPLEMENTED_BACKENDS_FILE}")

add_library(StormByte-Multimedia SHARED ${STORMBYTE_MULTIMEDIA_SOURCES})
add_library(StormByte::Multimedia ALIAS StormByte-Multimedia)

set_target_properties(StormByte-Multimedia PROPERTIES
	LINKER_LANGUAGE CXX
	SOVERSION ${CMAKE_PROJECT_VERSION}
	VERSION ${CMAKE_PROJECT_VERSION}
	POSITION_INDEPENDENT_CODE ON
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN ON
)

if(MSVC)
	target_compile_options(StormByte-Multimedia PRIVATE
		/EHsc
		$<$<CONFIG:Release>:/O2>
		$<$<CONFIG:Debug>:/Od>
	)
	target_compile_definitions(StormByte-Multimedia PUBLIC UNICODE NOMINMAX)
else()
	target_compile_options(StormByte-Multimedia PRIVATE
		-fvisibility=hidden
		$<$<COMPILE_LANGUAGE:CXX>:-fvisibility-inlines-hidden>
		$<$<CONFIG:Debug>:-pipe -g -ggdb -Wall -Wextra -Wnon-virtual-dtor -pedantic -pedantic-errors -O0>
	)
endif()

target_include_directories(StormByte-Multimedia
	SYSTEM BEFORE
		PUBLIC  "${CMAKE_CURRENT_LIST_DIR}/public"
		PRIVATE "${CMAKE_CURRENT_LIST_DIR}/private"
)
