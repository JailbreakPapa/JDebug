#pragma once

#define WD_SSE_20 0x20
#define WD_SSE_30 0x30
#define WD_SSE_31 0x31
#define WD_SSE_41 0x41
#define WD_SSE_42 0x42
#define WD_SSE_AVX 0x50
#define WD_SSE_AVX2 0x51

#define WD_SSE_LEVEL WD_SSE_41

#if WD_SSE_LEVEL >= WD_SSE_20
#  include <emmintrin.h>
#endif

#if WD_SSE_LEVEL >= WD_SSE_30
#  include <pmmintrin.h>
#endif

#if WD_SSE_LEVEL >= WD_SSE_31
#  include <tmmintrin.h>
#endif

#if WD_SSE_LEVEL >= WD_SSE_41
#  include <smmintrin.h>
#endif

#if WD_SSE_LEVEL >= WD_SSE_42
#  include <nmmintrin.h>
#endif

#if WD_SSE_LEVEL >= WD_SSE_AVX
#  include <immintrin.h>
#endif

#if WD_ENABLED(WD_COMPILE_FOR_DEBUG)
#  define WD_CHECK_SIMD_ALIGNMENT WD_CHECK_ALIGNMENT_16
#else
#  define WD_CHECK_SIMD_ALIGNMENT(x)
#endif

namespace wdInternal
{
  typedef __m128 QuadFloat;
  typedef __m128 QuadBool;
  typedef __m128i QuadInt;
  typedef __m128i QuadUInt;
} // namespace wdInternal

#include <Foundation/SimdMath/SimdSwizzle.h>

#define WD_SHUFFLE(a0, a1, b2, b3) ((a0) | ((a1) << 2) | ((b2) << 4) | ((b3) << 6))

#define WD_TO_SHUFFLE(s) (((s >> 12) & 0x03) | ((s >> 6) & 0x0c) | (s & 0x30) | ((s << 6) & 0xc0))
