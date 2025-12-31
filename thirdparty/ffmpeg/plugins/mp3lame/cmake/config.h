#ifndef LAME_CONFIG_H
#define LAME_CONFIG_H

/* --- Standard headers --- */
#define HAVE_STDINT_H 1
#define HAVE_STRING_H 1
#define HAVE_MEMCPY 1
#define HAVE_MEMSET 1
#define HAVE_STDLIB_H 1
#define HAVE_ERRNO_H 1

#include <string.h>
#include <stdint.h>

/* --- IEEE754 types (LAME expects these) --- */
typedef float        ieee754_float32_t;
typedef double       ieee754_float64_t;
typedef long double  ieee854_float80_t;

/* --- Integer fallback types (safe for all platforms) --- */
typedef int8_t   int8_t_fallback;
typedef int16_t  int16_t_fallback;
typedef int32_t  int32_t_fallback;
typedef int64_t  int64_t_fallback;

typedef uint8_t   uint8_t_fallback;
typedef uint16_t  uint16_t_fallback;
typedef uint32_t  uint32_t_fallback;
typedef uint64_t  uint64_t_fallback;

/* --- LAME internal flags --- */
#define HAVE_MPGLIB 0
#define DECODE_ONLY 0

/* --- SIMD --- */
#define HAVE_SSE 1
#define HAVE_SSE2 1

/* --- Platform headers --- */
#if defined(_WIN32) || defined(_WIN64)
    #define HAVE_WINDOWS_H 1
#else
    #define HAVE_UNISTD_H 1
#endif

/* --- Misc --- */
#define STDC_HEADERS 1

#endif /* LAME_CONFIG_H */
