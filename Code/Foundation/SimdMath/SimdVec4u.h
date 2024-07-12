#pragma once

#include <Foundation/SimdMath/SimdVec4i.h>

/// \brief A SIMD 4-component vector class of unsigned 32b integers
class NS_FOUNDATION_DLL nsSimdVec4u
{
public:
  NS_DECLARE_POD_TYPE();

  nsSimdVec4u();                                                   // [tested]

  explicit nsSimdVec4u(nsUInt32 uiXyzw);                           // [tested]

  nsSimdVec4u(nsUInt32 x, nsUInt32 y, nsUInt32 z, nsUInt32 w = 1); // [tested]

  nsSimdVec4u(nsInternal::QuadUInt v);                             // [tested]

  /// \brief Creates an nsSimdVec4u that is initialized to zero.
  [[nodiscard]] static nsSimdVec4u MakeZero();                     // [tested]

  void Set(nsUInt32 uiXyzw);                                       // [tested]

  void Set(nsUInt32 x, nsUInt32 y, nsUInt32 z, nsUInt32 w);        // [tested]

  void SetZero();                                                  // [tested]

public:
  explicit nsSimdVec4u(const nsSimdVec4i& i);                      // [tested]

public:
  nsSimdVec4f ToFloat() const;                                     // [tested]

  [[nodiscard]] static nsSimdVec4u Truncate(const nsSimdVec4f& f); // [tested]

public:
  template <int N>
  nsUInt32 GetComponent() const;                                   // [tested]

  nsUInt32 x() const;                                              // [tested]
  nsUInt32 y() const;                                              // [tested]
  nsUInt32 z() const;                                              // [tested]
  nsUInt32 w() const;                                              // [tested]

  template <nsSwizzle::Enum s>
  nsSimdVec4u Get() const;                                         // [tested]

public:
  [[nodiscard]] nsSimdVec4u operator+(const nsSimdVec4u& v) const; // [tested]
  [[nodiscard]] nsSimdVec4u operator-(const nsSimdVec4u& v) const; // [tested]

  [[nodiscard]] nsSimdVec4u CompMul(const nsSimdVec4u& v) const;   // [tested]

  [[nodiscard]] nsSimdVec4u operator|(const nsSimdVec4u& v) const; // [tested]
  [[nodiscard]] nsSimdVec4u operator&(const nsSimdVec4u& v) const; // [tested]
  [[nodiscard]] nsSimdVec4u operator^(const nsSimdVec4u& v) const; // [tested]
  [[nodiscard]] nsSimdVec4u operator~() const;                     // [tested]

  [[nodiscard]] nsSimdVec4u operator<<(nsUInt32 uiShift) const;    // [tested]
  [[nodiscard]] nsSimdVec4u operator>>(nsUInt32 uiShift) const;    // [tested]

  nsSimdVec4u& operator+=(const nsSimdVec4u& v);                   // [tested]
  nsSimdVec4u& operator-=(const nsSimdVec4u& v);                   // [tested]

  nsSimdVec4u& operator|=(const nsSimdVec4u& v);                   // [tested]
  nsSimdVec4u& operator&=(const nsSimdVec4u& v);                   // [tested]
  nsSimdVec4u& operator^=(const nsSimdVec4u& v);                   // [tested]

  nsSimdVec4u& operator<<=(nsUInt32 uiShift);                      // [tested]
  nsSimdVec4u& operator>>=(nsUInt32 uiShift);                      // [tested]

  [[nodiscard]] nsSimdVec4u CompMin(const nsSimdVec4u& v) const;   // [tested]
  [[nodiscard]] nsSimdVec4u CompMax(const nsSimdVec4u& v) const;   // [tested]

  nsSimdVec4b operator==(const nsSimdVec4u& v) const;              // [tested]
  nsSimdVec4b operator!=(const nsSimdVec4u& v) const;              // [tested]
  nsSimdVec4b operator<=(const nsSimdVec4u& v) const;              // [tested]
  nsSimdVec4b operator<(const nsSimdVec4u& v) const;               // [tested]
  nsSimdVec4b operator>=(const nsSimdVec4u& v) const;              // [tested]
  nsSimdVec4b operator>(const nsSimdVec4u& v) const;               // [tested]

public:
  nsInternal::QuadUInt m_v;
};

#if NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_SSE
#  include <Foundation/SimdMath/Implementation/SSE/SSEVec4u_inl.h>
#elif NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_FPU
#  include <Foundation/SimdMath/Implementation/FPU/FPUVec4u_inl.h>
#elif NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_NEON
#  include <Foundation/SimdMath/Implementation/NEON/NEONVec4u_inl.h>
#else
#  error "Unknown SIMD implementation."
#endif
