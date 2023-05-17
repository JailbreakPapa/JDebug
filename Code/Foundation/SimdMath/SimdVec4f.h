#pragma once

#include <Foundation/SimdMath/SimdFloat.h>
#include <Foundation/SimdMath/SimdVec4b.h>

/// \brief A 4-component SIMD vector class
class WD_FOUNDATION_DLL wdSimdVec4f
{
public:
  WD_DECLARE_POD_TYPE();

  wdSimdVec4f(); // [tested]

  explicit wdSimdVec4f(float fXyzw); // [tested]

  explicit wdSimdVec4f(const wdSimdFloat& fXyzw); // [tested]

  wdSimdVec4f(float x, float y, float z, float w = 1.0f); // [tested]

  wdSimdVec4f(wdInternal::QuadFloat v); // [tested]

  void Set(float fXyzw); // [tested]

  void Set(float x, float y, float z, float w); // [tested]

  void SetX(const wdSimdFloat& f); // [tested]
  void SetY(const wdSimdFloat& f); // [tested]
  void SetZ(const wdSimdFloat& f); // [tested]
  void SetW(const wdSimdFloat& f); // [tested]

  void SetZero(); // [tested]

  template <int N>
  void Load(const float* pFloats); // [tested]

  template <int N>
  void Store(float* pFloats) const; // [tested]

public:
  template <wdMathAcc::Enum acc = wdMathAcc::FULL>
  wdSimdVec4f GetReciprocal() const; // [tested]

  template <wdMathAcc::Enum acc = wdMathAcc::FULL>
  wdSimdVec4f GetSqrt() const; // [tested]

  template <wdMathAcc::Enum acc = wdMathAcc::FULL>
  wdSimdVec4f GetInvSqrt() const; // [tested]

  template <int N, wdMathAcc::Enum acc = wdMathAcc::FULL>
  wdSimdFloat GetLength() const; // [tested]

  template <int N, wdMathAcc::Enum acc = wdMathAcc::FULL>
  wdSimdFloat GetInvLength() const; // [tested]

  template <int N>
  wdSimdFloat GetLengthSquared() const; // [tested]

  template <int N, wdMathAcc::Enum acc = wdMathAcc::FULL>
  wdSimdFloat GetLengthAndNormalize(); // [tested]

  template <int N, wdMathAcc::Enum acc = wdMathAcc::FULL>
  wdSimdVec4f GetNormalized() const; // [tested]

  template <int N, wdMathAcc::Enum acc = wdMathAcc::FULL>
  void Normalize(); // [tested]

  template <int N, wdMathAcc::Enum acc = wdMathAcc::FULL>
  void NormalizeIfNotZero(const wdSimdFloat& fEpsilon = wdMath::SmallEpsilon<float>()); // [tested]

  template <int N>
  bool IsZero() const; // [tested]

  template <int N>
  bool IsZero(const wdSimdFloat& fEpsilon) const; // [tested]

  template <int N>
  bool IsNormalized(const wdSimdFloat& fEpsilon = wdMath::HugeEpsilon<float>()) const; // [tested]

  template <int N>
  bool IsNaN() const; // [tested]

  template <int N>
  bool IsValid() const; // [tested]

public:
  template <int N>
  wdSimdFloat GetComponent() const; // [tested]

  wdSimdFloat GetComponent(int i) const; // [tested]

  wdSimdFloat x() const; // [tested]
  wdSimdFloat y() const; // [tested]
  wdSimdFloat z() const; // [tested]
  wdSimdFloat w() const; // [tested]

  template <wdSwizzle::Enum s>
  wdSimdVec4f Get() const; // [tested]

  ///\brief x = this[s0], y = this[s1], z = other[s2], w = other[s3]
  template <wdSwizzle::Enum s>
  wdSimdVec4f GetCombined(const wdSimdVec4f& other) const; // [tested]

public:
  wdSimdVec4f operator-() const;                     // [tested]
  wdSimdVec4f operator+(const wdSimdVec4f& v) const; // [tested]
  wdSimdVec4f operator-(const wdSimdVec4f& v) const; // [tested]

  wdSimdVec4f operator*(const wdSimdFloat& f) const; // [tested]
  wdSimdVec4f operator/(const wdSimdFloat& f) const; // [tested]

