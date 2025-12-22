## windows_path(out_var, input_path)
##
## Normalize a path for Windows workspaces.
## Parameters:
##  - out_var: name of the variable to set (in parent scope) with the
##             normalized path.
##  - input_path: path to normalize. On WIN32 this will convert forward
##                slashes to backslashes and remove surrounding quotes.
##
## Behaviour:
##  - On WIN32: strips surrounding double quotes (if present) and replaces
##    '/' with '\\' to produce a basic Windows-style path.
##  - On non-WIN32 platforms: returns the original path unchanged.
function(windows_path _out_path _input_path )
	if(NOT ARGC EQUAL 2)
		message(FATAL_ERROR "windows_path requires output variable name and input path")
	endif()

	if(WIN32)
		# Minimal normalization: strip surrounding quotes and replace '/' with '\\'
		set(_p "${_input_path}")
		string(REGEX REPLACE "^\"(.*)\"$" "\\1" _p "${_p}")
		string(REPLACE "/" "\\" _out "${_p}")
		set(${_out_path} "${_out}" PARENT_SCOPE)
	else()
		set(${_out_path} "${_input_path}" PARENT_SCOPE)
	endif()
endfunction()

## library_import_hint(out_var, lib_name, prefix_path)
##
## Construct a shared-library filename hint for `find_library`/importing.
## Parameters:
##  - out_var: variable name to set in parent scope with the constructed name.
##  - lib_name: base library name without prefix/suffix (e.g. 'avcodec').
##  - prefix_path: optional directory prefix to prepend before the library
##                 filename (no trailing separator expected).
##
## Behaviour:
##  - On WIN32 uses `CMAKE_IMPORT_LIBRARY_PREFIX`/`_SUFFIX` and backslash
##    as separator.
##  - On other platforms uses shared library prefix/suffix and '/' as
##    separator.
##  - If `prefix_path` is provided it is prepended using the platform
##    separator followed by the library prefix/suffix and name.
function(library_import_hint _out_var _lib_name _prefix_path)
	if(NOT ARGC EQUAL 3)
		message(FATAL_ERROR "library_import_hint requires output variable name, library name and prefix.")
	endif()

	if (WIN32)
		set(_prefix "${CMAKE_IMPORT_LIBRARY_PREFIX}")
		set(_suffix "${CMAKE_IMPORT_LIBRARY_SUFFIX}")
	else()
		set(_prefix "${CMAKE_SHARED_LIBRARY_PREFIX}")
		set(_suffix "${CMAKE_SHARED_LIBRARY_SUFFIX}")
	endif()

	if(NOT _prefix_path STREQUAL "")
		set(_prefix "${_prefix_path}/${_prefix}")
	endif()
	
	set(${_out_var} "${_prefix}${_lib_name}${_suffix}" PARENT_SCOPE)
endfunction()

## library_import_static_hint(out_var, lib_name, prefix_path)
##
## Construct a static-library filename hint for importing/linking.
## Parameters:
##  - out_var: variable name to set in parent scope with the constructed name.
##  - lib_name: base library name without prefix/suffix.
##  - prefix_path: optional directory prefix to prepend before the library
##                 filename (no trailing separator expected).
##
## Behaviour:
##  - Uses `CMAKE_STATIC_LIBRARY_PREFIX` and `CMAKE_STATIC_LIBRARY_SUFFIX`.
##  - Prepends `prefix_path` followed by the appropriate path separator if
##    provided.
function(library_import_static_hint _out_var _lib_name _prefix_path)
	if(NOT ARGC EQUAL 3)
		message(FATAL_ERROR "library_import_static_hint requires output variable name, library name and prefix.")
	endif()

	set(_separator "/")

	if(NOT _prefix_path STREQUAL "")
		set(_prefix "${_prefix_path}${_separator}${CMAKE_STATIC_LIBRARY_PREFIX}")
	else()
		set(_prefix "${CMAKE_STATIC_LIBRARY_PREFIX}")
	endif()
	
	set(${_out_var} "${_prefix}${_lib_name}${CMAKE_STATIC_LIBRARY_SUFFIX}" PARENT_SCOPE)
endfunction()

## sanitize_for_filename(_out _input)
##
## Produce a filesystem-safe string from an arbitrary input suitable for
## use in filenames. The transformation is intentionally conservative to
## avoid surprising characters in generated file names used by the
## bootstrap helpers.
##
## Parameters:
##  - _out: name of the variable to set in the parent scope with the
##          sanitized string.
##  - _input: input string to sanitize.
##
## Behavior:
##  - Replaces any character not in the set [A-Za-z0-9._-] with '_'.
##  - Collapses consecutive underscores to a single underscore.
##  - Trims leading and trailing underscores.
##
function(sanitize_for_filename _out _input)
	if(NOT ARGC EQUAL 2)
		message(FATAL_ERROR "sanitize_for_filename requires output variable name and input string")
	endif()

	# Sanitize component name for safe filenames:
	# - replace any character not in [A-Za-z0-9._-] with '_'
	# - collapse repeated underscores
	# - trim leading/trailing underscores
	string(REGEX REPLACE "[^A-Za-z0-9._-]" "_" _output "${_input}")
	string(REGEX REPLACE "_+" "_" _output "${_output}")
	string(REGEX REPLACE "^_+|_+$" "" _output "${_output}")

	set(${_out} "${_output}" PARENT_SCOPE)
endfunction()

