#pragma once

#include <algorithm>

namespace nsMath
{
  NS_ALWAYS_INLINE bool IsFinite(float value)
  {
    // Check the 8 exponent bits.
    // NAN -> (exponent = all 1, mantissa = non-zero)
    // INF -> (exponent = all 1, mantissa = zero)

    nsIntFloatUnion i2f(value);
    return ((i2f.i & 0x7f800000u) != 0x7f800000u);
  }

  NS_ALWAYS_INLINE bool IsNaN(float value)
  {
    // Check the 8 exponent bits.
    // NAN -> (exponent = all 1, mantissa = non-zero)
    // INF -> (exponent = all 1, mantissa = zero)

    nsIntFloatUnion i2f(value);
    return (((i2f.i & 0x7f800000u) == 0x7f800000u) && ((i2f.i & 0x7FFFFFu) != 0));
  }

  NS_ALWAYS_INLINE float Floor(float f)
  {
    return floorf(f);
  }

  NS_ALWAYS_INLINE float Ceil(float f)
  {
    return ceilf(f);
  }

  NS_ALWAYS_INLINE float Round(float f)
  {
    return Floor(f + 0.5f);
  }

  NS_ALWAYS_INLINE float RoundToMultiple(float f, float fMultiple)
  {
    return Round(f / fMultiple) * fMultiple;
  }


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

  NS_ALWAYS_INLINE float Sin(nsAngle a)
  {
    return sinf(a.GetRadian());
  }

  NS_ALWAYS_INLINE float Cos(nsAngle a)
  {
    return cosf(a.GetRadian());
  }

  NS_ALWAYS_INLINE float Tan(nsAngle a)
  {
    return tanf(a.GetRadian());
  }

  NS_ALWAYS_INLINE nsAngle ASin(float f)
  {
    return nsAngle::MakeFromRadian(asinf(f));
  }

  NS_ALWAYS_INLINE nsAngle ACos(float f)
  {
    return nsAngle::MakeFromRadian(acosf(f));
  }

  NS_ALWAYS_INLINE nsAngle ATan(float f)
  {
    return nsAngle::MakeFromRadian(atanf(f));
  }

  NS_ALWAYS_INLINE nsAngle ATan2(float y, float x)
  {
    return nsAngle::MakeFromRadian(atan2f(y, x));
  }

  NS_ALWAYS_INLINE float Exp(float f)
  {
    return expf(f);
  }

  NS_ALWAYS_INLINE float Ln(float f)
  {
    return logf(f);
  }

  NS_ALWAYS_INLINE float Log2(float f)
  {
    return log2f(f);
  }

  NS_ALWAYS_INLINE float Log10(float f)
  {
    return log10f(f);
  }

  NS_ALWAYS_INLINE float Log(float fBase, float f)
  {
    return log10f(f) / log10f(fBase);
  }

  NS_ALWAYS_INLINE float Pow2(float f)
  {
    return exp2f(f);
  }

  NS_ALWAYS_INLINE float Pow(float fBase, float fExp)
  {
    return powf(fBase, fExp);
  }

  NS_ALWAYS_INLINE float Root(float f, float fNthRoot)
  {
    return powf(f, 1.0f / fNthRoot);
  }

  NS_ALWAYS_INLINE float Sqrt(float f)
  {
    return sqrtf(f);
  }

  NS_ALWAYS_INLINE float Mod(float f, float fDiv)
  {
    return fmodf(f, fDiv);
  }
} // namespace nsMath
