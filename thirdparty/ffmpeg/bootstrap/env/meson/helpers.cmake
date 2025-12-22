###
## Function: create_meson_setup_file
### Generates a Meson setup CMake script for a specified component.
## Parameters:
##  - _file: name of the variable to set in parent scope with the path to
##           the generated setup script.
##  - _component: simple name of the component (e.g. "ogg", "opus").
##  - _src_dir: path to the component's source directory.
##  - _build_dir: path to the component's build directory.
##  - _meson_options: list of Meson options to pass to the component's setup.
### Behaviour:
##  - The function generates a Meson setup CMake script from the template
##    `setup.cmake.in` located in the same directory as this helper.cmake.
##  - The generated script is placed in the bootstrap CMake binary directory
##    (`${CMAKE_BINARY_DIR}`) with a name derived from the
##    component name.
##  - The generated script sets up the component's build using the provided
##    source/build directories and options.
function(create_meson_setup_file _file _component _src_dir _build_dir _install_prefix _meson_options)
	if(NOT ARGC EQUAL 6)
		message(FATAL_ERROR "create_meson_setup_file requires 6 arguments, got ${ARGC}")
	endif()
	
	set(_MESON_COMPONENT "${_component}")
	set(_MESON_BUILD_DIR "${_build_dir}")
	set(_MESON_SRC_DIR "${_src_dir}")
	list_join(_MESON_OPTIONS "${_meson_options}" " ")
	string(CONCAT
		_MESON_OPTIONS
		"--prefix=${_install_prefix} "
		"--libdir=${CMAKE_INSTALL_LIBDIR} "
		"-Dbuildtype=${CMAKE_BUILD_TYPE_LOWERCASE} "
		"${_MESON_OPTIONS}"
	)
	sanitize_for_filename(_MESON_COMPONENT_SAFE "${_MESON_COMPONENT}")
	set(_MESON_SETUP_FILE "${BOOTSTRAP_SCRIPTS_MESON_DIR}/meson_configure_${_MESON_COMPONENT_SAFE}.cmake")
	configure_file(
		"${BOOTSTRAP_SRC_DIR}/env/meson/setup.cmake.in"
		"${_MESON_SETUP_FILE}"
		@ONLY
	)
	set(${_file} "${_MESON_SETUP_FILE}" PARENT_SCOPE)
endfunction()
