#pragma once

#include <Foundation/SimdMath/SimdFloat.h>
#include <Foundation/SimdMath/SimdVec4b.h>

/// \brief A 4-component SIMD vector class
class NS_FOUNDATION_DLL nsSimdVec4f
{
public:
  NS_DECLARE_POD_TYPE();

  nsSimdVec4f();                                          // [tested]

  explicit nsSimdVec4f(float fXyzw);                      // [tested]

  explicit nsSimdVec4f(const nsSimdFloat& fXyzw);         // [tested]

  nsSimdVec4f(float x, float y, float z, float w = 1.0f); // [tested]

  nsSimdVec4f(nsInternal::QuadFloat v);                   // [tested]

  /// \brief Creates an nsSimdVec4f that is initialized to zero.
  [[nodiscard]] static nsSimdVec4f MakeZero(); // [tested]

  /// \brief Creates an nsSimdVec4f that is initialized to Not-A-Number (NaN).
  [[nodiscard]] static nsSimdVec4f MakeNaN();   // [tested]

  void Set(float fXyzw);                        // [tested]

  void Set(float x, float y, float z, float w); // [tested]

  void SetX(const nsSimdFloat& f);              // [tested]
  void SetY(const nsSimdFloat& f);              // [tested]
  void SetZ(const nsSimdFloat& f);              // [tested]
  void SetW(const nsSimdFloat& f);              // [tested]

  void SetZero();                               // [tested]

  template <int N>
  void Load(const float* pFloats);              // [tested]

  template <int N>
  void Store(float* pFloats) const;             // [tested]

public:
  template <nsMathAcc::Enum acc = nsMathAcc::FULL>
  nsSimdVec4f GetReciprocal() const;                                                    // [tested]

  template <nsMathAcc::Enum acc = nsMathAcc::FULL>
  nsSimdVec4f GetSqrt() const;                                                          // [tested]

  template <nsMathAcc::Enum acc = nsMathAcc::FULL>
  nsSimdVec4f GetInvSqrt() const;                                                       // [tested]

  template <int N, nsMathAcc::Enum acc = nsMathAcc::FULL>
  nsSimdFloat GetLength() const;                                                        // [tested]

  template <int N, nsMathAcc::Enum acc = nsMathAcc::FULL>
  nsSimdFloat GetInvLength() const;                                                     // [tested]

  template <int N>
  nsSimdFloat GetLengthSquared() const;                                                 // [tested]

  template <int N, nsMathAcc::Enum acc = nsMathAcc::FULL>
  nsSimdFloat GetLengthAndNormalize();                                                  // [tested]

  template <int N, nsMathAcc::Enum acc = nsMathAcc::FULL>
  nsSimdVec4f GetNormalized() const;                                                    // [tested]

  template <int N, nsMathAcc::Enum acc = nsMathAcc::FULL>
  void Normalize();                                                                     // [tested]

  template <int N, nsMathAcc::Enum acc = nsMathAcc::FULL>
  void NormalizeIfNotZero(const nsSimdFloat& fEpsilon = nsMath::SmallEpsilon<float>()); // [tested]

  template <int N>
  bool IsZero() const;                                                                  // [tested]

  template <int N>
  bool IsZero(const nsSimdFloat& fEpsilon) const;                                       // [tested]

  template <int N>
  bool IsNormalized(const nsSimdFloat& fEpsilon = nsMath::HugeEpsilon<float>()) const;  // [tested]

  template <int N>
  bool IsNaN() const;                                                                   // [tested]

  template <int N>
  bool IsValid() const;                                                                 // [tested]

public:
  template <int N>
  nsSimdFloat GetComponent() const;      // [tested]

  nsSimdFloat GetComponent(int i) const; // [tested]

  nsSimdFloat x() const;                 // [tested]
  nsSimdFloat y() const;                 // [tested]
  nsSimdFloat z() const;                 // [tested]
  nsSimdFloat w() const;                 // [tested]

  template <nsSwizzle::Enum s>
  nsSimdVec4f Get() const;               // [tested]

  ///\brief x = this[s0], y = this[s1], z = other[s2], w = other[s3]
  template <nsSwizzle::Enum s>
  [[nodiscard]] nsSimdVec4f GetCombined(const nsSimdVec4f& other) const;                                                 // [tested]

public:
  [[nodiscard]] nsSimdVec4f operator-() const;                                                                           // [tested]
  [[nodiscard]] nsSimdVec4f operator+(const nsSimdVec4f& v) const;                                                       // [tested]
  [[nodiscard]] nsSimdVec4f operator-(const nsSimdVec4f& v) const;                                                       // [tested]

  [[nodiscard]] nsSimdVec4f operator*(const nsSimdFloat& f) const;                                                       // [tested]
  [[nodiscard]] nsSimdVec4f operator/(const nsSimdFloat& f) const;                                                       // [tested]

  [[nodiscard]] nsSimdVec4f CompMul(const nsSimdVec4f& v) const;                                                         // [tested]

