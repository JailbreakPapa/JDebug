#pragma once

#include <Foundation/SimdMath/SimdSwizzle.h>
#include <Foundation/SimdMath/SimdTypes.h>

class NS_FOUNDATION_DLL nsSimdVec4b
{
public:
  NS_DECLARE_POD_TYPE();

  nsSimdVec4b();                               // [tested]
  nsSimdVec4b(bool b);                         // [tested]
  nsSimdVec4b(bool x, bool y, bool z, bool w); // [tested]
  nsSimdVec4b(nsInternal::QuadBool b);         // [tested]

public:
  template <int N>
  bool GetComponent() const;                                                                               // [tested]

  bool x() const;                                                                                          // [tested]
  bool y() const;                                                                                          // [tested]
  bool z() const;                                                                                          // [tested]
  bool w() const;                                                                                          // [tested]

  template <nsSwizzle::Enum s>
  nsSimdVec4b Get() const;                                                                                 // [tested]

public:
  nsSimdVec4b operator&&(const nsSimdVec4b& rhs) const;                                                    // [tested]
  nsSimdVec4b operator||(const nsSimdVec4b& rhs) const;                                                    // [tested]
  nsSimdVec4b operator!() const;                                                                           // [tested]

  nsSimdVec4b operator==(const nsSimdVec4b& rhs) const;                                                    // [tested]
  nsSimdVec4b operator!=(const nsSimdVec4b& rhs) const;                                                    // [tested]

  template <int N = 4>
  bool AllSet() const;                                                                                     // [tested]

  template <int N = 4>
  bool AnySet() const;                                                                                     // [tested]

  template <int N = 4>
  bool NoneSet() const;                                                                                    // [tested]

  static nsSimdVec4b Select(const nsSimdVec4b& vCmp, const nsSimdVec4b& vTrue, const nsSimdVec4b& vFalse); // [tested]

public:
  nsInternal::QuadBool m_v;
};

#if NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_SSE
#  include <Foundation/SimdMath/Implementation/SSE/SSEVec4b_inl.h>
#elif NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_FPU
#  include <Foundation/SimdMath/Implementation/FPU/FPUVec4b_inl.h>
#elif NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_NEON
#  include <Foundation/SimdMath/Implementation/NEON/NEONVec4b_inl.h>
#else
#  error "Unknown SIMD implementation."
#endif
