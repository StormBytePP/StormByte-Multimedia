# Optimizations
#
# Windows: StormByte-Multimedia is clang-cl (#embed). FFmpeg / plugins
# stay MSVC. No LTO across those triples.
#
# Win/mac Release pins ISA and -O2 on CMAKE_C_FLAGS / CMAKE_CXX_FLAGS
# so Meson and CMake children see them. CMAKE_*_FLAGS_RELEASE is not
# enough for bundled FFmpeg. Not /fp:fast and not /EHsc: those break
# zimg (NAN / throw). zimg enables EH on its own target. Linux leaves
# flags to Gentoo.
#
# x86: x86-64-v3 (AVX2+FMA+BMI). Apple Silicon: apple-m1 (runs on
# M1 through current M-series). -O3 is not used; it miscompiles
# codecs and color paths.

if(WIN32)
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE FALSE)
else()
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
endif()

if((WIN32 OR APPLE) AND CMAKE_BUILD_TYPE STREQUAL "Release")
	if(MSVC)
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2 /Ob2 /DNDEBUG")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Ob2 /DNDEBUG")
	else()
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2 -DNDEBUG")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -DNDEBUG")
	endif()
	if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64|x86)$")
		if(MSVC)
			set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /arch:AVX2")
			set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /arch:AVX2")
		else()
			set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -march=x86-64-v3")
			set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=x86-64-v3")
		endif()
	elseif(APPLE AND CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)$")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -mcpu=apple-m1")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mcpu=apple-m1")
	endif()
endif()
