#pragma once

#include <Foundation/SimdMath/SimdVec4i.h>

struct wdSimdMath
{
  static wdSimdVec4f Exp(const wdSimdVec4f& f);
  static wdSimdVec4f Ln(const wdSimdVec4f& f);
  static wdSimdVec4f Log2(const wdSimdVec4f& f);
  static wdSimdVec4i Log2i(const wdSimdVec4i& i);
  static wdSimdVec4f Log10(const wdSimdVec4f& f);
  static wdSimdVec4f Pow2(const wdSimdVec4f& f);

  static wdSimdVec4f Sin(const wdSimdVec4f& f);
  static wdSimdVec4f Cos(const wdSimdVec4f& f);
  static wdSimdVec4f Tan(const wdSimdVec4f& f);

  static wdSimdVec4f ASin(const wdSimdVec4f& f);
  static wdSimdVec4f ACos(const wdSimdVec4f& f);
  static wdSimdVec4f ATan(const wdSimdVec4f& f);
};

#include <Foundation/SimdMath/Implementation/SimdMath_inl.h>
