#pragma once

#include <Foundation/Math/Angle.h>
#include <Foundation/Math/Math.h>
#include <Foundation/Math/Vec3.h>
#include <Foundation/Math/Vec4.h>

/// \brief A 4x4 component matrix class.
template <typename Type>
class nsMat4Template
{
public:
  NS_DECLARE_POD_TYPE();

  using ComponentType = Type;

  // *** Data ***
public:
  // The elements are stored in column-major order.
  // That means first is column 0 (with elements of row 0, row 1, row 2, row 3),
  // then column 1, then column 2 and finally column 3

  /// \brief The matrix as a 16-element Type array (column-major)
  Type m_fElementsCM[16];

  NS_ALWAYS_INLINE Type& Element(nsInt32 iColumn, nsInt32 iRow) { return m_fElementsCM[iColumn * 4 + iRow]; }
  NS_ALWAYS_INLINE Type Element(nsInt32 iColumn, nsInt32 iRow) const { return m_fElementsCM[iColumn * 4 + iRow]; }

  // *** Constructors ***
public:
  /// \brief Default Constructor DOES NOT INITIALIZE the matrix, at all.
  nsMat4Template(); // [tested]

  /// \brief Copies 16 values from pData into the matrix. Can handle the data in row-major or column-major order.
  ///
  /// \param pData
  ///   The array of Type values from which to set the matrix data.
  /// \param layout
  ///   The layout in which pData stores the matrix. The data will get transposed, if necessary.
  ///   The data should be in column-major format, if you want to prevent unnecessary transposes.
  nsMat4Template(const Type* const pData, nsMatrixLayout::Enum layout); // [tested]

  /// \brief Sets each element manually: Naming is "column-n row-m"
  nsMat4Template(Type c1r1, Type c2r1, Type c3r1, Type c4r1, Type c1r2, Type c2r2, Type c3r2, Type c4r2, Type c1r3, Type c2r3, Type c3r3, Type c4r3,
    Type c1r4, Type c2r4, Type c3r4, Type c4r4); // [tested]

  /// \brief Creates a transformation matrix from a rotation and a translation.
  nsMat4Template(const nsMat3Template<Type>& mRotation, const nsVec3Template<Type>& vTranslation); // [tested]

#if NS_ENABLED(NS_MATH_CHECK_FOR_NAN)
  void AssertNotNaN() const
  {
    NS_ASSERT_ALWAYS(!IsNaN(), "This object contains NaN values. This can happen when you forgot to initialize it before using it. Please "
                               "check that all code-paths properly initialize this object.");
  }
#endif

  /// \brief Returns a zero matrix.
  [[nodiscard]] static nsMat4Template<Type> MakeZero();

  /// \brief Returns an identity matrix.
  [[nodiscard]] static nsMat4Template<Type> MakeIdentity();

  /// \brief Creates a matrix from 16 values that are in row-major layout.
  [[nodiscard]] static nsMat4Template<Type> MakeFromRowMajorArray(const Type* const pData);

  /// \brief Creates a matrix from 16 values that are in column-major layout.
  [[nodiscard]] static nsMat4Template<Type> MakeFromColumnMajorArray(const Type* const pData);

  /// \brief Creates a matrix from 16 values. Naming is "column-n row-m"
  [[nodiscard]] static nsMat4Template<Type> MakeFromValues(Type c1r1, Type c2r1, Type c3r1, Type c4r1, Type c1r2, Type c2r2, Type c3r2, Type c4r2, Type c1r3, Type c2r3, Type c3r3, Type c4r3, Type c1r4, Type c2r4, Type c3r4, Type c4r4);

  /// \brief Creates a matrix with all zero values, except the last column, which is set to x, y, z, 1
  [[nodiscard]] static nsMat4Template<Type> MakeTranslation(const nsVec3Template<Type>& vTranslation);

  /// \brief Creates a transformation matrix from a rotation and a translation.
  [[nodiscard]] static nsMat4Template<Type> MakeTransformation(const nsMat3Template<Type>& mRotation, const nsVec3Template<Type>& vTranslation);

  /// \brief Creates a matrix with all zero values, except along the diagonal, which is set to x, y, z, 1
  [[nodiscard]] static nsMat4Template<Type> MakeScaling(const nsVec3Template<Type>& vScale);

