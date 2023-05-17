#pragma once

#include <Foundation/SimdMath/SimdQuat.h>

class WD_FOUNDATION_DLL wdSimdTransform
{
public:
  WD_DECLARE_POD_TYPE();

  /// \brief Default constructor: Does not do any initialization.
  wdSimdTransform(); // [tested]

  /// \brief Sets position, rotation and scale.
  explicit wdSimdTransform(const wdSimdVec4f& vPosition, const wdSimdQuat& qRotation = wdSimdQuat::IdentityQuaternion(),
    const wdSimdVec4f& vScale = wdSimdVec4f(1.0f)); // [tested]

  /// \brief Sets rotation.
  explicit wdSimdTransform(const wdSimdQuat& qRotation); // [tested]

  /// \brief Sets the position to be zero and the rotation to identity.
  void SetIdentity(); // [tested]

  /// \brief Returns an Identity Transform.
  static wdSimdTransform IdentityTransform(); // [tested]

  /// \brief Returns the scale component with maximum magnitude.
  wdSimdFloat GetMaxScale() const; // [tested]

  /// \brief Returns whether this transform contains negative scaling aka mirroring.
  bool ContainsNegativeScale() const;

  /// \brief Returns whether this transform contains uniform scaling.
  bool ContainsUniformScale() const;

public:
  /// \brief Equality Check with epsilon
  bool IsEqual(const wdSimdTransform& rhs, const wdSimdFloat& fEpsilon) const; // [tested]

public:
  /// \brief Inverts this transform.
  void Invert(); // [tested]

  /// \brief Returns the inverse of this transform.
  wdSimdTransform GetInverse() const; // [tested]

public:
  /// \brief Sets this transform to be the local transformation needed to get from the parent's transform to the child's.
  void SetLocalTransform(const wdSimdTransform& globalTransformParent, const wdSimdTransform& globalTransformChild); // [tested]

  /// \brief Sets this transform to the global transform, that is reached by applying the child's local transform to the parent's global
  /// one.
  void SetGlobalTransform(const wdSimdTransform& globalTransformParent, const wdSimdTransform& localTransformChild); // [tested]

  /// \brief Returns the transformation as a matrix.
  wdSimdMat4f GetAsMat4() const; // [tested]

public:
  wdSimdVec4f TransformPosition(const wdSimdVec4f& v) const;  // [tested]
  wdSimdVec4f TransformDirection(const wdSimdVec4f& v) const; // [tested]

  /// \brief Concatenates the two transforms. This is the same as a matrix multiplication, thus not commutative.
  void operator*=(const wdSimdTransform& other); // [tested]

  /// \brief Multiplies \a q into the rotation component, thus rotating the entire transformation.
  void operator*=(const wdSimdQuat& q); // [tested]

  void operator+=(const wdSimdVec4f& v); // [tested]
  void operator-=(const wdSimdVec4f& v); // [tested]

public:
  wdSimdVec4f m_Position;
  wdSimdQuat m_Rotation;
  wdSimdVec4f m_Scale;
};

// *** free functions ***

/// \brief Transforms the vector v by the transform.
WD_ALWAYS_INLINE const wdSimdVec4f operator*(const wdSimdTransform& t, const wdSimdVec4f& v); // [tested]

/// \brief Rotates the transform by the given quaternion. Multiplies q from the left with t.
WD_ALWAYS_INLINE const wdSimdTransform operator*(const wdSimdQuat& q, const wdSimdTransform& t); // [tested]

/// \brief Rotates the transform by the given quaternion. Multiplies q from the right with t.
WD_ALWAYS_INLINE const wdSimdTransform operator*(const wdSimdTransform& t, const wdSimdQuat& q); // [tested]

/// \brief Translates the wdSimdTransform by the vector. This will move the object in global space.
WD_ALWAYS_INLINE const wdSimdTransform operator+(const wdSimdTransform& t, const wdSimdVec4f& v); // [tested]

/// \brief Translates the wdSimdTransform by the vector. This will move the object in global space.
WD_ALWAYS_INLINE const wdSimdTransform operator-(const wdSimdTransform& t, const wdSimdVec4f& v); // [tested]

/// \brief Concatenates the two transforms. This is the same as a matrix multiplication, thus not commutative.
WD_ALWAYS_INLINE const wdSimdTransform operator*(const wdSimdTransform& lhs, const wdSimdTransform& rhs); // [tested]

WD_ALWAYS_INLINE bool operator==(const wdSimdTransform& t1, const wdSimdTransform& t2); // [tested]
WD_ALWAYS_INLINE bool operator!=(const wdSimdTransform& t1, const wdSimdTransform& t2); // [tested]


#include <Foundation/SimdMath/Implementation/SimdTransform_inl.h>
