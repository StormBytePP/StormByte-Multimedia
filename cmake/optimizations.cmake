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

if(WIN32 OR APPLE)
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE FALSE)
else()
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
endif()
