set(BOOTSTRAP_GIT_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(BOOTSTRAP_GIT_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")

## create_git_patch_file(_out_file _component _git_repo_dir _git_patches)
##
## Generate a CMake script that applies a set of git patches to a repository.
##
## Parameters:
##  - _out_file: name of the variable to set (in parent scope) with the
##               generated CMake filename that will apply the patches.
##  - _component: arbitrary component name used to produce a filesystem-
##                safe filename for the generated script.
##  - _git_repo_dir: path to the git repository where the patches should
##                   be applied (this value is written into the generated
##                   script as `GIT_REPO`).
##  - _git_patches: CMake list of patch filenames (may be a list variable
##                  name). The list is joined into a space-separated string
##                  and exposed as `GIT_PATCHES` in the generated script.
##
## Behavior:
##  - Sanitizes `_component` into a safe filename fragment.
##  - Produces a file named `git_patch_<sanitized>.cmake` under the
##    bootstrap git binary directory by configuring `patch.cmake.in` with the
##    variables above.
##  - Returns the path to the generated file in `_out_file` (parent scope).
##
function(create_git_patch_file _file _component _git_repo_dir _git_paches)
	set(GIT_REPO "${_git_repo_dir}")
	list_join(GIT_PATCHES "${_git_paches}" " ")
	sanitize_for_filename(_GIT_PATCH_NAME "${_component}")
	set(_GIT_PATCH_FILE "${BOOTSTRAP_SCRIPTS_GIT_DIR}/git_patch_${_GIT_PATCH_NAME}.cmake")
	configure_file(
		"${BOOTSTRAP_SRC_DIR}/env/git/patch.cmake.in"
		"${_GIT_PATCH_FILE}"
		@ONLY
	)
	set(${_file} "${_GIT_PATCH_FILE}" PARENT_SCOPE)
endfunction()


## create_git_reset_file(_out_file _component _git_repo_dir)
##
## Generate a CMake script that resets a repository to a clean state.
##
## Parameters:
##  - _out_file: name of the variable to set (in parent scope) with the
##               generated reset script filename.
##  - _component: arbitrary component name used for the generated filename.
##  - _git_repo_dir: path to the git repository to reset (written as
##                   `GIT_REPO` into the generated file).
##
## Behavior:
##  - Sanitizes `_component` and creates `git_reset_<sanitized>.cmake` in
##    the bootstrap git binary dir using `reset.cmake.in` as a template.
##  - Returns the path to the generated file in `_out_file` (parent scope).
function(create_git_reset_file _file _component _git_repo_dir)
	set(GIT_REPO "${_git_repo_dir}")
	sanitize_for_filename(_GIT_RESET_NAME "${_component}")
	set(_GIT_RESET_FILE "${BOOTSTRAP_SCRIPTS_GIT_DIR}/git_reset_${_GIT_RESET_NAME}.cmake")
	configure_file(
		"${BOOTSTRAP_SRC_DIR}/env/git/reset.cmake.in"
		"${_GIT_RESET_FILE}"
		@ONLY
	)
	set(${_file} "${_GIT_RESET_FILE}" PARENT_SCOPE)
endfunction()

## create_git_switch_branch(_out_file _component _git_repo_dir _git_branch)
##
## Generate a CMake script that switches a repository to a specific branch.
##
## Parameters:
##  - _out_file: name of the variable to set (in parent scope) with the
##               generated switch script filename.
##  - _component: arbitrary component name used to produce a filesystem-
##                safe filename for the generated script.
##  - _git_repo_dir: path to the git repository to switch (this value is
##                   written into the generated script as `GIT_REPO`).
##  - _git_branch: branch name to check out (exposed as `GIT_BRANCH`).
##
## Behavior:
##  - Sanitizes `_component` into a safe filename fragment.
##  - Produces a file named `git_switch_<sanitized>.cmake` under the
##    bootstrap git binary directory by configuring `switch.cmake.in` with
##    the variables above.
##  - Returns the path to the generated file in `_out_file` (parent scope).
function(create_git_switch_branch _file _component _git_repo_dir _git_branch)
	set(GIT_REPO "${_git_repo_dir}")
	set(GIT_BRANCH "${_git_branch}")
	sanitize_for_filename(_GIT_SWITCH_NAME "${_component}")
	set(_GIT_SWITCH_FILE "${BOOTSTRAP_SCRIPTS_GIT_DIR}/git_switch_${_GIT_SWITCH_NAME}.cmake")
	configure_file(
		"${BOOTSTRAP_SRC_DIR}/env/git/switch.cmake.in"
		"${_GIT_SWITCH_FILE}"
		@ONLY
	)
	set(${_file} "${_GIT_SWITCH_FILE}" PARENT_SCOPE)
endfunction()

## create_git_fetch(_out_file _component _git_repo_dir)
##
## Generate a CMake script that performs a `git fetch` operation for a
## repository. The function does not execute git itself; it produces a
## standalone CMake script (from `fetch.cmake.in`) that contains the
## commands to fetch updates for the target repository.
##
## Parameters:
##  - _out_file: name of the variable to set (in parent scope) with the
##               generated fetch script filename.
##  - _component: arbitrary component name used to produce a filesystem-
##                safe filename for the generated script.
##  - _git_repo_dir: path to the git repository to fetch (written as
##                   `GIT_REPO` into the generated file).
##
## Behavior:
##  - Sanitizes `_component` into a safe filename fragment.
##  - Produces a file named `git_fetch_<sanitized>.cmake` under the
##    bootstrap git binary directory by configuring `fetch.cmake.in` with
##    the variables above.
##  - Returns the path to the generated file in `_out_file` (parent scope).
function(create_git_fetch _file _component _git_repo_dir)
	set(GIT_REPO "${_git_repo_dir}")
	sanitize_for_filename(_GIT_FETCH_NAME "${_component}")
	set(_GIT_FETCH_FILE "${BOOTSTRAP_SCRIPTS_GIT_DIR}/git_fetch_${_GIT_FETCH_NAME}.cmake")
	configure_file(
		"${BOOTSTRAP_SRC_DIR}/env/git/fetch.cmake.in"
		"${_GIT_FETCH_FILE}"
		@ONLY
	)
	set(${_file} "${_GIT_FETCH_FILE}" PARENT_SCOPE)
endfunction()
