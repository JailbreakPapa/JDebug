#pragma once

#include <Foundation/Math/Math.h>

struct wdMathAcc
{
  enum Enum
  {
    FULL,
    BITS_23,
    BITS_12
  };
};

#if WD_SIMD_IMPLEMENTATION == WD_SIMD_IMPLEMENTATION_SSE
#  include <Foundation/SimdMath/Implementation/SSE/SSETypes_inl.h>
#elif WD_SIMD_IMPLEMENTATION == WD_SIMD_IMPLEMENTATION_FPU
#  include <Foundation/SimdMath/Implementation/FPU/FPUTypes_inl.h>
#else
#  error "Unknown SIMD implementation."
#endif
