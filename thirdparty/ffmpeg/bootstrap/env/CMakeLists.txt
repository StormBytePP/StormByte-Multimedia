# Add helper functions
include(${CMAKE_CURRENT_LIST_DIR}/helpers.cmake)

# Set up environment runner scripts
if(WIN32)
	# store as a list: interpreter and its first arg, then the script path
	set(ENV_RUNNER cmd "/C" "${BOOTSTRAP_SCRIPTS_ENV_DIR}/runner.bat")
	set(ENV_RUNNER_SILENT cmd "/C" "${BOOTSTRAP_SCRIPTS_ENV_DIR}/runner_silent.bat")
	# Strings for configure_file substitution (space-separated, portable)
	set(ENV_RUNNER_CMD "cmd /C ${BOOTSTRAP_SCRIPTS_ENV_DIR}/runner.bat")
	set(ENV_RUNNER_SILENT_CMD "cmd /C ${BOOTSTRAP_SCRIPTS_ENV_DIR}/runner_silent.bat")
	# Create silent runner
	configure_file(
		"runner_windows_silent.bat.in"
		"${BOOTSTRAP_SCRIPTS_ENV_DIR}/runner_silent.bat"
		@ONLY
	)
else()
	# store as a list so that when expanded in COMMAND the interpreter and
	# script become separate tokens instead of a single quoted string.
	set(ENV_RUNNER /bin/sh "${BOOTSTRAP_SCRIPTS_ENV_DIR}/runner.sh")
	set(ENV_RUNNER_SILENT /bin/sh "${BOOTSTRAP_SCRIPTS_ENV_DIR}/runner_silent.sh")
	# Strings for configure_file substitution (space-separated, portable)
	set(ENV_RUNNER_CMD "/bin/sh ${BOOTSTRAP_SCRIPTS_ENV_DIR}/runner.sh")
	set(ENV_RUNNER_SILENT_CMD "/bin/sh ${BOOTSTRAP_SCRIPTS_ENV_DIR}/runner_silent.sh")
	# Create silent runner
	configure_file(
		"runner_linux_silent.sh.in"
		"${BOOTSTRAP_SCRIPTS_ENV_DIR}/runner_silent.sh"
		@ONLY
	)
	# Ensure the generated silent runner has execute permissions so it can be
	# invoked directly by execute_process(). Some platforms require the
	# executable bit even when a shebang is present.
	execute_process(
		COMMAND ${CMAKE_COMMAND} -E chmod 0755 "${BOOTSTRAP_SCRIPTS_ENV_DIR}/runner_silent.sh"
		RESULT_VARIABLE _chmod_result
		OUTPUT_QUIET
		ERROR_QUIET
	)
endif()

# Prepare ENV_RUNNER for propagation
prepare_command(ENV_RUNNER "${ENV_RUNNER}")

# Propagate environment variables to parent scope
set(ENV_RUNNER ${ENV_RUNNER} PARENT_SCOPE)
set(ENV_RUNNER_SILENT ${ENV_RUNNER_SILENT} PARENT_SCOPE)

# Set up the rest of environment and propagate to parent
add_subdirectory(cmake)
set(ENV_CMAKE_COMMAND ${ENV_CMAKE_COMMAND} PARENT_SCOPE)

add_subdirectory(git)
set(ENV_GIT_COMMAND ${ENV_GIT_COMMAND} PARENT_SCOPE)

add_subdirectory(meson)
set(ENV_MESON_COMMAND ${ENV_MESON_COMMAND} PARENT_SCOPE)

add_subdirectory(ninja)
set(ENV_NINJA_COMMAND ${ENV_NINJA_COMMAND} PARENT_SCOPE)

# We create a basic env runner
update_bootstrap_env_runner()