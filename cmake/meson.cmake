# Helper to run Meson from CMake in a consistent way.
# function: run_meson(build_dir, src_dir, meson_options)
# - build_dir: path to meson build dir
# - src_dir: path to meson source dir
# - meson_options: single string (space- or semicolon-separated) of extra meson args

function(run_meson build_dir src_dir meson_options)
  if(NOT ARGV0)
    message(FATAL_ERROR "run_meson requires build_dir and src_dir")
  endif()

  # locate meson if not provided
  if(NOT DEFINED MESON_EXECUTABLE)
    find_program(MESON_EXECUTABLE meson)
    if(NOT MESON_EXECUTABLE)
      message(FATAL_ERROR "meson executable not found in PATH")
    endif()
  endif()

  # Normalize options: accept semicolon or space separated lists
  if(NOT meson_options)
    set(meson_options_str "")
  else()
    set(meson_options_str "${meson_options}")
  endif()
  string(REPLACE ";" " " _meson_opts_space "${meson_options_str}")
  separate_arguments(_meson_args UNIX_COMMAND "${_meson_opts_space}")

  # Build the meson setup command argv
  set(_cmd_base ${MESON_EXECUTABLE} setup "${build_dir}" "${src_dir}")

  if(WIN32)
    # If the VS dev environment wrapper is present in the environment, call it
    if(DEFINED ENV{VSDEVCMD})
      set(_vs_call "call \"${ENV{VSDEVCMD}}\"")
    else()
      set(_vs_call "")
    endif()

    if(_vs_call STREQUAL "")
      # No VS wrapper known, just run meson via cmd (this still runs in cmd shell)
      set(_cmdline "${MESON_EXECUTABLE} setup \"${build_dir}\" \"${src_dir}\"")
      foreach(a IN LISTS _meson_args)
        string(APPEND _cmdline " ${a}")
      endforeach()
    else()
      set(_cmdline "${_vs_call} && ${MESON_EXECUTABLE} setup \"${build_dir}\" \"${src_dir}\"")
      foreach(a IN LISTS _meson_args)
        string(APPEND _cmdline " ${a}")
      endforeach()
    endif()

    execute_process(
      COMMAND cmd /C "${_cmdline}"
      WORKING_DIRECTORY "${build_dir}"
      RESULT_VARIABLE MESON_RC
      OUTPUT_VARIABLE MESON_OUT
      ERROR_VARIABLE MESON_ERR
    )
  else()
    # POSIX-like platforms: pass tokenized args to execute_process
    execute_process(
      COMMAND ${_cmd_base} ${_meson_args}
      WORKING_DIRECTORY "${build_dir}"
      RESULT_VARIABLE MESON_RC
      OUTPUT_VARIABLE MESON_OUT
      ERROR_VARIABLE MESON_ERR
    )
  endif()

  # Export results to caller scope
  set(MESON_LAST_RESULT "${MESON_RC}" PARENT_SCOPE)
  set(MESON_LAST_OUT "${MESON_OUT}" PARENT_SCOPE)
  set(MESON_LAST_ERR "${MESON_ERR}" PARENT_SCOPE)
endfunction()
