###
# Helper function to update the bootstrap env runner script
# based on the current global properties.
###
function(update_bootstrap_env_runner)
	if(WIN32)
		configure_file(
			"${BOOTSTRAP_SRC_DIR}/env/runner_windows.bat.in"
			"${BOOTSTRAP_SCRIPTS_ENV_DIR}/runner.bat"
			@ONLY
		)
	else()
		configure_file(
			"${BOOTSTRAP_SRC_DIR}/env/runner_linux.sh.in"
			"${BOOTSTRAP_SCRIPTS_ENV_DIR}/runner.sh"
			@ONLY
		)

		# Ensure the generated runner has execute permissions so it can be
		# invoked directly by execute_process(). Some platforms require the
		# executable bit even when a shebang is present.
		execute_process(
			COMMAND ${CMAKE_COMMAND} -E chmod 0755 "${BOOTSTRAP_SCRIPTS_ENV_DIR}/runner.sh"
			RESULT_VARIABLE _chmod_result
			OUTPUT_QUIET
			ERROR_QUIET
		)
	endif()
endfunction()

## prepare_command(_out _command_list)
##
## Prepare a serialized command list for use with `execute_process` or
## similar APIs by expanding CMake's internal list form into a space-
## separated argument list and splitting it into tokens appropriate for
## the current platform.
##
## Parameters:
##  - _out: name of the variable to set in the parent scope with the
##          resulting tokenized command (as a CMake list).
##  - _command_list: a CMake list (or a variable name containing a list)
##                   representing the command and its arguments. The
##                   function will join list elements with spaces and
##                   then call `separate_arguments` to produce a list of
##                   tokens suitable for passing to `execute_process`.
##
## Behavior:
##  - Replaces CMake's semicolon separators with spaces to form a single
##    string, then uses `separate_arguments` with the platform-appropriate
##    mode (`WINDOWS_COMMAND` or `UNIX_COMMAND`) to produce a tokenized
##    command suitable for execution.
##  - Returns the tokenized command list in `_out` (parent scope).
function(prepare_command _out _command_list)
	if(NOT ARGC EQUAL 2)
		message(FATAL_ERROR "prepare_command requires out variable and command list")
	endif()

	string(REPLACE ";" " " _command_list_spaces "${_command_list}")
	if(WIN32)
		separate_arguments(_separated_command_list WINDOWS_COMMAND "${_command_list_spaces}")
	else()
		separate_arguments(_separated_command_list UNIX_COMMAND "${_command_list_spaces}")
	endif()
	# Return the tokenized command as a proper CMake list so callers can
	# pass it directly to `execute_process(COMMAND ...)` as multiple args.
	set(${_out} ${_separated_command_list} PARENT_SCOPE)
endfunction()