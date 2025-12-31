## register_plugin(_plugin_name _plugin_options)
##
## Register a plugin build target with the top-level `ffmpeg-plugins` meta-
## target and optionally provide Meson/CMake option fragments needed by that
## plugin.
##
## Parameters:
##  - _plugin_name: name of the plugin target expected to be defined in the
##    plugin subdirectory (e.g. "libavfilter_myplugin").
##  - _plugin_options: a string or list of Meson option fragments appended to
##    the global `FFMPEG_PLUGIN_OPTIONS` list (these options are forwarded to
##    the FFmpeg Meson invocation). Examples: "-Denable_myplugin=enabled" or
##    a CMake list of several such items.
##
## Behavior:
##  - Validates the argument count and errors if incorrect.
##  - Verifies the target `${_plugin_name}` exists; if not, it fails
##    configuration with `message(FATAL_ERROR ...)`.
##  - Adds the plugin subdirectory so the external project can be built and
##    adds a dependency from `ffmpeg-plugins` to the plugin target so the
##    plugin is built as part of the meta-target.
##  - Appends `_plugin_options` into the global `FFMPEG_PLUGIN_OPTIONS` list
##    and exports that list to the parent scope for later use when configuring
##    the FFmpeg Meson build.
macro(register_plugin _plugin_name _plugin_options)
	if(NOT ${ARGC} EQUAL 2)
		message(FATAL_ERROR "register_plugin requires plugin name and plugin options")
	endif()

	# Add subdirectory so external project can be built
	add_subdirectory(${_plugin_name})

	# If required target is not set, error out
	if(NOT TARGET ${_plugin_name}_install)
		message(FATAL_ERROR "Plugin '${_plugin_name}' did not define required target '${_plugin_name}_install'")
	endif()

	# If required outputs variable is not set, error out
	if(NOT DEFINED ${_plugin_name}_outputs)
		message(FATAL_ERROR "Plugin '${_plugin_name}' did not define required variable '${_plugin_name}_outputs'")
	endif()

	# Add a library with the plugin outputs
	if(FFMPEG_BUNDLED STREQUAL "static")
		add_library(${_plugin_name} STATIC IMPORTED GLOBAL)
	else()
		add_library(${_plugin_name} SHARED IMPORTED GLOBAL)
	endif()
	set_target_properties(${_plugin_name} PROPERTIES
		IMPORTED_LOCATION "${${_plugin_name}_outputs}"
		IMPORTED_LOCATION_DEBUG "${${_plugin_name}_outputs}"
		IMPORTED_LOCATION_RELEASE "${${_plugin_name}_outputs}"
		IMPORTED_LOCATION_MINSIZEREL "${${_plugin_name}_outputs}"
		IMPORTED_LOCATION_RELWITHDEBINFO "${${_plugin_name}_outputs}"
	)

	# Add dependency on the plugin target
	target_link_libraries(ffmpeg-plugins INTERFACE ${_plugin_name})
	add_dependencies(ffmpeg-plugins-install ${_plugin_name}_install)

	# Declare the plugin output files as produced by the plugin's install
	# target so build generators (ninja) know which rule generates the
	# concrete files and can schedule the install before linking. This
	# mirrors the approach used for core ffmpeg outputs.
	add_custom_command(OUTPUT ${${_plugin_name}_outputs} DEPENDS ${_plugin_name}_install)

	# Create a custom target that depends on the output files. This makes
	# CMake/Ninja generate an explicit rule for producing the files (via
	# the command above) and lets other targets depend on this target to
	# ensure correct build ordering.
	add_custom_target(${_plugin_name}_outputs_target DEPENDS ${${_plugin_name}_outputs})
	add_dependencies(ffmpeg-plugins-install ${_plugin_name}_outputs_target)

	# Register plugin options: append to a temporary list and export to parent
	separate_arguments(_opts_list ${_plugin_options} UNIX_COMMAND)
	list(APPEND FFMPEG_PLUGIN_OPTIONS ${_opts_list})
	set(FFMPEG_PLUGIN_OPTIONS "${FFMPEG_PLUGIN_OPTIONS}" PARENT_SCOPE)
endmacro()

## register_plugin_optional(_plugin_name _truthy_value _enable_plugin_options _disable_plugin_options)
##
## Conditionally register a plugin and its associated option fragments.
##
## Parameters:
##  - _plugin_name: plugin target name to register when the condition is true.
##  - _truthy_value: expression evaluated as truthy/falsey; if truthy,
##    `register_plugin(_plugin_name _enable_plugin_options)` is invoked.
##  - _enable_plugin_options: option fragments appended to
##    `FFMPEG_PLUGIN_OPTIONS` when the plugin is enabled.
##  - _disable_plugin_options: option fragments appended when the plugin is
##    disabled.
##
## Behavior:
##  - Validates argument count and forwards to `register_plugin` when
##    `_truthy_value` evaluates to true; otherwise appends the disabled
##    option fragments into `FFMPEG_PLUGIN_OPTIONS`.
macro(register_plugin_optional _plugin_name _truthy_value _enable_plugin_options _disable_plugin_options)
	if(NOT ${ARGC} EQUAL 4)
		message(FATAL_ERROR "register_plugin_optional requires plugin name, a truthy value, plugin options if enabled and plugin options if disabled")
	endif()
	if(${_truthy_value})
		register_plugin(${_plugin_name} ${_enable_plugin_options})
	else()
		# Register plugin options: append to a temporary list and export to parent
		separate_arguments(_opts_list ${_disable_plugin_options} UNIX_COMMAND)
		list(APPEND FFMPEG_PLUGIN_OPTIONS ${_opts_list})
		set(FFMPEG_PLUGIN_OPTIONS "${FFMPEG_PLUGIN_OPTIONS}" PARENT_SCOPE)
	endif()
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