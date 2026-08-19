cmake_minimum_required(VERSION 3.12)
project(mp3lame LANGUAGES C)

option(BUILD_SHARED_LIBS "Build shared library" ON)
option(LAME_ENABLE_SSE "Enable SSE2 intrinsics" ON)

# --- Detect SSE2 ---------------------------------------------------------

include(CheckCCompilerFlag)

if(LAME_ENABLE_SSE)
    if(MSVC)
        check_c_compiler_flag("/arch:SSE2" COMPILER_SUPPORTS_SSE2)
        if(COMPILER_SUPPORTS_SSE2)
            add_compile_options(/arch:SSE2)
            message(STATUS "SSE2 enabled (/arch:SSE2)")
        endif()
    else()
        check_c_compiler_flag("-msse2" COMPILER_SUPPORTS_SSE2)
        if(COMPILER_SUPPORTS_SSE2)
            add_compile_options(-msse2)
            message(STATUS "SSE2 enabled (-msse2)")
        endif()
    endif()
endif()

# --- Sources -------------------------------------------------------------

set(LAME_SRC
    ../src/libmp3lame/bitstream.c
	../src/libmp3lame/gain_analysis.c
    ../src/libmp3lame/encoder.c
    ../src/libmp3lame/fft.c
    ../src/libmp3lame/id3tag.c
    ../src/libmp3lame/lame.c
    ../src/libmp3lame/newmdct.c
    ../src/libmp3lame/presets.c
    ../src/libmp3lame/psymodel.c
    ../src/libmp3lame/quantize.c
    ../src/libmp3lame/quantize_pvt.c
    ../src/libmp3lame/reservoir.c
    ../src/libmp3lame/set_get.c
    ../src/libmp3lame/tables.c
    ../src/libmp3lame/takehiro.c
    ../src/libmp3lame/util.c
    ../src/libmp3lame/vbrquantize.c
    ../src/libmp3lame/VbrTag.c
    ../src/libmp3lame/version.c
)

# --- Library -------------------------------------------------------------

if(BUILD_SHARED_LIBS)
	add_library(mp3lame SHARED ${LAME_SRC})
else()
	add_library(mp3lame STATIC ${LAME_SRC})
endif()
set_target_properties(mp3lame PROPERTIES VERSION 3.100.1 SOVERSION 3 OUTPUT_NAME "mp3lame")
if(WIN32)
	if(MSVC)
		set_target_properties(mp3lame PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
	else()
		target_link_options(mp3lame PRIVATE "-Wl,--export-all-symbols")
	endif()
endif()

target_include_directories(mp3lame
    PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src/libmp3lame>
        $<INSTALL_INTERFACE:include>
)

target_compile_definitions(mp3lame
    PRIVATE
        HAVE_CONFIG_H=1
)

if(WIN32)
    target_compile_definitions(mp3lame PRIVATE
        _CRT_SECURE_NO_WARNINGS
        LAME_LIBRARY_BUILD
    )
endif()

add_library(mp3lame::mp3lame ALIAS mp3lame)

# --- Optional/optimized sources -----------------------------------------
include(CheckIncludeFile)
check_include_file("xmmintrin.h" HAVE_XMMINTRIN_H)

if(HAVE_XMMINTRIN_H AND COMPILER_SUPPORTS_SSE2)
    target_sources(mp3lame PRIVATE ../src/libmp3lame/vector/xmm_quantize_sub.c)
    target_compile_definitions(mp3lame PRIVATE HAVE_XMMINTRIN_H=1)
    message(STATUS "Adding xmm_quantize_sub.c (SSE intrinsics)")
endif()

# Assemble i386 NASM sources for 32-bit builds (if nasm available)
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    find_program(NASM_EXECUTABLE nasm)
    if(NASM_EXECUTABLE)
        file(GLOB I386_NAS_SRC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "../src/libmp3lame/i386/*.nas")
        set(NASM_OBJECTS)
        foreach(NAS ${I386_NAS_SRC})
            get_filename_component(NAME_WE ${NAS} NAME_WE)
            set(OBJ ${CMAKE_CURRENT_BINARY_DIR}/${NAME_WE}.o)
            add_custom_command(OUTPUT ${OBJ}
                COMMAND ${NASM_EXECUTABLE} -f elf -o ${OBJ} ${CMAKE_CURRENT_SOURCE_DIR}/${NAS}
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${NAS}
                COMMENT "Assembling ${NAS}"
            )
            list(APPEND NASM_OBJECTS ${OBJ})
        endforeach()
        if(NASM_OBJECTS)
            target_sources(mp3lame PRIVATE ${NASM_OBJECTS})
            message(STATUS "Adding i386 NASM objects to mp3lame")
        endif()
    else()
        message(STATUS "NASM not found; skipping i386 optimized sources")
    endif()
endif()

# DirectShow front-end (Windows-only)
if(WIN32)
    file(GLOB DSHOW_SRCS ../src/dshow/*.c ../src/dshow/*.cpp)
    if(DSHOW_SRCS)
        target_sources(mp3lame PRIVATE ${DSHOW_SRCS})
        if(MSVC)
            target_link_libraries(mp3lame PRIVATE Strmiids ole32 uuid)
        else()
            target_link_libraries(mp3lame PRIVATE strmiids ole32 uuid)
        endif()
        message(STATUS "Adding DirectShow front-end sources")
    endif()
endif()

# --- Install -------------------------------------------------------------

include(GNUInstallDirs)

install(
    TARGETS mp3lame
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    FILES ../src/include/lame.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lame
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/mp3lame.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/mp3lame.pc
    @ONLY
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/mp3lame.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)