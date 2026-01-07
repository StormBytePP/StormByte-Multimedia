include(GNUInstallDirs)

# Sources
file(GLOB_RECURSE STORMBYTE_MULTIMEDIA_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/*.cxx")

# Generate implemented backends file
set(IMPLEMENTED_BACKENDS "Implementation::FFmpeg") # Improve this when there is more than one backend available
set(SOURCE_IMPLEMENTED_BACKENDS_FILE "${CMAKE_CURRENT_LIST_DIR}/private/StormByte/multimedia/engine/implementation.cxx.in")
set(GENERATED_IMPLEMENTED_BACKENDS_FILE "${CMAKE_CURRENT_BINARY_DIR}/implementation.cxx")
configure_file(
	${SOURCE_IMPLEMENTED_BACKENDS_FILE}
	${GENERATED_IMPLEMENTED_BACKENDS_FILE}
	@ONLY
)
list(APPEND STORMBYTE_MULTIMEDIA_SOURCES "${GENERATED_IMPLEMENTED_BACKENDS_FILE}")

# Library
add_library(StormByte-Multimedia SHARED ${STORMBYTE_MULTIMEDIA_SOURCES})
add_library(StormByte::Multimedia ALIAS StormByte-Multimedia)
set_target_properties(StormByte-Multimedia PROPERTIES
	LINKER_LANGUAGE CXX
	SOVERSION		${CMAKE_PROJECT_VERSION}
	VERSION 		${CMAKE_PROJECT_VERSION}
)
set_target_properties(StormByte-Multimedia PROPERTIES
	POSITION_INDEPENDENT_CODE        ON
	CXX_VISIBILITY_PRESET            hidden
	VISIBILITY_INLINES_HIDDEN        ON
)

# Compile options
if(MSVC)
	target_compile_options(StormByte-Multimedia PRIVATE /EHsc)
	target_compile_options(StormByte-Multimedia PRIVATE $<$<CONFIG:Release>:/O2>)
	target_compile_options(StormByte-Multimedia PRIVATE $<$<CONFIG:Debug>:/Od>)
	target_compile_definitions(StormByte-Multimedia PUBLIC UNICODE NOMINMAX) # Create problems if disabled
else()
	set(CMAKE_CXX_FLAGS_DEBUG "-pipe -g -ggdb -Wall -Wextra -Wnon-virtual-dtor -pedantic -pedantic-errors -O0")
	target_compile_options(StormByte-Multimedia PRIVATE -fvisibility=hidden $<$<COMPILE_LANGUAGE:CXX>:-fvisibility-inlines-hidden>)
endif()

# Include directories
target_include_directories(StormByte-Multimedia
	SYSTEM BEFORE
		PUBLIC "${CMAKE_CURRENT_LIST_DIR}/public"
		PRIVATE "${CMAKE_CURRENT_LIST_DIR}/private"
)

