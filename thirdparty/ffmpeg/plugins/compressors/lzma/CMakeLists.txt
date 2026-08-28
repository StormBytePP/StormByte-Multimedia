set(LZMA_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/src")
set(LZMA_COMPONENT "liblzma (xz) compression library")
set(LZMA_PATCH_PC
	"${CMAKE_CURRENT_LIST_DIR}/liblzma-pc-promote-LZMA_API_STATIC-to-Cflags.patch")

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

buildmaster_component(
	lzma
	"${LZMA_COMPONENT}"
	"${LZMA_SRC_DIR}"
	"${LZMA_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"lzma"
	"GIT={RESET;PATCH=${LZMA_PATCH_PC};TITLE=lzma pc Cflags}"
)
buildmaster_group_add(ff-compressors lzma)
register_plugin(lzma "lzma=enabled")
