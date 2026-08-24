cmake_minimum_required(VERSION 3.16)
project(tessdata-data NONE)

# Submodule: cmake-port/src
set(TESSDATA_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src")

include(GNUInstallDirs)

file(GLOB TESSDATA_FILES "${TESSDATA_SRC}/*.traineddata")
if(NOT TESSDATA_FILES)
	message(FATAL_ERROR "tessdata: no *.traineddata under ${TESSDATA_SRC}")
endif()

install(FILES ${TESSDATA_FILES}
	DESTINATION "${CMAKE_INSTALL_DATADIR}/tessdata"
)