  /// \brief Creates a matrix that is a rotation matrix around the X-axis.
  [[nodiscard]] static nsMat4Template<Type> MakeRotationX(nsAngle angle);

  /// \brief Creates a matrix that is a rotation matrix around the Y-axis.
  [[nodiscard]] static nsMat4Template<Type> MakeRotationY(nsAngle angle);

  /// \brief Creates a matrix that is a rotation matrix around the Z-axis.
  [[nodiscard]] static nsMat4Template<Type> MakeRotationZ(nsAngle angle);

  /// \brief Creates a matrix that is a rotation matrix around the given axis.
  [[nodiscard]] static nsMat4Template<Type> MakeAxisRotation(const nsVec3Template<Type>& vAxis, nsAngle angle);

  /// \brief Copies the 16 values of this matrix into the given array. 'layout' defines whether the data should end up in column-major or
  /// row-major format.
  void GetAsArray(Type* out_pData, nsMatrixLayout::Enum layout) const; // [tested]

  /// \brief Sets a transformation matrix from a rotation and a translation.
  void SetTransformationMatrix(const nsMat3Template<Type>& mRotation, const nsVec3Template<Type>& vTranslation); // [tested]

  // *** Special matrix constructors ***
public:
  /// \brief Sets all elements to zero.
  void SetZero(); // [tested]

  /// \brief Sets all elements to zero, except the diagonal, which is set to one.
  void SetIdentity(); // [tested]

  // *** Common Matrix Operations ***
public:
  /// \brief Transposes this matrix.
  void Transpose(); // [tested]

  /// \brief Returns the transpose of this matrix.
  const nsMat4Template<Type> GetTranspose() const; // [tested]

  /// \brief Inverts this matrix. Return value indicates whether the matrix could be inverted.
  nsResult Invert(Type fEpsilon = nsMath::SmallEpsilon<Type>()); // [tested]

  /// \brief Returns the inverse of this matrix.
  const nsMat4Template<Type> GetInverse(Type fEpsilon = nsMath::SmallEpsilon<Type>()) const; // [tested]

  // *** Checks ***
public:
  /// \brief Checks whether all elements are zero.
  bool IsZero(Type fEpsilon = nsMath::DefaultEpsilon<Type>()) const; // [tested]

  /// \brief Checks whether this is an identity matrix.
  bool IsIdentity(Type fEpsilon = nsMath::DefaultEpsilon<Type>()) const; // [tested]

  /// \brief Checks whether all components are finite numbers.
  bool IsValid() const; // [tested]

  /// \brief Checks whether any component is NaN.
  bool IsNaN() const; // [tested]

  // *** Special Accessors ***
public:
  /// \brief Returns all 4 components of the i-th row.
  nsVec4Template<Type> GetRow(nsUInt32 uiRow) const; // [tested]

  /// \brief Sets all 4 components of the i-th row.
  void SetRow(nsUInt32 uiRow, const nsVec4Template<Type>& vRow); // [tested]

  /// \brief Returns all 4 components of the i-th column.
  nsVec4Template<Type> GetColumn(nsUInt32 uiColumn) const; // [tested]

  /// \brief Sets all 4 components of the i-th column.
  void SetColumn(nsUInt32 uiColumn, const nsVec4Template<Type>& vColumn); // [tested]

  /// \brief Returns all 4 components on the diagonal of the matrix.
  nsVec4Template<Type> GetDiagonal() const; // [tested]

  /// \brief Sets all 4 components on the diagonal of the matrix.
  void SetDiagonal(const nsVec4Template<Type>& vDiag); // [tested]

  /// \brief Returns the first 3 components of the last column.
  const nsVec3Template<Type> GetTranslationVector() const; // [tested]

  /// \brief Sets the first 3 components of the last column.
  void SetTranslationVector(const nsVec3Template<Type>& v); // [tested]

  /// \brief Sets the 3x3 rotational part of the matrix.
  void SetRotationalPart(const nsMat3Template<Type>& mRotation); // [tested]

  /// \brief Returns the 3x3 rotational and scaling part of the matrix.
  const nsMat3Template<Type> GetRotationalPart() const; // [tested]

  /// \brief Returns the 3 scaling factors that are encoded in the matrix.
  const nsVec3Template<Type> GetScalingFactors() const; // [tested]

