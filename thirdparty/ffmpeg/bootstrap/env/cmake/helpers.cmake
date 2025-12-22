### create_cmake_configure_file(_file _component _src_dir _build_dir _install_prefix _options)
##
### Create a CMake configure script for a third-party component.
### Parameters:
##  - _file: name of the variable to set in parent scope with the path to
##           the generated configure script.
##  - _component: simple name of the component (e.g. "ogg", "opus").
##  - _src_dir: path to the component's source directory.
##  - _build_dir: path to the component's build directory.
##  - _install_prefix: installation prefix to configure for the component
##                    (used to set CMAKE_INSTALL_PREFIX in the generated
##                     configure script).
##  - _options: list of CMake options to pass to the component's configure.
### Behaviour:
##  - The function generates a CMake configure script from the template
##    `configure.cmake.in` located in the same directory as this helper.cmake.
##  - The generated script is placed in the bootstrap CMake binary directory
##    (`${CMAKE_BINARY_DIR}`) with a name derived from the
##    component name.
##  - The generated script sets up the component's build using the provided
##    source/build directories and options.
##  - The path to the generated script is written into `_file` in the parent
##    scope.
function(create_cmake_configure_file _file _component _src_dir _build_dir _install_prefix _options)
	if(ARGC GREATER 7)
		message(FATAL_ERROR "create_cmake_configure_file requires 6 arguments or an optional verbatim flag")
	endif()
	if(ARGC EQUAL 7)
		if(ARGV6)
			set(_preserve_quotes TRUE)
		else()
			set(_preserve_quotes FALSE)
		endif()
	else()
		set(_preserve_quotes TRUE)
	endif()
	set(_CMAKE_COMPONENT "${_component}")
	set(_CMAKE_SRC_DIR "${_src_dir}")
	set(_CMAKE_BUILD_DIR "${_build_dir}")
	set(_CMAKE_INSTALL_PREFIX "${_install_prefix}")
	list_join(_CMAKE_OPTIONS "${_options}" "\n\t\t" ${_preserve_quotes})
	sanitize_for_filename(_CMAKE_COMPONENT_SAFE "${_CMAKE_COMPONENT}")
	set(_CMAKE_CONFIGURE_FILE "${BOOTSTRAP_SCRIPTS_CMAKE_DIR}/configure_${_CMAKE_COMPONENT_SAFE}.cmake")
	configure_file(
		"${BOOTSTRAP_SRC_DIR}/env/cmake/configure.cmake.in"
		"${_CMAKE_CONFIGURE_FILE}"
		@ONLY
	)
	set(${_file} "${_CMAKE_CONFIGURE_FILE}" PARENT_SCOPE)
endfunction()