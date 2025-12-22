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

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()
string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_LOWERCASE)
include(cmake/asan.cmake)

add_subdirectory(doc)
add_subdirectory(lib)
add_subdirectory(thirdparty)
add_subdirectory(test)

include(cmake/outputflags.cmake)
include(cmake/install.cmake)