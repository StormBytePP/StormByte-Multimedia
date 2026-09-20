# Optimizations
#
# Windows: StormByte-Multimedia is built with clang-cl because C++26
# #embed is not usable as a portable MSVC-only path for tessdata.
# FFmpeg and ffmpeg-plugins stay TOOLCHAIN=msvc (Meson + the plugin
# graph). Mixing clang-cl -flto / lld-link with MSVC /GL archives
# fails at link ("not a native COFF file. Recompile without /GL").
# There is no LTO across those two triples, so IPO is off for every
# Windows configuration. Unix keeps Release IPO.
#
# /arch:AVX2 and /fp:fast stay on the library target. Putting them
# in CMAKE_C_FLAGS / CMAKE_CXX_FLAGS leaks into bundled zimg, which
# uses throw and NAN; clang-cl then fails with exceptions disabled
# and -Wnan-infinity-disabled.

if(WIN32 OR APPLE)
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE FALSE)
else()
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
endif()
