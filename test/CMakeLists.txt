option(ENABLE_TEST "Enable Unit Tests" OFF)
if(ENABLE_TEST)
	enable_testing()

	add_executable(Test test.cxx)
	target_link_libraries(Test StormByte-Multimedia)
	add_test(NAME Test COMMAND Test)

	add_executable(TestWithParams test_with_params.cxx)
	target_link_libraries(TestWithParams StormByte-Multimedia)
	add_test(NAME TestWithParams COMMAND "TestWithParams")
endif()