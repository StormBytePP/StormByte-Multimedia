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
## Prepare a tokenized command suitable for passing to
## `execute_process(COMMAND ...)` by converting CMake's internal list
## representation into a platform-appropriate sequence of arguments.
##
## Parameters:
##  - _out: name of the variable to set in the parent scope. The value
##          will be a CMake list where each element is a single token
##          (i.e. an argument) suitable for expanding directly in
##          `execute_process(COMMAND ${_out} ...)`.
##  - _command_list: a CMake list (or the name of a variable containing
##                   a list) representing the command and its
##                   arguments. For example: `/bin/sh;${SCRIPT}` or
##                   `cmd;/C;${SCRIPT}`.
##
## Behavior and notes:
##  - Joins the CMake list elements with spaces and then calls
##    `separate_arguments` using `WINDOWS_COMMAND` or `UNIX_COMMAND`
##    depending on the platform. This handles quoting and token
##    splitting so the caller does not need to perform manual parsing.
##  - The returned `_out` is a proper CMake list of tokens; callers
##    should expand it as multiple arguments in `execute_process`, not
##    as a single quoted string.
##  - The function requires exactly two arguments; it will `FATAL_ERROR`
##    if called incorrectly.
##
## Example:
## ```cmake
## set(_cmd /bin/sh "${BOOTSTRAP_SCRIPTS_ENV_DIR}/runner.sh")
## prepare_command(ENV_RUNNER "${_cmd}")
## execute_process(COMMAND ${ENV_RUNNER} --version WORKING_DIRECTORY ${WD})
## ```
## This produces a token list like `/bin/sh` and `/path/to/runner.sh`
## so `execute_process` receives them as two separate arguments.
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