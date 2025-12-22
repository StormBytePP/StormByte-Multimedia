# Set initial state: assume pkg-config/pkgconf is not usable
set(PKG_CONFIG_WORKING FALSE)

# We try to find an existing pkg-config or pkgconf installation
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
	# Test if pkg-config is functional
	execute_process(
		COMMAND "${PKG_CONFIG_EXECUTABLE}" --version
		OUTPUT_VARIABLE _pkg_config_version_ignored
		RESULT_VARIABLE _pkg_config_test_result
		ERROR_VARIABLE _pkg_config_test_err_ignored
	)

	if(NOT _pkg_config_test_result EQUAL 0)
		message(WARNING "Testing system pkg-config failed, using bundled one instead")
	else()
		set(PKG_CONFIG_WORKING TRUE)
		set(PKG_CONFIG_VERSION "${PKG_CONFIG_VERSION_STRING}")
		message(STATUS "Using system pkg-config version: ${PKG_CONFIG_VERSION}")
	endif()
endif()

# Force build our pkgconf if none found or on Windows because it might have installed a broken pkgconfig (like Strawbery)
if(NOT PKG_CONFIG_WORKING)
	set(PKGCONF_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/src")
	ensure_build_dir(PKGCONF_BUILD_DIR)

	set(PKGCONF_MESON_OPTIONS "-Ddefault_library=static")

	set(PKGCONF_COMPONENT "pkgconf")
	set(PKGCONF_OPTIONS "${PKGCONF_MESON_OPTIONS}")
	set(PKGCONF_SRC_DIR "${PKGCONF_SRC_DIR}")
	create_meson_setup_file(
		PKGCONF_MESON_SETUP_FILE
		"${PKGCONF_COMPONENT}"
		"${PKGCONF_SRC_DIR}"
		"${PKGCONF_BUILD_DIR}"
		"${BOOTSTRAP_INSTALL_DIR}"
		"${PKGCONF_OPTIONS}"
	)
	include("${PKGCONF_MESON_SETUP_FILE}")

	# Build and install pkgconf
	execute_process(
		COMMAND ${ENV_NINJA_COMMAND} -C "${PKGCONF_BUILD_DIR}" install
		RESULT_VARIABLE _pkgconf_build_result
		OUTPUT_VARIABLE _pkgconf_build_out
		ERROR_VARIABLE _pkgconf_build_err
		WORKING_DIRECTORY "${PKGCONF_BUILD_DIR}"
	)
	if(NOT _pkgconf_build_result EQUAL 0)
		message(FATAL_ERROR "Building pkgconf failed:\n${_pkgconf_build_err}")
	endif()

	# Set pkgconf executable path
	set(PKG_CONFIG "${BOOTSTRAP_INSTALL_BIN_DIR}/pkgconf${CMAKE_EXECUTABLE_SUFFIX}")

	# Test pkgconf/pkg-config version
	execute_process(
		COMMAND ${ENV_RUNNER} "${PKG_CONFIG}" --version
		OUTPUT_VARIABLE _pkgconf_version
		RESULT_VARIABLE _pkgconf_test_result
		ERROR_VARIABLE _pkgconf_test_err_ignored
	)

	if(NOT _pkgconf_test_result EQUAL 0)
		message(FATAL_ERROR "Testing pkgconf failed")
	else()
		set(PKG_CONFIG_WORKING TRUE) # Not really needed, but for consistency
		string(REPLACE "\n" "" PKG_CONFIG_VERSION "${_pkgconf_version}")
		message(STATUS "Using bundled pkgconf version: ${PKG_CONFIG_VERSION}")

		# Propagate to parent scope and regenerate bootstrap env runner
		update_bootstrap_env_runner()
		set(PKG_CONFIG "${PKG_CONFIG}" PARENT_SCOPE)
	endif()
endif()