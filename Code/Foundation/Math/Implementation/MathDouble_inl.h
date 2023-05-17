#pragma once

namespace wdMath
{
  WD_ALWAYS_INLINE bool IsFinite(double value)
  {
    // Check the 11 exponent bits.
    // NAN -> (exponent = all 1, mantissa = non-zero)
    // INF -> (exponent = all 1, mantissa = zero)

    wdInt64DoubleUnion i2f(value);
    return ((i2f.i & 0x7FF0000000000000ull) != 0x7FF0000000000000ull);
  }

  WD_ALWAYS_INLINE bool IsNaN(double value)
  {
    // Check the 11 exponent bits.
    // NAN -> (exponent = all 1, mantissa = non-zero)
    // INF -> (exponent = all 1, mantissa = zero)

    wdInt64DoubleUnion i2f(value);
    return (((i2f.i & 0x7FF0000000000000ull) == 0x7FF0000000000000ull) && ((i2f.i & 0xFFFFFFFFFFFFFull) != 0));
  }

  WD_ALWAYS_INLINE double Floor(double f) { return floor(f); }

  WD_ALWAYS_INLINE double Ceil(double f) { return ceil(f); }

  WD_ALWAYS_INLINE double Round(double f) { return Floor(f + 0.5f); }

  inline double RoundDown(double f, double fMultiple)
  {
    double fDivides = f / fMultiple;
    double fFactor = Floor(fDivides);
    return fFactor * fMultiple;
  }

  inline double RoundUp(double f, double fMultiple)
  {
    double fDivides = f / fMultiple;
    double fFactor = Ceil(fDivides);
    return fFactor * fMultiple;
  }

  WD_ALWAYS_INLINE double RoundToMultiple(double f, double fMultiple) { return Round(f / fMultiple) * fMultiple; }

  WD_ALWAYS_INLINE double Exp(double f) { return exp(f); }

  WD_ALWAYS_INLINE double Ln(double f) { return log(f); }

  WD_ALWAYS_INLINE double Log2(double f) { return log10(f) / log10(2.0); }

  WD_ALWAYS_INLINE double Log10(double f) { return log10(f); }

  WD_ALWAYS_INLINE double Log(double fBase, double f) { return log10(f) / log10(fBase); }

  WD_ALWAYS_INLINE double Pow2(double f) { return pow(2.0, f); }

  WD_ALWAYS_INLINE double Pow(double fBase, double fExp) { return pow(fBase, fExp); }

  WD_ALWAYS_INLINE double Root(double f, double fNthRoot) { return pow(f, 1.0 / fNthRoot); }

  WD_ALWAYS_INLINE double Sqrt(double f) { return sqrt(f); }

  WD_ALWAYS_INLINE double Mod(double f, double fDiv) { return fmod(f, fDiv); }
} // namespace wdMath
