set(XVID_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake-port")
ensure_build_dir(XVID_BUILD_DIR)
if(FFMPEG_BUNDLED STREQUAL "static")
	library_import_static_hint(XVID_LIBRARY "xvidcore" "${FFMPEG_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}")
else()
	library_import_hint(XVID_LIBRARY "xvidcore" "${FFMPEG_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}")
endif()

set(XVID_COMPONENT "XVID codec")
set(XVID_OPTIONS
	"-DBUILD_SHARED_LIBS=${CMAKE_SHARED_MODE}"
	"-DCMAKE_POSITION_INDEPENDENT_CODE=ON"
	"-DCMAKE_ASM_NASM_FLAGS=-DPIC -w-pp-macro-params-legacy"
)
create_cmake_component(
	XVID_LIBRARY_CREATE_FILE
	"xvid"
	"${XVID_COMPONENT}"
	"${XVID_SRC_DIR}"
	"${XVID_BUILD_DIR}"
	"${XVID_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"xvidcore"
	2
)
include("${XVID_LIBRARY_CREATE_FILE}")