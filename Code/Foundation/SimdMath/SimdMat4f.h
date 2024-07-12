#pragma once

#include <Foundation/SimdMath/SimdVec4f.h>

/// \brief A 4x4 matrix class
class NS_FOUNDATION_DLL nsSimdMat4f
{
public:
  NS_DECLARE_POD_TYPE();

  nsSimdMat4f();

  /// \brief Returns a zero matrix.
  [[nodiscard]] static nsSimdMat4f MakeZero();

  /// \brief Returns an identity matrix.
  [[nodiscard]] static nsSimdMat4f MakeIdentity();

  /// \brief Creates a matrix from 16 values that are in row-major layout.
  [[nodiscard]] static nsSimdMat4f MakeFromRowMajorArray(const float* const pData);

  /// \brief Creates a matrix from 16 values that are in column-major layout.
  [[nodiscard]] static nsSimdMat4f MakeFromColumnMajorArray(const float* const pData);

  /// \brief Creates a matrix from 4 column vectors.
  [[nodiscard]] static nsSimdMat4f MakeFromColumns(const nsSimdVec4f& vCol0, const nsSimdVec4f& vCol1, const nsSimdVec4f& vCol2, const nsSimdVec4f& vCol3);

  /// \brief Creates a matrix from 16 values. Naming is "column-n row-m"
  [[nodiscard]] static nsSimdMat4f MakeFromValues(float f1r1, float f2r1, float f3r1, float f4r1, float f1r2, float f2r2, float f3r2, float f4r2, float f1r3, float f2r3, float f3r3, float f4r3, float f1r4, float f2r4, float f3r4, float f4r4);

  void GetAsArray(float* out_pData, nsMatrixLayout::Enum layout) const; // [tested]

public:
  /// \brief Transposes this matrix.
  void Transpose(); // [tested]

  /// \brief Returns the transpose of this matrix.
  nsSimdMat4f GetTranspose() const; // [tested]

  /// \brief Inverts this matrix. Return value indicates whether the matrix could be inverted.
  nsResult Invert(const nsSimdFloat& fEpsilon = nsMath::SmallEpsilon<float>()); // [tested]

  /// \brief Returns the inverse of this matrix.
  nsSimdMat4f GetInverse(const nsSimdFloat& fEpsilon = nsMath::SmallEpsilon<float>()) const; // [tested]

public:
  /// \brief Equality Check with epsilon
  bool IsEqual(const nsSimdMat4f& rhs, const nsSimdFloat& fEpsilon) const; // [tested]

  /// \brief Checks whether this is an identity matrix.
  bool IsIdentity(const nsSimdFloat& fEpsilon = nsMath::DefaultEpsilon<float>()) const; // [tested]

  /// \brief Checks whether all components are finite numbers.
  bool IsValid() const; // [tested]

  /// \brief Checks whether any component is NaN.
  bool IsNaN() const;                                                                                                   // [tested]

public:
  void SetRows(const nsSimdVec4f& vRow0, const nsSimdVec4f& vRow1, const nsSimdVec4f& vRow2, const nsSimdVec4f& vRow3); // [tested]
  void GetRows(nsSimdVec4f& ref_vRow0, nsSimdVec4f& ref_vRow1, nsSimdVec4f& ref_vRow2, nsSimdVec4f& ref_vRow3) const;   // [tested]

public:
  /// \brief Matrix-vector multiplication, assuming the 4th component of the vector is one (default behavior).
  [[nodiscard]] nsSimdVec4f TransformPosition(const nsSimdVec4f& v) const; // [tested]

  /// \brief Matrix-vector multiplication, assuming the 4th component of the vector is zero. So, rotation/scaling only.
  [[nodiscard]] nsSimdVec4f TransformDirection(const nsSimdVec4f& v) const; // [tested]

  [[nodiscard]] nsSimdMat4f operator*(const nsSimdMat4f& rhs) const;        // [tested]
  void operator*=(const nsSimdMat4f& rhs);

  [[nodiscard]] bool operator==(const nsSimdMat4f& rhs) const;              // [tested]
  [[nodiscard]] bool operator!=(const nsSimdMat4f& rhs) const;              // [tested]

public:
  nsSimdVec4f m_col0;
  nsSimdVec4f m_col1;
  nsSimdVec4f m_col2;
  nsSimdVec4f m_col3;
};

#include <Foundation/SimdMath/Implementation/SimdMat4f_inl.h>

#if NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_SSE
#  include <Foundation/SimdMath/Implementation/SSE/SSEMat4f_inl.h>
#elif NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_FPU
#  include <Foundation/SimdMath/Implementation/FPU/FPUMat4f_inl.h>
#elif NS_SIMD_IMPLEMENTATION == NS_SIMD_IMPLEMENTATION_NEON
#  include <Foundation/SimdMath/Implementation/NEON/NEONMat4f_inl.h>
#else
#  error "Unknown SIMD implementation."
#endif
