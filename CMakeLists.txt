cmake_minimum_required(VERSION 3.12)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project("StormByte C++ Library: Multimedia module"
	VERSION 0.0.9999
	DESCRIPTION "StormByte-Multimedia is a StormByte library module for parsing configuration files"
	HOMEPAGE_URL "https://dev.stormbyte.org/StormByte-Multimedia"
)

enable_testing()
add_subdirectory(thirdparty)
add_subdirectory(doc)
add_subdirectory(lib)
add_subdirectory(test)
