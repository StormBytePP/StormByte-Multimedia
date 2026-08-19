if(MSVC)
	math(EXPR PLUGIN_LEVEL "${PLUGIN_LEVEL} + 1")
	add_subdirectory(pthread-win32)
endif()