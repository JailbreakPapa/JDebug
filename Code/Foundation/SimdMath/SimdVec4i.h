#pragma once

#include <Foundation/SimdMath/SimdVec4f.h>

class nsSimdVec4u;

/// \brief A SIMD 4-component vector class of signed 32b integers
class NS_FOUNDATION_DLL nsSimdVec4i
{
public:
  NS_DECLARE_POD_TYPE();

  nsSimdVec4i();                                               // [tested]

  explicit nsSimdVec4i(nsInt32 iXyzw);                         // [tested]

  nsSimdVec4i(nsInt32 x, nsInt32 y, nsInt32 z, nsInt32 w = 1); // [tested]

  nsSimdVec4i(nsInternal::QuadInt v);                          // [tested]

  /// \brief Creates an nsSimdVec4i that is initialized to zero.
  [[nodiscard]] static nsSimdVec4i MakeZero();                     // [tested]

  void Set(nsInt32 iXyzw);                                         // [tested]

  void Set(nsInt32 x, nsInt32 y, nsInt32 z, nsInt32 w);            // [tested]

  void SetZero();                                                  // [tested]

  template <int N>
  void Load(const nsInt32* pInts);                                 // [tested]

  template <int N>
  void Store(nsInt32* pInts) const;                                // [tested]

public:
  explicit nsSimdVec4i(const nsSimdVec4u& u);                      // [tested]

public:
  nsSimdVec4f ToFloat() const;                                     // [tested]

  [[nodiscard]] static nsSimdVec4i Truncate(const nsSimdVec4f& f); // [tested]

public:
  template <int N>
  nsInt32 GetComponent() const; // [tested]

  nsInt32 x() const;            // [tested]
  nsInt32 y() const;            // [tested]
  nsInt32 z() const;            // [tested]
  nsInt32 w() const;            // [tested]

  template <nsSwizzle::Enum s>
  nsSimdVec4i Get() const;      // [tested]

  ///\brief x = this[s0], y = this[s1], z = other[s2], w = other[s3]
  template <nsSwizzle::Enum s>
  [[nodiscard]] nsSimdVec4i GetCombined(const nsSimdVec4i& other) const;                                                 // [tested]

public:
  [[nodiscard]] nsSimdVec4i operator-() const;                                                                           // [tested]
  [[nodiscard]] nsSimdVec4i operator+(const nsSimdVec4i& v) const;                                                       // [tested]
  [[nodiscard]] nsSimdVec4i operator-(const nsSimdVec4i& v) const;                                                       // [tested]

  [[nodiscard]] nsSimdVec4i CompMul(const nsSimdVec4i& v) const;                                                         // [tested]
  [[nodiscard]] nsSimdVec4i CompDiv(const nsSimdVec4i& v) const;                                                         // [tested]

  [[nodiscard]] nsSimdVec4i operator|(const nsSimdVec4i& v) const;                                                       // [tested]
  [[nodiscard]] nsSimdVec4i operator&(const nsSimdVec4i& v) const;                                                       // [tested]
  [[nodiscard]] nsSimdVec4i operator^(const nsSimdVec4i& v) const;                                                       // [tested]
  [[nodiscard]] nsSimdVec4i operator~() const;                                                                           // [tested]

  [[nodiscard]] nsSimdVec4i operator<<(nsUInt32 uiShift) const;                                                          // [tested]
  [[nodiscard]] nsSimdVec4i operator>>(nsUInt32 uiShift) const;                                                          // [tested]
  [[nodiscard]] nsSimdVec4i operator<<(const nsSimdVec4i& v) const;                                                      // [tested]
  [[nodiscard]] nsSimdVec4i operator>>(const nsSimdVec4i& v) const;                                                      // [tested]

  nsSimdVec4i& operator+=(const nsSimdVec4i& v);                                                                         // [tested]
  nsSimdVec4i& operator-=(const nsSimdVec4i& v);                                                                         // [tested]

  nsSimdVec4i& operator|=(const nsSimdVec4i& v);                                                                         // [tested]
  nsSimdVec4i& operator&=(const nsSimdVec4i& v);                                                                         // [tested]
  nsSimdVec4i& operator^=(const nsSimdVec4i& v);                                                                         // [tested]

  nsSimdVec4i& operator<<=(nsUInt32 uiShift);                                                                            // [tested]
  nsSimdVec4i& operator>>=(nsUInt32 uiShift);                                                                            // [tested]

  [[nodiscard]] nsSimdVec4i CompMin(const nsSimdVec4i& v) const;                                                         // [tested]
  [[nodiscard]] nsSimdVec4i CompMax(const nsSimdVec4i& v) const;                                                         // [tested]
  [[nodiscard]] nsSimdVec4i Abs() const;                                                                                 // [tested]

  [[nodiscard]] nsSimdVec4b operator==(const nsSimdVec4i& v) const;                                                      // [tested]
  [[nodiscard]] nsSimdVec4b operator!=(const nsSimdVec4i& v) const;                                                      // [tested]
  [[nodiscard]] nsSimdVec4b operator<=(const nsSimdVec4i& v) const;                                                      // [tested]
  [[nodiscard]] nsSimdVec4b operator<(const nsSimdVec4i& v) const;                                                       // [tested]
  [[nodiscard]] nsSimdVec4b operator>=(const nsSimdVec4i& v) const;                                                      // [tested]
  [[nodiscard]] nsSimdVec4b operator>(const nsSimdVec4i& v) const;                                                       // [tested]

  [[nodiscard]] static nsSimdVec4i Select(const nsSimdVec4b& vCmp, const nsSimdVec4i& vTrue, const nsSimdVec4i& vFalse); // [tested]

public:
  nsInternal::QuadInt m_v;
};

#if NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_SSE
#  include <Foundation/SimdMath/Implementation/SSE/SSEVec4i_inl.h>
#elif NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_FPU
#  include <Foundation/SimdMath/Implementation/FPU/FPUVec4i_inl.h>
#elif NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_NEON
#  include <Foundation/SimdMath/Implementation/NEON/NEONVec4i_inl.h>
#else
#  error "Unknown SIMD implementation."
#endif