  /// \brief Tries to set the three scaling factors in the matrix. Returns NS_FAILURE if the matrix columns cannot be normalized and thus no
  /// rescaling is possible.
  nsResult SetScalingFactors(const nsVec3Template<Type>& vXYZ, Type fEpsilon = nsMath::DefaultEpsilon<Type>()); // [tested]

  // *** Operators ***
public:
  /// \brief Matrix-vector multiplication, assuming the 4th component of the vector is one (default behavior).
  const nsVec3Template<Type> TransformPosition(const nsVec3Template<Type>& v) const; // [tested]

  /// \brief Matrix-vector multiplication, assuming the 4th component of the vector is one (default behavior).
  void TransformPosition(nsVec3Template<Type>* pV, nsUInt32 uiNumVectors, nsUInt32 uiStride = sizeof(nsVec3Template<Type>)) const; // [tested]

  /// \brief Matrix-vector multiplication, assuming the 4th component of the vector is zero. So, rotation/scaling only. Useful as an
  /// optimization.
  const nsVec3Template<Type> TransformDirection(const nsVec3Template<Type>& v) const; // [tested]

  /// \brief Matrix-vector multiplication, assuming the 4th component of the vector is zero. So, rotation/scaling only. Useful as an
  /// optimization.
  void TransformDirection(nsVec3Template<Type>* pV, nsUInt32 uiNumVectors, nsUInt32 uiStride = sizeof(nsVec3Template<Type>)) const; // [tested]

  /// \brief Matrix-vector multiplication.
  const nsVec4Template<Type> Transform(const nsVec4Template<Type>& v) const; // [tested]

  /// \brief Matrix-vector multiplication.
  void Transform(nsVec4Template<Type>* pV, nsUInt32 uiNumVectors, nsUInt32 uiStride = sizeof(nsVec4Template<Type>)) const; // [tested]

  /// \brief Component-wise multiplication (commutative)
  void operator*=(Type f); // [tested]

  /// \brief Component-wise division
  void operator/=(Type f); // [tested]

  /// \brief Equality Check
  bool IsIdentical(const nsMat4Template<Type>& rhs) const; // [tested]

  /// \brief Equality Check with epsilon
  bool IsEqual(const nsMat4Template<Type>& rhs, Type fEpsilon) const; // [tested]
};

// *** free functions ***

/// \brief Matrix-Matrix multiplication
template <typename Type>
const nsMat4Template<Type> operator*(const nsMat4Template<Type>& m1, const nsMat4Template<Type>& m2); // [tested]

/// \brief Matrix-vector multiplication
template <typename Type>
const nsVec3Template<Type> operator*(const nsMat4Template<Type>& m, const nsVec3Template<Type>& v); // [tested]

/// \brief Matrix-vector multiplication
template <typename Type>
const nsVec4Template<Type> operator*(const nsMat4Template<Type>& m, const nsVec4Template<Type>& v); // [tested]

/// \brief Component-wise multiplication (commutative)
template <typename Type>
const nsMat4Template<Type> operator*(const nsMat4Template<Type>& m1, Type f); // [tested]

/// \brief Component-wise multiplication (commutative)
template <typename Type>
const nsMat4Template<Type> operator*(Type f, const nsMat4Template<Type>& m1); // [tested]

/// \brief Component-wise division
template <typename Type>
const nsMat4Template<Type> operator/(const nsMat4Template<Type>& m1, Type f); // [tested]

/// \brief Adding two matrices (component-wise)
template <typename Type>
const nsMat4Template<Type> operator+(const nsMat4Template<Type>& m1, const nsMat4Template<Type>& m2); // [tested]

/// \brief Subtracting two matrices (component-wise)
template <typename Type>
const nsMat4Template<Type> operator-(const nsMat4Template<Type>& m1, const nsMat4Template<Type>& m2); // [tested]

/// \brief Comparison Operator ==
template <typename Type>
bool operator==(const nsMat4Template<Type>& lhs, const nsMat4Template<Type>& rhs); // [tested]

/// \brief Comparison Operator !=
template <typename Type>
bool operator!=(const nsMat4Template<Type>& lhs, const nsMat4Template<Type>& rhs); // [tested]

#include <Foundation/Math/Implementation/Mat4_inl.h>