  template <nsMathAcc::Enum acc = nsMathAcc::FULL>
  [[nodiscard]] nsSimdVec4f CompDiv(const nsSimdVec4f& v) const;                                                         // [tested]

  [[nodiscard]] nsSimdVec4f CompMin(const nsSimdVec4f& rhs) const;                                                       // [tested]
  [[nodiscard]] nsSimdVec4f CompMax(const nsSimdVec4f& rhs) const;                                                       // [tested]

  [[nodiscard]] nsSimdVec4f Abs() const;                                                                                 // [tested]
  [[nodiscard]] nsSimdVec4f Round() const;                                                                               // [tested]
  [[nodiscard]] nsSimdVec4f Floor() const;                                                                               // [tested]
  [[nodiscard]] nsSimdVec4f Ceil() const;                                                                                // [tested]
  [[nodiscard]] nsSimdVec4f Trunc() const;                                                                               // [tested]
  [[nodiscard]] nsSimdVec4f Fraction() const;                                                                            // [tested]

  [[nodiscard]] nsSimdVec4f FlipSign(const nsSimdVec4b& vCmp) const;                                                     // [tested]

  [[nodiscard]] static nsSimdVec4f Select(const nsSimdVec4b& vCmp, const nsSimdVec4f& vTrue, const nsSimdVec4f& vFalse); // [tested]

  [[nodiscard]] static nsSimdVec4f Lerp(const nsSimdVec4f& a, const nsSimdVec4f& b, const nsSimdVec4f& t);

  nsSimdVec4f& operator+=(const nsSimdVec4f& v);                                  // [tested]
  nsSimdVec4f& operator-=(const nsSimdVec4f& v);                                  // [tested]

  nsSimdVec4f& operator*=(const nsSimdFloat& f);                                  // [tested]
  nsSimdVec4f& operator/=(const nsSimdFloat& f);                                  // [tested]

  nsSimdVec4b IsEqual(const nsSimdVec4f& rhs, const nsSimdFloat& fEpsilon) const; // [tested]

  [[nodiscard]] nsSimdVec4b operator==(const nsSimdVec4f& v) const;               // [tested]
  [[nodiscard]] nsSimdVec4b operator!=(const nsSimdVec4f& v) const;               // [tested]
  [[nodiscard]] nsSimdVec4b operator<=(const nsSimdVec4f& v) const;               // [tested]
  [[nodiscard]] nsSimdVec4b operator<(const nsSimdVec4f& v) const;                // [tested]
  [[nodiscard]] nsSimdVec4b operator>=(const nsSimdVec4f& v) const;               // [tested]
  [[nodiscard]] nsSimdVec4b operator>(const nsSimdVec4f& v) const;                // [tested]

  template <int N>
  [[nodiscard]] nsSimdFloat HorizontalSum() const;                                // [tested]

  template <int N>
  [[nodiscard]] nsSimdFloat HorizontalMin() const;                                // [tested]

  template <int N>
  [[nodiscard]] nsSimdFloat HorizontalMax() const;                                // [tested]

  template <int N>
  [[nodiscard]] nsSimdFloat Dot(const nsSimdVec4f& v) const;                      // [tested]

  ///\brief 3D cross product, w is ignored.
  [[nodiscard]] nsSimdVec4f CrossRH(const nsSimdVec4f& v) const; // [tested]

  ///\brief Generates an arbitrary vector such that Dot<3>(GetOrthogonalVector()) == 0
  [[nodiscard]] nsSimdVec4f GetOrthogonalVector() const;                                                     // [tested]

  [[nodiscard]] static nsSimdVec4f MulAdd(const nsSimdVec4f& a, const nsSimdVec4f& b, const nsSimdVec4f& c); // [tested]
  [[nodiscard]] static nsSimdVec4f MulAdd(const nsSimdVec4f& a, const nsSimdFloat& b, const nsSimdVec4f& c); // [tested]

  [[nodiscard]] static nsSimdVec4f MulSub(const nsSimdVec4f& a, const nsSimdVec4f& b, const nsSimdVec4f& c); // [tested]
  [[nodiscard]] static nsSimdVec4f MulSub(const nsSimdVec4f& a, const nsSimdFloat& b, const nsSimdVec4f& c); // [tested]

  [[nodiscard]] static nsSimdVec4f CopySign(const nsSimdVec4f& vMagnitude, const nsSimdVec4f& vSign);        // [tested]

public:
  nsInternal::QuadFloat m_v;
};

#include <Foundation/SimdMath/Implementation/SimdVec4f_inl.h>

#if NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_SSE
#  include <Foundation/SimdMath/Implementation/SSE/SSEVec4f_inl.h>
#elif NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_FPU
#  include <Foundation/SimdMath/Implementation/FPU/FPUVec4f_inl.h>
#elif NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_NEON
#  include <Foundation/SimdMath/Implementation/NEON/NEONVec4f_inl.h>
#else
#  error "Unknown SIMD implementation."
#endif
