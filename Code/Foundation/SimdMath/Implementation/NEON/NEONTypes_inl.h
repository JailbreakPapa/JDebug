#pragma once

#include <arm_neon.h>

#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
#  define NS_CHECK_SIMD_ALIGNMENT NS_CHECK_ALIGNMENT_16
#else
#  define NS_CHECK_SIMD_ALIGNMENT(x)
#endif

namespace nsInternal
{
  using QuadFloat = float32x4_t;
  using QuadBool = uint32x4_t;
  using QuadInt = int32x4_t;
  using QuadUInt = uint32x4_t;

  // Neon equivalent of _mm_movemask_ps
  NS_ALWAYS_INLINE uint32_t NeonMoveMask(uint32x4_t x)
  {
    // Isolate the sign bit of each vector element and shift it into its position in the final mask, the horizontally add to combine each element mask.
    alignas(16) static const int32_t shift[4] = {0, 1, 2, 3};
    return vaddvq_u32(vshlq_u32(vshrq_n_u32(x, 31), vld1q_s32(shift)));
  }

} // namespace nsInternal

// Converts an nsSwizzle into the mask selection format of __builtin_shufflevector
#define NS_TO_SHUFFLE(s) (s >> 12) & 3, (s >> 8) & 3, ((s >> 4) & 0x3) + 4, (s & 3) + 4
