# =============================================================================
# FFmpeg plugins helpers
# =============================================================================
# plugin_require            – optional plugins: disable flags + return() if off
# register_builtin_plugin   – Meson enable/disable flags only (no graph)
# register_plugin           – meta membership + Meson enable flags
# register_dependency       – meta membership, no Meson flags
# =============================================================================

## @brief Strip a leading `-D` from one Meson fragment.
## @param[in] _tok Raw token (`-Dlibgsm=enabled` or `libgsm=enabled`).
## @param[out] _out Name of the variable receiving the bare `key=value`.
function(_ff_meson_opt_bare _tok _out)
	set(_t "${_tok}")
	if(_t MATCHES "^[-]D(.+)$")
		set(_t "${CMAKE_MATCH_1}")
	endif()
	set(${_out} "${_t}" PARENT_SCOPE)
endfunction()

## @brief Prefix `-D` for human output only.
## @param[in] _tok Bare or already-prefixed fragment.
## @param[out] _out Name of the variable receiving `-Dkey=value`.
function(_ff_meson_opt_display _tok _out)
	_ff_meson_opt_bare("${_tok}" _bare)
	if(_bare STREQUAL "")
		set(${_out} "" PARENT_SCOPE)
	else()
		set(${_out} "-D${_bare}" PARENT_SCOPE)
	endif()
endfunction()

## @brief Gate for optional plugins (call at the top of the plugin CMakeLists).
## @param _truthy_value Variable name (e.g. WITH_GPL) or ON/TRUE/1 literal.
## @param _disable_plugin_options Meson fragments when the plugin is off
##        (`key=value` or legacy `-Dkey=value`; stored bare).
## @note Must be a macro so return() leaves the plugin CMakeLists.txt.
## @note Off path only records Meson flags. No component is registered.
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
		register_builtin_plugin("${_disable_plugin_options}")
		return()
	endif()
endmacro()

## @brief Append FFmpeg/Meson fragments. No component. No graph edge.
## @param _plugin_options Space-separated Meson fragments
##        (e.g. "ass_demuxer=enabled ass_muxer=enabled").
## @note Leading `-D` is stripped. Writes FFMPEG_PLUGIN_OPTIONS in the
##       caller scope (macro).
## @note Use this for built-in FFmpeg features that have no third-party lib.
macro(register_builtin_plugin _plugin_options)
	separate_arguments(_register_plugin_opts ${_plugin_options} UNIX_COMMAND)
	set(_register_plugin_bare "")
	foreach(_register_plugin_tok IN LISTS _register_plugin_opts)
		_ff_meson_opt_bare("${_register_plugin_tok}" _register_plugin_one)
		if(NOT _register_plugin_one STREQUAL "")
			list(APPEND _register_plugin_bare "${_register_plugin_one}")
		endif()
	endforeach()
	list(APPEND FFMPEG_PLUGIN_OPTIONS ${_register_plugin_bare})
	set(FFMPEG_PLUGIN_OPTIONS "${FFMPEG_PLUGIN_OPTIONS}" PARENT_SCOPE)
endmacro()

## @brief Register an enabled plugin on the ffmpeg-plugins meta + Meson flags.
## @param _plugin_name Component id. Must already have been declared.
## @param _plugin_options Space-separated Meson enable fragments (bare or `-D`).
## @note Membership is not consumption: the parent still
##       buildmaster_link(ffmpeg ffmpeg-plugins).
macro(register_plugin _plugin_name _plugin_options)
	buildmaster_meta_add(ffmpeg-plugins "${_plugin_name}")
	buildmaster_link(ffmpeg-plugins "${_plugin_name}")
	register_builtin_plugin("${_plugin_options}")
endmacro()

## @brief Register a graph member that does not enable any FFmpeg feature.
## @param _plugin_name Component id. Must already have been declared.
## @note Same membership + link as register_plugin, without Meson flags.
macro(register_dependency _plugin_name)
	buildmaster_meta_add(ffmpeg-plugins "${_plugin_name}")
	buildmaster_link(ffmpeg-plugins "${_plugin_name}")
endmacro()

## @brief Format a list of strings into two aligned columns.
## @param[out] out_var Parent-scope variable that receives the multi-line string.
## @param[in] indent Prefix applied to every generated line.
## @param[in] col_width Minimum width of each column.
## @param[in] ARGN Items to format.
## @note Display tokens get `-D` if they do not already have it.
function(list_to_columns out_var indent col_width)
	set(result "")
	set(line "")
	set(count 0)

	foreach(item IN LISTS ARGN)
		_ff_meson_opt_display("${item}" item)
		string(LENGTH "${item}" len)
		math(EXPR pad "${col_width} - ${len}")
		if(pad LESS 1)
			set(pad 1)
		endif()

		string(REPEAT " " ${pad} spaces)
		set(line "${line}${item}${spaces}")

		math(EXPR count "${count} + 1")

		if(count EQUAL 2)
			set(result "${result}${indent}${line}\n")
			set(line "")
			set(count 0)
		endif()
	endforeach()

	if(NOT line STREQUAL "")
		set(result "${result}${indent}${line}\n")
	endif()

	set(${out_var} "${result}" PARENT_SCOPE)
endfunction()