## toggle_bool(var_name)
##
## Toggle a variable between TRUE and FALSE in the caller (parent) scope.
## - var_name: name of the variable to toggle. The function reads the
##   current value and writes the negated value into the parent scope.
function(toggle_bool _var)
	if(NOT ARGC EQUAL 1)
		message(FATAL_ERROR "toggle_bool requires one variable name")
	endif()

	if(${${_var}})
		set(${_var} FALSE PARENT_SCOPE)
	else()
		set(${_var} TRUE PARENT_SCOPE)
	endif()
endfunction()

## list_join(_out_var _list_var _separator [preserve_quotes])
##
## Join a CMake list into a single string using a provided separator while
## preserving semicolons that appear inside single- or double-quoted
## substrings. Optionally keeps the quote characters themselves in the
## output.
##
## Parameters:
##  - _out_var: name of the variable to set in the parent scope with the
##              resulting joined string.
##  - _list_var: name of a variable that contains a CMake list. The
##               function reads the variable's raw serialization (which
##               uses ';' as an element separator) and operates on that
##               string. Pass the variable name, not a literal list.
##  - _separator: string used to replace top-level semicolons (those not
##                inside quotes). Typically a space or a single character.
##  - preserve_quotes: optional boolean (TRUE/FALSE, default TRUE). If
##                     TRUE the function will keep single and double quote
##                     characters in the output; if FALSE quotes are
##                     removed during processing.
##
## Behavior:
##  - Serializes the list into CMake's internal form and iterates the
##    characters one-by-one while tracking whether it is currently inside
##    single or double quotes.
##  - Semicolons encountered when not inside any quotes are replaced with
##    the provided `_separator` value; semicolons inside quotes are left
##    unchanged and escaped so CMake does not treat them as separators.
##  - Quote characters are preserved in the output unless
##    `preserve_quotes` is FALSE.
##  - The final joined string is stored in `_out_var` in the parent scope.
##
## Notes and examples:
##  - This function is intended as a safe alternative to `list(JOIN ...)`
##    when list elements may contain quoted semicolons that must be
##    preserved.
##  - Example usage:
##      set(mylist "a" "param=\"x;y\"" "b")
##      list_join(joined mylist ",")          # quotes preserved by default
##      list_join(joined mylist "," FALSE)    # remove quotes from output
##  - The function does not validate matching quotes; unbalanced quotes
##    may produce unexpected output.
##
function(list_join _out_var _raw_string _separator)
	if(ARGC GREATER 4)
		message(FATAL_ERROR "list_join requires output variable name, raw string and separator (optional a boolean to preserve quotes)")
	endif()

	set(result "\"")
	set(in_single_quote FALSE)
	set(in_double_quote FALSE)

	set(raw "${_raw_string}")

	if("${raw}" STREQUAL "")
		message(FATAL_ERROR "list_join: input string is empty")
	endif()

	string(LENGTH "${raw}" N)
	math(EXPR N "${N} - 1")

	foreach(i RANGE ${N})
		string(SUBSTRING "${raw}" ${i} 1 ch)

		# Detect opening/closing quotes — but DO NOT output them
		if(ch STREQUAL "'")
			if(NOT in_double_quote)
				toggle_bool(in_single_quote)
			endif()
			continue()
		endif()

		if(ch STREQUAL "\"")
			if(NOT in_single_quote)
				toggle_bool(in_double_quote)
			endif()
			continue()
		endif()

		# Replace semicolon only when NOT inside quotes
		if(ch STREQUAL ";")
			if(NOT in_single_quote AND NOT in_double_quote)
				set(ch "\"${_separator}\"")
			else()
				set(ch ";")
			endif()
		endif()

		# Append the character
		set(result "${result}${ch}")
	endforeach()

	set(result "${result}\"")
	set(${_out_var} "${result}" PARENT_SCOPE)
endfunction()


## ensure_build_dir(_out [_component])
##
## Ensure a per-component build directory exists under the current CMake
## binary directory and return its path.
##
## Parameters:
##  - _out: name of the variable to set in the parent scope with the
##          created directory path.
##  - _component: optional component name; if provided the resulting
##                directory will be `${CMAKE_CURRENT_BINARY_DIR}/<sanitized>/`
##                where `<sanitized>` is produced by
##                `sanitize_for_filename` and prefixed with a '/'.
##
## Behavior:
##  - Creates the directory (recursively) if it doesn't exist using
##    `file(MAKE_DIRECTORY ...)` and returns the absolute path in `_out`.
##
function(ensure_build_dir _out)
	if(ARGC LESS 1)
		message(FATAL_ERROR "ensure_build_dir requires an output variable name and optional component name")
	endif()

	set(_out_var "${_out}")

	# Join any additional args into a single component string
	if(ARGC EQUAL 2)
		set(_component "${ARGV1}")
	else()
		set(_component "")
	endif()

	if("${_component}" STREQUAL "")
		set(_sanitized "build")
	else()
		sanitize_for_filename(_sanitized "${_component}")
		set(_sanitized "build/${_sanitized}")
	endif()

	set(_builddir "${CMAKE_CURRENT_BINARY_DIR}/${_sanitized}")
	file(MAKE_DIRECTORY "${_builddir}")
	set(${_out_var} "${_builddir}" PARENT_SCOPE)
endfunction()

# A convenience files to add all bootstrap helper functions
include(${BOOTSTRAP_SRC_DIR}/env/helpers.cmake)
include(${BOOTSTRAP_SRC_DIR}/env/cmake/helpers.cmake)
include(${BOOTSTRAP_SRC_DIR}/env/git/helpers.cmake)
include(${BOOTSTRAP_SRC_DIR}/env/meson/helpers.cmake)