get_filename_component(CMAKE_EXECUTABLE "${CMAKE_COMMAND}" NAME)
set(_env_cmake_list "${ENV_RUNNER} ${CMAKE_EXECUTABLE}")
prepare_command(ENV_CMAKE_COMMAND "${_env_cmake_list}")
set(ENV_CMAKE_COMMAND ${ENV_CMAKE_COMMAND} PARENT_SCOPE)