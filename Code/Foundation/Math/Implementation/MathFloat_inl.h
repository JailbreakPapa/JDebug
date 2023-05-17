#pragma once

#include <algorithm>

namespace wdMath
{
  WD_ALWAYS_INLINE bool IsFinite(float value)
  {
    // Check the 8 exponent bits.
    // NAN -> (exponent = all 1, mantissa = non-zero)
    // INF -> (exponent = all 1, mantissa = zero)

    wdIntFloatUnion i2f(value);
    return ((i2f.i & 0x7f800000u) != 0x7f800000u);
  }

  WD_ALWAYS_INLINE bool IsNaN(float value)
  {
    // Check the 8 exponent bits.
    // NAN -> (exponent = all 1, mantissa = non-zero)
    // INF -> (exponent = all 1, mantissa = zero)

    wdIntFloatUnion i2f(value);
    return (((i2f.i & 0x7f800000u) == 0x7f800000u) && ((i2f.i & 0x7FFFFFu) != 0));
  }

  WD_ALWAYS_INLINE float Floor(float f) { return floorf(f); }

  WD_ALWAYS_INLINE float Ceil(float f) { return ceilf(f); }

  WD_ALWAYS_INLINE float Round(float f) { return Floor(f + 0.5f); }

  WD_ALWAYS_INLINE float RoundToMultiple(float f, float fMultiple) { return Round(f / fMultiple) * fMultiple; }


  inline float RoundDown(float f, float fMultiple)
  {
    float fDivides = f / fMultiple;
    float fFactor = Floor(fDivides);
    return fFactor * fMultiple;
  }

  inline float RoundUp(float f, float fMultiple)
  {
    float fDivides = f / fMultiple;
    float fFactor = Ceil(fDivides);
    return fFactor * fMultiple;
  }

  WD_ALWAYS_INLINE float Sin(wdAngle a) { return sinf(a.GetRadian()); }

  WD_ALWAYS_INLINE float Cos(wdAngle a) { return cosf(a.GetRadian()); }

  WD_ALWAYS_INLINE float Tan(wdAngle a) { return tanf(a.GetRadian()); }

  WD_ALWAYS_INLINE wdAngle ASin(float f) { return wdAngle::Radian(asinf(f)); }

  WD_ALWAYS_INLINE wdAngle ACos(float f) { return wdAngle::Radian(acosf(f)); }

  WD_ALWAYS_INLINE wdAngle ATan(float f) { return wdAngle::Radian(atanf(f)); }

  WD_ALWAYS_INLINE wdAngle ATan2(float y, float x) { return wdAngle::Radian(atan2f(y, x)); }

  WD_ALWAYS_INLINE float Exp(float f) { return expf(f); }

  WD_ALWAYS_INLINE float Ln(float f) { return logf(f); }

  WD_ALWAYS_INLINE float Log2(float f) { return log2f(f); }

  WD_ALWAYS_INLINE float Log10(float f) { return log10f(f); }

  WD_ALWAYS_INLINE float Log(float fBase, float f) { return log10f(f) / log10f(fBase); }

  WD_ALWAYS_INLINE float Pow2(float f) { return exp2f(f); }

  WD_ALWAYS_INLINE float Pow(float fBase, float fExp) { return powf(fBase, fExp); }

  WD_ALWAYS_INLINE float Root(float f, float fNthRoot) { return powf(f, 1.0f / fNthRoot); }

  WD_ALWAYS_INLINE float Sqrt(float f) { return sqrtf(f); }

  WD_ALWAYS_INLINE float Mod(float f, float fDiv) { return fmodf(f, fDiv); }
} // namespace wdMath
