# =============================================================================
# FFmpeg plugins helpers
# =============================================================================
# plugin_require  – optional plugins: disable options + return() if off
# register_plugin – wire ffmpeg-plugins + enable options (no add_subdirectory)
# =============================================================================

## @brief Gate for optional plugins (call at the top of the plugin CMakeLists).
## @param _truthy_value Variable name (e.g. WITH_GPL) or ON/TRUE/1 literal.
## @param _disable_plugin_options Meson fragments when the plugin is off.
## @note Must be a macro so return() leaves the plugin CMakeLists.txt.
macro(plugin_require _truthy_value _disable_plugin_options)
	if(DEFINED ${_truthy_value})
		set(_plugin_require_resolved "${${_truthy_value}}")
	else()
		string(TOUPPER "${_truthy_value}" _plugin_require_tv)
		if(_plugin_require_tv STREQUAL "ON" OR _plugin_require_tv STREQUAL "TRUE" OR _plugin_require_tv STREQUAL "1")
			set(_plugin_require_resolved "TRUE")
		else()
			set(_plugin_require_resolved "")
		endif()
	endif()

	if(NOT _plugin_require_resolved)
		separate_arguments(_plugin_require_opts ${_disable_plugin_options} UNIX_COMMAND)
		list(APPEND FFMPEG_PLUGIN_OPTIONS ${_plugin_require_opts})
		set(FFMPEG_PLUGIN_OPTIONS "${FFMPEG_PLUGIN_OPTIONS}" PARENT_SCOPE)
		return()
	endif()
endmacro()

## @brief Register an enabled plugin with the ffmpeg-plugins meta-target.
## @param _plugin_name Base name; expects target ${_plugin_name}_install and
##        interface/imported library ${_plugin_name}.
## @param _plugin_options Meson enable fragments (space-separated string).
## @note Call after the plugin has defined its install target. Does not
##       add_subdirectory.
macro(register_plugin _plugin_name _plugin_options)
	if(NOT TARGET ffmpeg-plugins)
		message(FATAL_ERROR "register_plugin: target 'ffmpeg-plugins' does not exist yet")
	endif()
	if(NOT TARGET ${_plugin_name}_install)
		message(FATAL_ERROR
			"Plugin '${_plugin_name}' did not define required target '${_plugin_name}_install'")
	endif()

	add_dependencies(ffmpeg-plugins ${_plugin_name}_install)
	target_link_libraries(ffmpeg-plugins INTERFACE ${_plugin_name})

	separate_arguments(_register_plugin_opts ${_plugin_options} UNIX_COMMAND)
	list(APPEND FFMPEG_PLUGIN_OPTIONS ${_register_plugin_opts})
	set(FFMPEG_PLUGIN_OPTIONS "${FFMPEG_PLUGIN_OPTIONS}" PARENT_SCOPE)
endmacro()

## list_to_columns(_out_var _indent _column_width ...)
##
## Format a list of strings into aligned, multi‑column output.
##
## Parameters:
##  - _out_var: variable name (in the parent scope) that will receive the
##              formatted, multi‑line string.
##  - _indent: indentation prefix applied to every generated line.
##  - _column_width: minimum width of each column.
##  - ...: list of items to format.
##
## Behavior:
##  - Arranges items into two aligned columns per line.
##  - Pads each item to `_column_width` characters.
##  - Prepends `_indent` to every generated line.
##  - Stores the final multi‑line string in `_out_var` (parent scope).
function(list_to_columns out_var indent col_width)
	set(result "")
	set(line "")
	set(count 0)

	foreach(item IN LISTS ARGN)
		string(LENGTH "${item}" len)
		math(EXPR pad "${col_width} - ${len}")
		if(pad LESS 1)
			set(pad 1)
		endif()

		string(REPEAT " " ${pad} spaces)
		set(line "${line}${item}${spaces}")

		math(EXPR count "${count} + 1")

		# Emit line every 2 items
		if(count EQUAL 2)
			set(result "${result}${indent}${line}\n")
			set(line "")
			set(count 0)
		endif()
	endforeach()

	# Emit last line if odd number of items
	if(NOT line STREQUAL "")
		set(result "${result}${indent}${line}\n")
	endif()

	set(${out_var} "${result}" PARENT_SCOPE)
endfunction()

## @brief Print a STATUS message with a configurable tab indentation.
## @param[in] _message      Text to show after the leading tabs.
## @param[in] _indent_level Number of tab characters to prepend (non-negative
##                          integer). Values that are not a non-negative integer
##                          are treated as 0.
## @note Uses message(STATUS …). Safe to call from functions or macros.
## @example
##   message_indented("Configuring Opus codec" 2)
##   # → -- \t\tConfiguring Opus codec
function(message_indented _message _indent_level)
	if(_indent_level MATCHES "^[0-9]+$")
		string(REPEAT "\t" ${_indent_level} _indent)
	else()
		set(_indent "")
	endif()
	message(STATUS "${_indent}${_message}")
endfunction()

## @brief Register a build dependency that does not enable any FFmpeg feature.
## @param _plugin_name Base name; expects target ${_plugin_name}_install and
##        interface/imported library ${_plugin_name}.
macro(register_dependency _plugin_name)
	if(NOT TARGET ffmpeg-plugins)
		message(FATAL_ERROR "register_dependency: target 'ffmpeg-plugins' does not exist yet")
	endif()
	if(NOT TARGET ${_plugin_name}_install)
		message(FATAL_ERROR
			"Dependency '${_plugin_name}' did not define required target '${_plugin_name}_install'")
	endif()

	add_dependencies(ffmpeg-plugins ${_plugin_name}_install)
	target_link_libraries(ffmpeg-plugins INTERFACE ${_plugin_name})
endmacro()