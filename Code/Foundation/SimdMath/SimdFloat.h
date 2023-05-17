#pragma once

#include <Foundation/Math/Angle.h>
#include <Foundation/SimdMath/SimdTypes.h>

class WD_FOUNDATION_DLL wdSimdFloat
{
public:
  WD_DECLARE_POD_TYPE();

  /// \brief Default constructor, leaves the data uninitialized.
  wdSimdFloat(); // [tested]

  /// \brief Constructs from a given float.
  wdSimdFloat(float f); // [tested]

  /// \brief Constructs from a given integer.
  wdSimdFloat(wdInt32 i); // [tested]

  /// \brief Constructs from a given integer.
  wdSimdFloat(wdUInt32 i); // [tested]

  /// \brief Constructs from given angle.
  wdSimdFloat(wdAngle a); // [tested]

  /// \brief Constructs from the internal implementation type.
  wdSimdFloat(wdInternal::QuadFloat v); // [tested]

  /// \brief Returns the stored number as a standard float.
  operator float() const; // [tested]

  static wdSimdFloat Zero(); // [tested]

public:
  wdSimdFloat operator+(const wdSimdFloat& f) const; // [tested]
  wdSimdFloat operator-(const wdSimdFloat& f) const; // [tested]
  wdSimdFloat operator*(const wdSimdFloat& f) const; // [tested]
  wdSimdFloat operator/(const wdSimdFloat& f) const; // [tested]

  wdSimdFloat& operator+=(const wdSimdFloat& f); // [tested]
  wdSimdFloat& operator-=(const wdSimdFloat& f); // [tested]
  wdSimdFloat& operator*=(const wdSimdFloat& f); // [tested]
  wdSimdFloat& operator/=(const wdSimdFloat& f); // [tested]

  bool IsEqual(const wdSimdFloat& rhs, const wdSimdFloat& fEpsilon) const;

  bool operator==(const wdSimdFloat& f) const; // [tested]
  bool operator!=(const wdSimdFloat& f) const; // [tested]
  bool operator>(const wdSimdFloat& f) const;  // [tested]
  bool operator>=(const wdSimdFloat& f) const; // [tested]
  bool operator<(const wdSimdFloat& f) const;  // [tested]
  bool operator<=(const wdSimdFloat& f) const; // [tested]

  bool operator==(float f) const; // [tested]
  bool operator!=(float f) const; // [tested]
  bool operator>(float f) const;  // [tested]
  bool operator>=(float f) const; // [tested]
  bool operator<(float f) const;  // [tested]
  bool operator<=(float f) const; // [tested]

  template <wdMathAcc::Enum acc = wdMathAcc::FULL>
  wdSimdFloat GetReciprocal() const; // [tested]

  template <wdMathAcc::Enum acc = wdMathAcc::FULL>
  wdSimdFloat GetSqrt() const; // [tested]

  template <wdMathAcc::Enum acc = wdMathAcc::FULL>
  wdSimdFloat GetInvSqrt() const; // [tested]

  wdSimdFloat Max(const wdSimdFloat& f) const; // [tested]
  wdSimdFloat Min(const wdSimdFloat& f) const; // [tested]
  wdSimdFloat Abs() const;                     // [tested]

public:
  wdInternal::QuadFloat m_v;
};

#if WD_SIMD_IMPLEMENTATION == WD_SIMD_IMPLEMENTATION_SSE
#  include <Foundation/SimdMath/Implementation/SSE/SSEFloat_inl.h>
#elif WD_SIMD_IMPLEMENTATION == WD_SIMD_IMPLEMENTATION_FPU
#  include <Foundation/SimdMath/Implementation/FPU/FPUFloat_inl.h>
#else
#  error "Unknown SIMD implementation."
#endif
