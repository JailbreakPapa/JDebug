#pragma once

#include <Foundation/Math/Angle.h>
#include <Foundation/SimdMath/SimdTypes.h>

class NS_FOUNDATION_DLL nsSimdFloat
{
public:
  NS_DECLARE_POD_TYPE();

  /// \brief Default constructor, leaves the data uninitialized.
  nsSimdFloat(); // [tested]

  /// \brief Constructs from a given float.
  nsSimdFloat(float f); // [tested]

  /// \brief Constructs from a given integer.
  nsSimdFloat(nsInt32 i); // [tested]

  /// \brief Constructs from a given integer.
  nsSimdFloat(nsUInt32 i); // [tested]

  /// \brief Constructs from given angle.
  nsSimdFloat(nsAngle a); // [tested]

  /// \brief Constructs from the internal implementation type.
  nsSimdFloat(nsInternal::QuadFloat v); // [tested]

  /// \brief Returns the stored number as a standard float.
  operator float() const; // [tested]

  /// \brief Creates an nsSimdFloat that is initialized to zero.
  [[nodiscard]] static nsSimdFloat MakeZero(); // [tested]

  /// \brief Creates an nsSimdFloat that is initialized to Not-A-Number (NaN).
  [[nodiscard]] static nsSimdFloat MakeNaN();        // [tested]

public:
  nsSimdFloat operator+(const nsSimdFloat& f) const; // [tested]
  nsSimdFloat operator-(const nsSimdFloat& f) const; // [tested]
  nsSimdFloat operator*(const nsSimdFloat& f) const; // [tested]
  nsSimdFloat operator/(const nsSimdFloat& f) const; // [tested]

  nsSimdFloat& operator+=(const nsSimdFloat& f);     // [tested]
  nsSimdFloat& operator-=(const nsSimdFloat& f);     // [tested]
  nsSimdFloat& operator*=(const nsSimdFloat& f);     // [tested]
  nsSimdFloat& operator/=(const nsSimdFloat& f);     // [tested]

  bool IsEqual(const nsSimdFloat& rhs, const nsSimdFloat& fEpsilon) const;

  bool operator==(const nsSimdFloat& f) const;               // [tested]
  bool operator!=(const nsSimdFloat& f) const;               // [tested]
  bool operator>(const nsSimdFloat& f) const;                // [tested]
  bool operator>=(const nsSimdFloat& f) const;               // [tested]
  bool operator<(const nsSimdFloat& f) const;                // [tested]
  bool operator<=(const nsSimdFloat& f) const;               // [tested]

  bool operator==(float f) const;                            // [tested]
  bool operator!=(float f) const;                            // [tested]
  bool operator>(float f) const;                             // [tested]
  bool operator>=(float f) const;                            // [tested]
  bool operator<(float f) const;                             // [tested]
  bool operator<=(float f) const;                            // [tested]

  template <nsMathAcc::Enum acc = nsMathAcc::FULL>
  nsSimdFloat GetReciprocal() const;                         // [tested]

  template <nsMathAcc::Enum acc = nsMathAcc::FULL>
  nsSimdFloat GetSqrt() const;                               // [tested]

  template <nsMathAcc::Enum acc = nsMathAcc::FULL>
  nsSimdFloat GetInvSqrt() const;                            // [tested]

  [[nodiscard]] nsSimdFloat Max(const nsSimdFloat& f) const; // [tested]
  [[nodiscard]] nsSimdFloat Min(const nsSimdFloat& f) const; // [tested]
  [[nodiscard]] nsSimdFloat Abs() const;                     // [tested]

public:
  nsInternal::QuadFloat m_v;
};

#if NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_SSE
#  include <Foundation/SimdMath/Implementation/SSE/SSEFloat_inl.h>
#elif NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_FPU
#  include <Foundation/SimdMath/Implementation/FPU/FPUFloat_inl.h>
#elif NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_NEON
#  include <Foundation/SimdMath/Implementation/NEON/NEONFloat_inl.h>
#else
#  error "Unknown SIMD implementation."
#endif
