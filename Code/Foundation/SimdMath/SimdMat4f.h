#pragma once

#include <Foundation/SimdMath/SimdVec4f.h>

/// \brief A 4x4 matrix class
class WD_FOUNDATION_DLL wdSimdMat4f
{
public:
  WD_DECLARE_POD_TYPE();

  wdSimdMat4f();

  wdSimdMat4f(const float* const pData, wdMatrixLayout::Enum layout); // [tested]

  wdSimdMat4f(const wdSimdVec4f& vCol0, const wdSimdVec4f& vCol1, const wdSimdVec4f& vCol2, const wdSimdVec4f& vCol3); // [tested]

  /// \brief Sets each element manually: Naming is "column-n row-m"
  wdSimdMat4f(float f1r1, float f2r1, float f3r1, float f4r1, float f1r2, float f2r2, float f3r2, float f4r2, float f1r3, float f2r3, float f3r3,
    float f4r3, float f1r4, float f2r4, float f3r4, float f4r4); // [tested]

  void SetFromArray(const float* const pData, wdMatrixLayout::Enum layout); // [tested]

  void GetAsArray(float* out_pData, wdMatrixLayout::Enum layout) const; // [tested]

  /// \brief Sets all elements to zero, except the diagonal, which is set to one.
  void SetIdentity(); // [tested]

  /// \brief Returns an Identity Matrix.
  static wdSimdMat4f IdentityMatrix(); // [tested]

public:
  /// \brief Transposes this matrix.
  void Transpose(); // [tested]

  /// \brief Returns the transpose of this matrix.
  wdSimdMat4f GetTranspose() const; // [tested]

  /// \brief Inverts this matrix. Return value indicates whether the matrix could be inverted.
  wdResult Invert(const wdSimdFloat& fEpsilon = wdMath::SmallEpsilon<float>()); // [tested]

  /// \brief Returns the inverse of this matrix.
  wdSimdMat4f GetInverse(const wdSimdFloat& fEpsilon = wdMath::SmallEpsilon<float>()) const; // [tested]

public:
  /// \brief Equality Check with epsilon
  bool IsEqual(const wdSimdMat4f& rhs, const wdSimdFloat& fEpsilon) const; // [tested]

  /// \brief Checks whether this is an identity matrix.
  bool IsIdentity(const wdSimdFloat& fEpsilon = wdMath::DefaultEpsilon<float>()) const; // [tested]

  /// \brief Checks whether all components are finite numbers.
  bool IsValid() const; // [tested]

  /// \brief Checks whether any component is NaN.
  bool IsNaN() const; // [tested]

public:
  void SetRows(const wdSimdVec4f& vRow0, const wdSimdVec4f& vRow1, const wdSimdVec4f& vRow2, const wdSimdVec4f& vRow3); // [tested]
  void GetRows(wdSimdVec4f& ref_vRow0, wdSimdVec4f& ref_vRow1, wdSimdVec4f& ref_vRow2, wdSimdVec4f& ref_vRow3) const;   // [tested]

public:
  /// \brief Matrix-vector multiplication, assuming the 4th component of the vector is one (default behavior).
  wdSimdVec4f TransformPosition(const wdSimdVec4f& v) const; // [tested]

  /// \brief Matrix-vector multiplication, assuming the 4th component of the vector is zero. So, rotation/scaling only.
  wdSimdVec4f TransformDirection(const wdSimdVec4f& v) const; // [tested]

  wdSimdMat4f operator*(const wdSimdMat4f& rhs) const; // [tested]
  void operator*=(const wdSimdMat4f& rhs);

  bool operator==(const wdSimdMat4f& rhs) const; // [tested]
  bool operator!=(const wdSimdMat4f& rhs) const; // [tested]

public:
  wdSimdVec4f m_col0;
  wdSimdVec4f m_col1;
  wdSimdVec4f m_col2;
  wdSimdVec4f m_col3;
};

#include <Foundation/SimdMath/Implementation/SimdMat4f_inl.h>

#if WD_SIMD_IMPLEMENTATION == WD_SIMD_IMPLEMENTATION_SSE
#  include <Foundation/SimdMath/Implementation/SSE/SSEMat4f_inl.h>
#elif WD_SIMD_IMPLEMENTATION == WD_SIMD_IMPLEMENTATION_FPU
#  include <Foundation/SimdMath/Implementation/FPU/FPUMat4f_inl.h>
#else
#  error "Unknown SIMD implementation."
#endif