  wdSimdVec4f CompMul(const wdSimdVec4f& v) const; // [tested]

  template <wdMathAcc::Enum acc = wdMathAcc::FULL>
  wdSimdVec4f CompDiv(const wdSimdVec4f& v) const; // [tested]

  wdSimdVec4f CompMin(const wdSimdVec4f& rhs) const; // [tested]
  wdSimdVec4f CompMax(const wdSimdVec4f& rhs) const; // [tested]

  wdSimdVec4f Abs() const;      // [tested]
  wdSimdVec4f Round() const;    // [tested]
  wdSimdVec4f Floor() const;    // [tested]
  wdSimdVec4f Ceil() const;     // [tested]
  wdSimdVec4f Trunc() const;    // [tested]
  wdSimdVec4f Fraction() const; // [tested]

  wdSimdVec4f FlipSign(const wdSimdVec4b& vCmp) const; // [tested]

  static wdSimdVec4f Select(const wdSimdVec4b& vCmp, const wdSimdVec4f& vTrue, const wdSimdVec4f& vFalse); // [tested]

  static wdSimdVec4f Lerp(const wdSimdVec4f& a, const wdSimdVec4f& b, const wdSimdVec4f& t);

  wdSimdVec4f& operator+=(const wdSimdVec4f& v); // [tested]
  wdSimdVec4f& operator-=(const wdSimdVec4f& v); // [tested]

  wdSimdVec4f& operator*=(const wdSimdFloat& f); // [tested]
  wdSimdVec4f& operator/=(const wdSimdFloat& f); // [tested]

  wdSimdVec4b IsEqual(const wdSimdVec4f& rhs, const wdSimdFloat& fEpsilon) const; // [tested]

  wdSimdVec4b operator==(const wdSimdVec4f& v) const; // [tested]
  wdSimdVec4b operator!=(const wdSimdVec4f& v) const; // [tested]
  wdSimdVec4b operator<=(const wdSimdVec4f& v) const; // [tested]
  wdSimdVec4b operator<(const wdSimdVec4f& v) const;  // [tested]
  wdSimdVec4b operator>=(const wdSimdVec4f& v) const; // [tested]
  wdSimdVec4b operator>(const wdSimdVec4f& v) const;  // [tested]

  template <int N>
  wdSimdFloat HorizontalSum() const; // [tested]

  template <int N>
  wdSimdFloat HorizontalMin() const; // [tested]

  template <int N>
  wdSimdFloat HorizontalMax() const; // [tested]

  template <int N>
  wdSimdFloat Dot(const wdSimdVec4f& v) const; // [tested]

  ///\brief 3D cross product, w is ignored.
  wdSimdVec4f CrossRH(const wdSimdVec4f& v) const; // [tested]

  ///\brief Generates an arbitrary vector such that Dot<3>(GetOrthogonalVector()) == 0
  wdSimdVec4f GetOrthogonalVector() const; // [tested]

  static wdSimdVec4f ZeroVector(); // [tested]

  static wdSimdVec4f MulAdd(const wdSimdVec4f& a, const wdSimdVec4f& b, const wdSimdVec4f& c); // [tested]
  static wdSimdVec4f MulAdd(const wdSimdVec4f& a, const wdSimdFloat& b, const wdSimdVec4f& c); // [tested]

  static wdSimdVec4f MulSub(const wdSimdVec4f& a, const wdSimdVec4f& b, const wdSimdVec4f& c); // [tested]
  static wdSimdVec4f MulSub(const wdSimdVec4f& a, const wdSimdFloat& b, const wdSimdVec4f& c); // [tested]

  static wdSimdVec4f CopySign(const wdSimdVec4f& vMagnitude, const wdSimdVec4f& vSign); // [tested]

public:
  wdInternal::QuadFloat m_v;
};

#include <Foundation/SimdMath/Implementation/SimdVec4f_inl.h>

#if WD_SIMD_IMPLEMENTATION == WD_SIMD_IMPLEMENTATION_SSE
#  include <Foundation/SimdMath/Implementation/SSE/SSEVec4f_inl.h>
#elif WD_SIMD_IMPLEMENTATION == WD_SIMD_IMPLEMENTATION_FPU
#  include <Foundation/SimdMath/Implementation/FPU/FPUVec4f_inl.h>
#else
#  error "Unknown SIMD implementation."
#endif
