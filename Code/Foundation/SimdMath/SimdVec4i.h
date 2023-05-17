#pragma once

#include <Foundation/SimdMath/SimdVec4f.h>

class wdSimdVec4u;

/// \brief A SIMD 4-component vector class of signed 32b integers
class WD_FOUNDATION_DLL wdSimdVec4i
{
public:
  WD_DECLARE_POD_TYPE();

  wdSimdVec4i(); // [tested]

  explicit wdSimdVec4i(wdInt32 iXyzw); // [tested]

  wdSimdVec4i(wdInt32 x, wdInt32 y, wdInt32 z, wdInt32 w = 1); // [tested]

  wdSimdVec4i(wdInternal::QuadInt v); // [tested]

  void Set(wdInt32 iXyzw); // [tested]

  void Set(wdInt32 x, wdInt32 y, wdInt32 z, wdInt32 w); // [tested]

  void SetZero(); // [tested]

  template <int N>
  void Load(const wdInt32* pInts); // [tested]

  template <int N>
  void Store(wdInt32* pInts) const; // [tested]

public:
  explicit wdSimdVec4i(const wdSimdVec4u& u); // [tested]

public:
  wdSimdVec4f ToFloat() const; // [tested]

  static wdSimdVec4i Truncate(const wdSimdVec4f& f); // [tested]

public:
  template <int N>
  wdInt32 GetComponent() const; // [tested]

  wdInt32 x() const; // [tested]
  wdInt32 y() const; // [tested]
  wdInt32 z() const; // [tested]
  wdInt32 w() const; // [tested]

  template <wdSwizzle::Enum s>
  wdSimdVec4i Get() const; // [tested]

public:
  wdSimdVec4i operator-() const;                     // [tested]
  wdSimdVec4i operator+(const wdSimdVec4i& v) const; // [tested]
  wdSimdVec4i operator-(const wdSimdVec4i& v) const; // [tested]

  wdSimdVec4i CompMul(const wdSimdVec4i& v) const; // [tested]
  wdSimdVec4i CompDiv(const wdSimdVec4i& v) const; // [tested]

  wdSimdVec4i operator|(const wdSimdVec4i& v) const; // [tested]
  wdSimdVec4i operator&(const wdSimdVec4i& v) const; // [tested]
  wdSimdVec4i operator^(const wdSimdVec4i& v) const; // [tested]
  wdSimdVec4i operator~() const;                     // [tested]

  wdSimdVec4i operator<<(wdUInt32 uiShift) const; // [tested]
  wdSimdVec4i operator>>(wdUInt32 uiShift) const; // [tested]
  wdSimdVec4i operator<<(const wdSimdVec4i& v) const; // [tested]
  wdSimdVec4i operator>>(const wdSimdVec4i& v) const; // [tested]

  wdSimdVec4i& operator+=(const wdSimdVec4i& v); // [tested]
  wdSimdVec4i& operator-=(const wdSimdVec4i& v); // [tested]

  wdSimdVec4i& operator|=(const wdSimdVec4i& v); // [tested]
  wdSimdVec4i& operator&=(const wdSimdVec4i& v); // [tested]
  wdSimdVec4i& operator^=(const wdSimdVec4i& v); // [tested]

  wdSimdVec4i& operator<<=(wdUInt32 uiShift); // [tested]
  wdSimdVec4i& operator>>=(wdUInt32 uiShift); // [tested]

  wdSimdVec4i CompMin(const wdSimdVec4i& v) const; // [tested]
  wdSimdVec4i CompMax(const wdSimdVec4i& v) const; // [tested]
  wdSimdVec4i Abs() const;                         // [tested]

  wdSimdVec4b operator==(const wdSimdVec4i& v) const; // [tested]
  wdSimdVec4b operator!=(const wdSimdVec4i& v) const; // [tested]
  wdSimdVec4b operator<=(const wdSimdVec4i& v) const; // [tested]
  wdSimdVec4b operator<(const wdSimdVec4i& v) const;  // [tested]
  wdSimdVec4b operator>=(const wdSimdVec4i& v) const; // [tested]
  wdSimdVec4b operator>(const wdSimdVec4i& v) const;  // [tested]

  static wdSimdVec4i ZeroVector(); // [tested]

  static wdSimdVec4i Select(const wdSimdVec4b& vCmp, const wdSimdVec4i& vTrue, const wdSimdVec4i& vFalse); // [tested]

public:
  wdInternal::QuadInt m_v;
};

#if WD_SIMD_IMPLEMENTATION == WD_SIMD_IMPLEMENTATION_SSE
#  include <Foundation/SimdMath/Implementation/SSE/SSEVec4i_inl.h>
#elif WD_SIMD_IMPLEMENTATION == WD_SIMD_IMPLEMENTATION_FPU
#  include <Foundation/SimdMath/Implementation/FPU/FPUVec4i_inl.h>
#else
#  error "Unknown SIMD implementation."
#endif
