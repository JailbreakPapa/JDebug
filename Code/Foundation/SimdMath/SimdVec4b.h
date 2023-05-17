#pragma once

#include <Foundation/SimdMath/SimdSwizzle.h>
#include <Foundation/SimdMath/SimdTypes.h>

class WD_FOUNDATION_DLL wdSimdVec4b
{
public:
  WD_DECLARE_POD_TYPE();

  wdSimdVec4b();                               // [tested]
  wdSimdVec4b(bool b);                         // [tested]
  wdSimdVec4b(bool x, bool y, bool z, bool w); // [tested]
  wdSimdVec4b(wdInternal::QuadBool b);         // [tested]

public:
  template <int N>
  bool GetComponent() const; // [tested]

  bool x() const; // [tested]
  bool y() const; // [tested]
  bool z() const; // [tested]
  bool w() const; // [tested]

  template <wdSwizzle::Enum s>
  wdSimdVec4b Get() const; // [tested]

public:
  wdSimdVec4b operator&&(const wdSimdVec4b& rhs) const; // [tested]
  wdSimdVec4b operator||(const wdSimdVec4b& rhs) const; // [tested]
  wdSimdVec4b operator!() const;                        // [tested]

  wdSimdVec4b operator==(const wdSimdVec4b& rhs) const; // [tested]
  wdSimdVec4b operator!=(const wdSimdVec4b& rhs) const; // [tested]

  template <int N = 4>
  bool AllSet() const; // [tested]

  template <int N = 4>
  bool AnySet() const; // [tested]

  template <int N = 4>
  bool NoneSet() const; // [tested]

  static wdSimdVec4b Select(const wdSimdVec4b& vCmp, const wdSimdVec4b& vTrue, const wdSimdVec4b& vFalse); // [tested]

public:
  wdInternal::QuadBool m_v;
};

#if WD_SIMD_IMPLEMENTATION == WD_SIMD_IMPLEMENTATION_SSE
#  include <Foundation/SimdMath/Implementation/SSE/SSEVec4b_inl.h>
#elif WD_SIMD_IMPLEMENTATION == WD_SIMD_IMPLEMENTATION_FPU
#  include <Foundation/SimdMath/Implementation/FPU/FPUVec4b_inl.h>
#else
#  error "Unknown SIMD implementation."
#endif
