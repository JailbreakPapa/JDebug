#pragma once

#define NS_SSE_20 0x20
#define NS_SSE_30 0x30
#define NS_SSE_31 0x31
#define NS_SSE_41 0x41
#define NS_SSE_42 0x42
#define NS_SSE_AVX 0x50
#define NS_SSE_AVX2 0x51

#define NS_SSE_LEVEL NS_SSE_41

#if NS_SSE_LEVEL >= NS_SSE_20
#  include <emmintrin.h>
#endif

#if NS_SSE_LEVEL >= NS_SSE_30
#  include <pmmintrin.h>
#endif

#if NS_SSE_LEVEL >= NS_SSE_31
#  include <tmmintrin.h>
#endif

#if NS_SSE_LEVEL >= NS_SSE_41
#  include <smmintrin.h>
#endif

#if NS_SSE_LEVEL >= NS_SSE_42
#  include <nmmintrin.h>
#endif

#if NS_SSE_LEVEL >= NS_SSE_AVX
#  include <immintrin.h>
#endif

#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
#  define NS_CHECK_SIMD_ALIGNMENT NS_CHECK_ALIGNMENT_16
#else
#  define NS_CHECK_SIMD_ALIGNMENT(x)
#endif

namespace nsInternal
{
  using QuadFloat = __m128;
  using QuadBool = __m128;
  using QuadInt = __m128i;
  using QuadUInt = __m128i;
} // namespace nsInternal

#include <Foundation/SimdMath/SimdSwizzle.h>

#define NS_SHUFFLE(a0, a1, b2, b3) ((a0) | ((a1) << 2) | ((b2) << 4) | ((b3) << 6))

#define NS_TO_SHUFFLE(s) ((((s) >> 12) & 0x03) | (((s) >> 6) & 0x0c) | ((s) & 0x30) | (((s) << 6) & 0xc0))
