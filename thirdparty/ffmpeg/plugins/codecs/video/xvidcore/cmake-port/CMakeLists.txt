cmake_minimum_required(VERSION 3.16)
project(xvidcore LANGUAGES C)

# NASM only on x86. Apple Silicon stays pure C.
set(XVID_IS_X86 FALSE)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|i[3-6]86|x86")
	set(XVID_IS_X86 TRUE)
endif()

# Honour CMAKE_OSX_ARCHITECTURES when the parent forces a slice.
if(APPLE AND CMAKE_OSX_ARCHITECTURES)
	list(FIND CMAKE_OSX_ARCHITECTURES "x86_64" _idx)
	if(_idx GREATER -1)
		set(XVID_IS_X86 TRUE)
	else()
		set(XVID_IS_X86 FALSE)
	endif()
endif()

if(XVID_IS_X86)
	enable_language(ASM_NASM)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ---------------------------------------------
# Sources (C always)
# ---------------------------------------------
set(XVID_SRC
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/decoder.c
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/encoder.c

	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/bitstream/bitstream.c
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/bitstream/cbp.c
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/bitstream/mbcoding.c

	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/dct/fdct.c
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/dct/idct.c
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/dct/simple_idct.c

	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/image/colorspace.c
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/image/font.c
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/image/image.c
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/image/interpolate8x8.c
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/image/postprocessing.c
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/image/qpel.c
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/image/reduced.c

	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/motion/estimation_bvop.c
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/motion/estimation_common.c
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/motion/estimation_gmc.c
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/motion/estimation_pvop.c
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/motion/estimation_rd_based.c
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/motion/estimation_rd_based_bvop.c
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/motion/gmc.c
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/motion/motion_comp.c
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/motion/sad.c
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/motion/vop_type_decision.c

	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/quant/quant_h263.c
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/quant/quant_matrix.c
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/quant/quant_mpeg.c

	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/utils/emms.c
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/utils/mbtransquant.c
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/utils/mem_align.c
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/utils/mem_transfer.c
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/utils/timer.c

	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/prediction/mbprediction.c

	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/plugins/plugin_2pass1.c
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/plugins/plugin_2pass2.c
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/plugins/plugin_dump.c
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/plugins/plugin_lumimasking.c
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/plugins/plugin_psnr.c
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/plugins/plugin_psnrhvsm.c
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/plugins/plugin_single.c
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/plugins/plugin_ssim.c

	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/xvid.c
)

