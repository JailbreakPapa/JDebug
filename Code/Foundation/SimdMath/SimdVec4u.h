#pragma once

#include <Foundation/SimdMath/SimdVec4i.h>

/// \brief A SIMD 4-component vector class of unsigned 32b integers
class WD_FOUNDATION_DLL wdSimdVec4u
{
public:
  WD_DECLARE_POD_TYPE();

  wdSimdVec4u(); // [tested]

  explicit wdSimdVec4u(wdUInt32 uiXyzw); // [tested]

  wdSimdVec4u(wdUInt32 x, wdUInt32 y, wdUInt32 z, wdUInt32 w = 1); // [tested]

  wdSimdVec4u(wdInternal::QuadUInt v); // [tested]

  void Set(wdUInt32 uiXyzw); // [tested]

  void Set(wdUInt32 x, wdUInt32 y, wdUInt32 z, wdUInt32 w); // [tested]

  void SetZero(); // [tested]

public:
  explicit wdSimdVec4u(const wdSimdVec4i& i); // [tested]

public:
  wdSimdVec4f ToFloat() const; // [tested]

  static wdSimdVec4u Truncate(const wdSimdVec4f& f); // [tested]

public:
  template <int N>
  wdUInt32 GetComponent() const; // [tested]

  wdUInt32 x() const; // [tested]
  wdUInt32 y() const; // [tested]
  wdUInt32 z() const; // [tested]
  wdUInt32 w() const; // [tested]

  template <wdSwizzle::Enum s>
  wdSimdVec4u Get() const; // [tested]

public:
  wdSimdVec4u operator+(const wdSimdVec4u& v) const; // [tested]
  wdSimdVec4u operator-(const wdSimdVec4u& v) const; // [tested]

  wdSimdVec4u CompMul(const wdSimdVec4u& v) const; // [tested]

  wdSimdVec4u operator|(const wdSimdVec4u& v) const; // [tested]
  wdSimdVec4u operator&(const wdSimdVec4u& v) const; // [tested]
  wdSimdVec4u operator^(const wdSimdVec4u& v) const; // [tested]
  wdSimdVec4u operator~() const;                     // [tested]

  wdSimdVec4u operator<<(wdUInt32 uiShift) const; // [tested]
  wdSimdVec4u operator>>(wdUInt32 uiShift) const; // [tested]

  wdSimdVec4u& operator+=(const wdSimdVec4u& v); // [tested]
  wdSimdVec4u& operator-=(const wdSimdVec4u& v); // [tested]

  wdSimdVec4u& operator|=(const wdSimdVec4u& v); // [tested]
  wdSimdVec4u& operator&=(const wdSimdVec4u& v); // [tested]
  wdSimdVec4u& operator^=(const wdSimdVec4u& v); // [tested]

  wdSimdVec4u& operator<<=(wdUInt32 uiShift); // [tested]
  wdSimdVec4u& operator>>=(wdUInt32 uiShift); // [tested]

  wdSimdVec4u CompMin(const wdSimdVec4u& v) const; // [tested]
  wdSimdVec4u CompMax(const wdSimdVec4u& v) const; // [tested]

  wdSimdVec4b operator==(const wdSimdVec4u& v) const; // [tested]
  wdSimdVec4b operator!=(const wdSimdVec4u& v) const; // [tested]
  wdSimdVec4b operator<=(const wdSimdVec4u& v) const; // [tested]
  wdSimdVec4b operator<(const wdSimdVec4u& v) const;  // [tested]
  wdSimdVec4b operator>=(const wdSimdVec4u& v) const; // [tested]
  wdSimdVec4b operator>(const wdSimdVec4u& v) const;  // [tested]

  static wdSimdVec4u ZeroVector(); // [tested]

public:
  wdInternal::QuadUInt m_v;
};

#if WD_SIMD_IMPLEMENTATION == WD_SIMD_IMPLEMENTATION_SSE
#  include <Foundation/SimdMath/Implementation/SSE/SSEVec4u_inl.h>
#elif WD_SIMD_IMPLEMENTATION == WD_SIMD_IMPLEMENTATION_FPU
#  include <Foundation/SimdMath/Implementation/FPU/FPUVec4u_inl.h>
#else
#  error "Unknown SIMD implementation."
#endif
