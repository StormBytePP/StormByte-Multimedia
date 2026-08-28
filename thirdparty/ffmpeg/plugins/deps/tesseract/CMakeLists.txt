set(TESSERACT_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(TESSERACT_COMPONENT "tesseract")
set(TESSERACT_PATCHES
	"${CMAKE_CURRENT_LIST_DIR}/0001-cmake-unversioned-tesseract-output-name.patch"
)

set(TESSERACT_OPTIONS
	"BUILD_TRAINING_TOOLS=OFF"
	"BUILD_TESTS=OFF"
	"INSTALL_CONFIGS=ON"
	"DISABLE_CURL=ON"
	"DISABLE_ARCHIVE=ON"
	"ENABLE_UNITY_BUILD=ON"
)
if(CMAKE_INTERPROCEDURAL_OPTIMIZATION STREQUAL "ON")
	list(APPEND TESSERACT_OPTIONS "ENABLE_LTO=ON")
endif()
if(FFMPEG_BUNDLED_TYPE STREQUAL "shared")
	list(APPEND TESSERACT_OPTIONS "-DBUILD_SHARED_LIBS=ON")
else()
	list(APPEND TESSERACT_OPTIONS "-DBUILD_SHARED_LIBS=OFF")
endif()

buildmaster_component(
	tesseract
	"${TESSERACT_COMPONENT}"
	"${TESSERACT_SRC_DIR}"
	"${TESSERACT_OPTIONS}"
	"${FFMPEG_BUNDLED_TYPE}"
	"tesseract"
	"GIT={RESET;PATCH={${TESSERACT_PATCHES}};TITLE=tesseract unversioned name}"
)
buildmaster_link(tesseract leptonica tiff jpeg-turbo png zlib)
buildmaster_group_add(ff-deps tesseract)
register_plugin(tesseract "libtesseract=enabled ocr_filter=enabled")
