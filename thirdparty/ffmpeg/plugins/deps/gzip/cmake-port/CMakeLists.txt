cmake_minimum_required(VERSION 3.20)
project(gzip_tool C)

include(GNUInstallDirs)

add_executable(gzip gzip_main.c)

find_package(ZLIB REQUIRED)
target_include_directories(gzip PRIVATE ${ZLIB_INCLUDE_DIRS})
target_link_libraries(gzip PRIVATE ZLIB::ZLIB)

if(MSVC)
	target_compile_definitions(gzip PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

install(TARGETS gzip RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
