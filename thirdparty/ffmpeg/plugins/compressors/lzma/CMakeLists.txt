set(LZMA_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/src")
ensure_build_dir(LZMA_BUILD_DIR)
set(LZMA_COMPONENT "liblzma (xz) compression library")

# Workaround for https://github.com/mesonbuild/meson/issues/16138
# (static dependency: Cflags.private not applied to compile feature checks).
# xz puts -DLZMA_API_STATIC in Cflags.private; without promoting it to public
# Cflags, MSVC has_function(lzma_version_number) fails with unresolved __imp_*.
set(LZMA_PATCH_PC
    "${CMAKE_CURRENT_LIST_DIR}/liblzma-pc-promote-LZMA_API_STATIC-to-Cflags.patch")
create_git_patch_file(
    LZMA_GIT_PATCH_FILE
    lzma
    "liblzma.pc: put LZMA_API_STATIC in public Cflags"
    "${LZMA_SRC_DIR}"
    "${LZMA_PATCH_PC}"
)
include("${LZMA_GIT_PATCH_FILE}")

# tukaani-project/xz: static liblzma only (no CLI tools/docs)
set(LZMA_OPTIONS
    "-DBUILD_SHARED_LIBS=OFF"
    "-DXZ_TOOL_XZ=OFF"
    "-DXZ_TOOL_XZDEC=OFF"
    "-DXZ_TOOL_LZMADEC=OFF"
    "-DXZ_TOOL_LZMAINFO=OFF"
    "-DXZ_TOOL_SYMLINKS=OFF"
    "-DXZ_TOOL_SYMLINKS_LZMA=OFF"
    "-DXZ_TOOL_SCRIPTS=OFF"
    "-DXZ_DOC=OFF"
    "-DXZ_DOXYGEN=OFF"
    "-DXZ_NLS=OFF"
)

create_cmake_component(
    LZMA_LIBRARY_CREATE_FILE
    "lzma"
    "${LZMA_COMPONENT}"
    "${LZMA_SRC_DIR}"
    "${LZMA_BUILD_DIR}"
    "${LZMA_OPTIONS}"
    "${FFMPEG_BUNDLED_TYPE}"
    "lzma"
    ${PLUGIN_LEVEL}
)
include("${LZMA_LIBRARY_CREATE_FILE}")

register_plugin(lzma "-Dlzma=enabled")