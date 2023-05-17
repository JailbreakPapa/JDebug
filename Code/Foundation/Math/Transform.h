#pragma once

#include <Foundation/Math/Mat3.h>
#include <Foundation/Math/Mat4.h>
#include <Foundation/Math/Quat.h>
#include <Foundation/Math/Vec3.h>

/// \todo Fix docs and unit tests

/// \brief A class that represents position, rotation and scaling via a position vector, a quaternion and a scale vector.
///
/// Scale is applied first, then rotation and finally translation is added. Thus scale and rotation are always in 'local space',
/// i.e. applying a rotation to the wdTransformTemplate will rotate objects in place around their local center.
/// Since the translation is added afterwards, the translation component is always the global center position, around which
/// objects rotate.
///
/// The functions SetLocalTransform() and SetGlobalTransform() allow to create transforms that either represent the full
/// global transformation of an object, factoring its parent's transform in, or the local transformation that will get you
/// from the parent's global transformation to the current global transformation of a child (i.e. only the difference).
/// This is particularly useful when editing entities in a hierarchical structure.
///
/// This representation cannot handle shearing, which means rotations and scalings cannot be combined correctly.
/// Many parts of game engine cannot handle shearing or non-uniform scaling across hierarchies anyway. Therefore this
/// class implements a simplified way of combining scalings when multiplying two wdTransform's. Instead of rotating scale into
/// the proper space, the two values are simply multiplied component-wise.
///
/// In situations where this is insufficient, use a 3x3 or 4x4 matrix instead. Sometimes it is sufficient to use the matrix for
/// the computation and the result can be stored in a transform again.
template <typename Type>
class wdTransformTemplate
{
public:
  WD_DECLARE_POD_TYPE();

  // *** Data ***
  wdVec3Template<Type> m_vPosition;
  wdQuatTemplate<Type> m_qRotation;
  wdVec3Template<Type> m_vScale;

  // *** Constructors ***
public:
  /// \brief Default constructor: Does not do any initialization.
  wdTransformTemplate(){}; // [tested]

  /// \brief Sets position and rotation.
  explicit wdTransformTemplate(const wdVec3Template<Type>& vPosition,
    const wdQuatTemplate<Type>& qRotation = wdQuatTemplate<Type>::IdentityQuaternion(),
    const wdVec3Template<Type>& vScale = wdVec3Template<Type>(1)); // [tested]

  /// \brief Attempts to extract position, scale and rotation from the matrix. Negative scaling and shearing will get lost in the process.
  void SetFromMat4(const wdMat4Template<Type>& mMat);

  /// \brief Sets the position to be zero and the rotation to identity.
  void SetIdentity(); // [tested]

  /// \brief Returns an Identity Transform.
  static const wdTransformTemplate<Type> IdentityTransform();

  /// \brief Returns the scale component with maximum magnitude.
  Type GetMaxScale() const;

  /// \brief Returns whether this transform contains negative scaling aka mirroring.
  bool ContainsNegativeScale() const;

  /// \brief Returns whether this transform contains uniform scaling.
  bool ContainsUniformScale() const;

  // *** Equality ***
public:
  /// \brief Equality Check (bitwise)
  bool IsIdentical(const wdTransformTemplate& rhs) const; // [tested]

  /// \brief Equality Check with epsilon
  bool IsEqual(const wdTransformTemplate& rhs, Type fEpsilon) const; // [tested]

  // *** Inverse ***
public:
  /// \brief Inverts this transform.
  void Invert(); // [tested]

  /// \brief Returns the inverse of this transform.
  const wdTransformTemplate GetInverse() const; // [tested]

  wdVec3Template<Type> TransformPosition(const wdVec3Template<Type>& v) const;  // [tested]
  wdVec3Template<Type> TransformDirection(const wdVec3Template<Type>& v) const; // [tested]

  void operator+=(const wdVec3Template<Type>& v); // [tested]
  void operator-=(const wdVec3Template<Type>& v); // [tested]

  // *** Conversion operations ***
public:
  /// \brief Sets this transform to be the local transformation needed to get from the parent's transform to the child's.
  void SetLocalTransform(const wdTransformTemplate& globalTransformParent, const wdTransformTemplate& globalTransformChild); // [tested]

  /// \brief Sets this transform to the global transform, that is reached by applying the child's local transform to the parent's global one.
  void SetGlobalTransform(const wdTransformTemplate& globalTransformParent, const wdTransformTemplate& localTransformChild); // [tested]

  /// \brief Returns the transformation as a matrix.
  const wdMat4Template<Type> GetAsMat4() const; // [tested]
};

// *** free functions ***

/// \brief Transforms the vector v by the transform.
template <typename Type>
const wdVec3Template<Type> operator*(const wdTransformTemplate<Type>& t, const wdVec3Template<Type>& v); // [tested]

/// \brief Rotates the transform by the given quaternion. Multiplies q from the left with t.
template <typename Type>
const wdTransformTemplate<Type> operator*(const wdQuatTemplate<Type>& q, const wdTransformTemplate<Type>& t); // [tested]

/// \brief Rotates the transform by the given quaternion. Multiplies q from the right with t.
template <typename Type>
const wdTransformTemplate<Type> operator*(const wdTransformTemplate<Type>& t, const wdQuatTemplate<Type>& q);

/// \brief Translates the wdTransform by the vector. This will move the object in global space.
template <typename Type>
const wdTransformTemplate<Type> operator+(const wdTransformTemplate<Type>& t, const wdVec3Template<Type>& v); // [tested]

/// \brief Translates the wdTransform by the vector. This will move the object in global space.
template <typename Type>
const wdTransformTemplate<Type> operator-(const wdTransformTemplate<Type>& t, const wdVec3Template<Type>& v); // [tested]

/// \brief Concatenates the two transforms. This is the same as a matrix multiplication, thus not commutative.
template <typename Type>
const wdTransformTemplate<Type> operator*(const wdTransformTemplate<Type>& t1, const wdTransformTemplate<Type>& t2); // [tested]

template <typename Type>
bool operator==(const wdTransformTemplate<Type>& t1, const wdTransformTemplate<Type>& t2); // [tested]

template <typename Type>
bool operator!=(const wdTransformTemplate<Type>& t1, const wdTransformTemplate<Type>& t2); // [tested]

#include <Foundation/Math/Implementation/Transform_inl.h>
