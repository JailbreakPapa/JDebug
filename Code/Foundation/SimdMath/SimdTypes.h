#pragma once

#include <Foundation/Math/Math.h>

struct nsMathAcc
{
  enum Enum
  {
    FULL,
    BITS_23,
    BITS_12
  };
};

#if NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_SSE
#  include <Foundation/SimdMath/Implementation/SSE/SSETypes_inl.h>
#elif NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_FPU
#  include <Foundation/SimdMath/Implementation/FPU/FPUTypes_inl.h>
#elif NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_NEON
#  include <Foundation/SimdMath/Implementation/NEON/NEONTypes_inl.h>
#else
#  error "Unknown SIMD implementation."
#endif
