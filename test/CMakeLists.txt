option(ENABLE_TEST "Enable Unit Tests" OFF)
if(ENABLE_TEST AND NOT STORMBYTE_AS_DEPENDENCY)
	enable_testing()

	add_executable(Test test.cxx)
	target_link_libraries(Test StormByte-Multimedia)
	add_test(NAME Test COMMAND Test)
endif()