# ---------------------------------------------
# ASM (x86 only)
# ---------------------------------------------
set(XVID_ASM)
if(XVID_IS_X86)
	set(XVID_ASM
		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/bitstream/x86_asm/cbp_mmx.asm
		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/bitstream/x86_asm/cbp_sse2.asm

		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/dct/x86_asm/fdct_mmx_ffmpeg.asm
		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/dct/x86_asm/fdct_mmx_skal.asm
		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/dct/x86_asm/fdct_sse2_skal.asm
		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/dct/x86_asm/idct_3dne.asm
		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/dct/x86_asm/idct_mmx.asm
		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/dct/x86_asm/idct_sse2_dmitry.asm

		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/image/x86_asm/colorspace_rgb_mmx.asm
		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/image/x86_asm/colorspace_yuv_mmx.asm
		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/image/x86_asm/colorspace_yuyv_mmx.asm
		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/image/x86_asm/deintl_sse.asm
		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/image/x86_asm/gmc_mmx.asm
		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/image/x86_asm/interpolate8x8_3dn.asm
		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/image/x86_asm/interpolate8x8_3dne.asm
		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/image/x86_asm/interpolate8x8_mmx.asm
		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/image/x86_asm/interpolate8x8_xmm.asm
		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/image/x86_asm/postprocessing_mmx.asm
		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/image/x86_asm/postprocessing_sse2.asm
		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/image/x86_asm/qpel_mmx.asm
		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/image/x86_asm/reduced_mmx.asm

		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/motion/x86_asm/sad_3dn.asm
		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/motion/x86_asm/sad_3dne.asm
		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/motion/x86_asm/sad_mmx.asm
		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/motion/x86_asm/sad_sse2.asm
		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/motion/x86_asm/sad_xmm.asm

		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/quant/x86_asm/quantize_h263_3dne.asm
		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/quant/x86_asm/quantize_h263_mmx.asm
		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/quant/x86_asm/quantize_mpeg_mmx.asm
		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/quant/x86_asm/quantize_mpeg_xmm.asm

		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/utils/x86_asm/cpuid.asm
		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/utils/x86_asm/interlacing_mmx.asm
		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/utils/x86_asm/mem_transfer_3dne.asm
		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/utils/x86_asm/mem_transfer_mmx.asm

		${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/plugins/x86_asm/plugin_ssim-a.asm
	)

	set(CMAKE_ASM_NASM_FLAGS "${CMAKE_ASM_NASM_FLAGS} -I${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/bitstream/x86_asm -I${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/dct/x86_asm -I${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/image/x86_asm -I${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/motion/x86_asm -I${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/plugins/x86_asm -I${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/quant/x86_asm -I${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/utils/x86_asm")
endif()

# ---------------------------------------------
# Platform-specific configuration
# ---------------------------------------------
if(MSVC)
	add_compile_definitions(
		WIN32
		_WINDOWS
		_CRT_SECURE_NO_WARNINGS
		HAVE_PTHREAD
	)

	if(XVID_IS_X86)
		if(CMAKE_SIZEOF_VOID_P EQUAL 8)
			add_compile_definitions(ARCH_IS_X86_64 ARCH_IS_64BIT)
			set(NASM_FORMAT win64)
		else()
			add_compile_definitions(ARCH_IS_IA32 ARCH_IS_32BIT)
			set(NASM_FORMAT win32)
		endif()

		set(CMAKE_ASM_NASM_OBJECT_FORMAT ${NASM_FORMAT})
		if(CMAKE_SIZEOF_VOID_P EQUAL 8)
			set(CMAKE_ASM_NASM_FLAGS "${CMAKE_ASM_NASM_FLAGS} -DWINDOWS -DNO_PREFIX -I${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src")
		else()
			set(CMAKE_ASM_NASM_FLAGS "${CMAKE_ASM_NASM_FLAGS} -DWINDOWS -I${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src")
		endif()
	else()
		add_compile_definitions(ARCH_IS_64BIT)
	endif()

	set(MSVC_FLAGS /W3 /MP)

else()
	add_compile_definitions(HAVE_PTHREAD)

	if(XVID_IS_X86)
		if(CMAKE_SIZEOF_VOID_P EQUAL 8)
			add_compile_definitions(ARCH_IS_X86_64 ARCH_IS_64BIT)
		else()
			add_compile_definitions(ARCH_IS_IA32 ARCH_IS_32BIT)
		endif()

		# Darwin C uses Mach-O underscore prefix. elf64 leaves
		# check_cpu_features without '_' and ld looks for _check_cpu_features.
		if(APPLE)
			if(CMAKE_SIZEOF_VOID_P EQUAL 8)
				set(NASM_FORMAT macho64)
			else()
				set(NASM_FORMAT macho32)
			endif()
		else()
			if(CMAKE_SIZEOF_VOID_P EQUAL 8)
				set(NASM_FORMAT elf64)
			else()
				set(NASM_FORMAT elf32)
			endif()
		endif()

		set(CMAKE_ASM_NASM_OBJECT_FORMAT ${NASM_FORMAT})
		set(CMAKE_ASM_NASM_FLAGS "${CMAKE_ASM_NASM_FLAGS} -I${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src")
		if(APPLE)
			set(CMAKE_ASM_NASM_FLAGS "${CMAKE_ASM_NASM_FLAGS} -DPREFIX")
		endif()
	else()
		add_compile_definitions(ARCH_IS_GENERIC)
		if(CMAKE_SIZEOF_VOID_P EQUAL 8)
			add_compile_definitions(ARCH_IS_64BIT)
		else()
			add_compile_definitions(ARCH_IS_32BIT)
		endif()
	endif()
endif()

# ---------------------------------------------
# Library
# ---------------------------------------------
option(BUILD_SHARED_LIBS "Build XVID as shared library" OFF)
if(BUILD_SHARED_LIBS)
	add_library(xvidcore SHARED ${XVID_SRC} ${XVID_ASM})
else()
	add_library(xvidcore STATIC ${XVID_SRC} ${XVID_ASM})
endif()

target_include_directories(xvidcore PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src
)

# ---------------------------------------------
# Compiler flags
# ---------------------------------------------
if(MSVC)
	target_compile_options(xvidcore PRIVATE $<$<COMPILE_LANGUAGE:C>:${MSVC_FLAGS}>)
else()
	target_compile_options(xvidcore PRIVATE $<$<COMPILE_LANGUAGE:C>:-Wall -Wextra -Wno-unused-function>)
endif()

# ---------------------------------------------
# Output names
# ---------------------------------------------
if(MSVC)
	set_target_properties(xvidcore PROPERTIES
		OUTPUT_NAME "xvidcore"
		PREFIX ""
	)
endif()

# ---------------------------------------------
# Install
# ---------------------------------------------
include(GNUInstallDirs)

install(TARGETS xvidcore
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES ${CMAKE_CURRENT_LIST_DIR}/src/xvidcore/src/xvid.h
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
