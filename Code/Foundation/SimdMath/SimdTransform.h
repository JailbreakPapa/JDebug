#pragma once

#include <Foundation/SimdMath/SimdQuat.h>

class NS_FOUNDATION_DLL nsSimdTransform
{
public:
  NS_DECLARE_POD_TYPE();

  /// \brief Default constructor: Does not do any initialization.
  nsSimdTransform(); // [tested]

  /// \brief Sets position, rotation and scale.
  explicit nsSimdTransform(const nsSimdVec4f& vPosition, const nsSimdQuat& qRotation = nsSimdQuat::MakeIdentity(), const nsSimdVec4f& vScale = nsSimdVec4f(1.0f)); // [tested]

  /// \brief Sets rotation.
  explicit nsSimdTransform(const nsSimdQuat& qRotation); // [tested]

  /// \brief Creates a transform from the given position, rotation and scale.
  [[nodiscard]] static nsSimdTransform Make(const nsSimdVec4f& vPosition, const nsSimdQuat& qRotation = nsSimdQuat::MakeIdentity(), const nsSimdVec4f& vScale = nsSimdVec4f(1.0f)); // [tested]

  /// \brief Creates an identity transform.
  [[nodiscard]] static nsSimdTransform MakeIdentity(); // [tested]

  /// \brief Creates a transform that is the local transformation needed to get from the parent's transform to the child's.
  [[nodiscard]] static nsSimdTransform MakeLocalTransform(const nsSimdTransform& globalTransformParent, const nsSimdTransform& globalTransformChild); // [tested]

  /// \brief Creates a transform that is the global transform, that is reached by applying the child's local transform to the parent's global one.
  [[nodiscard]] static nsSimdTransform MakeGlobalTransform(const nsSimdTransform& globalTransformParent, const nsSimdTransform& localTransformChild); // [tested]

  /// \brief Returns the scale component with maximum magnitude.
  nsSimdFloat GetMaxScale() const; // [tested]

  /// \brief Returns whether this transform contains negative scaling aka mirroring.
  bool ContainsNegativeScale() const;

  /// \brief Returns whether this transform contains uniform scaling.
  bool ContainsUniformScale() const;

public:
  /// \brief Equality Check with epsilon
  bool IsEqual(const nsSimdTransform& rhs, const nsSimdFloat& fEpsilon) const; // [tested]

public:
  /// \brief Inverts this transform.
  void Invert(); // [tested]

  /// \brief Returns the inverse of this transform.
  nsSimdTransform GetInverse() const; // [tested]

  /// \brief Returns the transformation as a matrix.
  nsSimdMat4f GetAsMat4() const;                                            // [tested]

public:
  [[nodiscard]] nsSimdVec4f TransformPosition(const nsSimdVec4f& v) const;  // [tested]
  [[nodiscard]] nsSimdVec4f TransformDirection(const nsSimdVec4f& v) const; // [tested]

  /// \brief Concatenates the two transforms. This is the same as a matrix multiplication, thus not commutative.
  void operator*=(const nsSimdTransform& other); // [tested]

  /// \brief Multiplies \a q into the rotation component, thus rotating the entire transformation.
  void operator*=(const nsSimdQuat& q);  // [tested]

  void operator+=(const nsSimdVec4f& v); // [tested]
  void operator-=(const nsSimdVec4f& v); // [tested]

public:
  nsSimdVec4f m_Position;
  nsSimdQuat m_Rotation;
  nsSimdVec4f m_Scale;
};

// *** free functions ***

/// \brief Transforms the vector v by the transform.
NS_ALWAYS_INLINE const nsSimdVec4f operator*(const nsSimdTransform& t, const nsSimdVec4f& v); // [tested]

/// \brief Rotates the transform by the given quaternion. Multiplies q from the left with t.
NS_ALWAYS_INLINE const nsSimdTransform operator*(const nsSimdQuat& q, const nsSimdTransform& t); // [tested]

/// \brief Rotates the transform by the given quaternion. Multiplies q from the right with t.
NS_ALWAYS_INLINE const nsSimdTransform operator*(const nsSimdTransform& t, const nsSimdQuat& q); // [tested]

/// \brief Translates the nsSimdTransform by the vector. This will move the object in global space.
NS_ALWAYS_INLINE const nsSimdTransform operator+(const nsSimdTransform& t, const nsSimdVec4f& v); // [tested]

/// \brief Translates the nsSimdTransform by the vector. This will move the object in global space.
NS_ALWAYS_INLINE const nsSimdTransform operator-(const nsSimdTransform& t, const nsSimdVec4f& v); // [tested]

/// \brief Concatenates the two transforms. This is the same as a matrix multiplication, thus not commutative.
NS_ALWAYS_INLINE const nsSimdTransform operator*(const nsSimdTransform& lhs, const nsSimdTransform& rhs); // [tested]

NS_ALWAYS_INLINE bool operator==(const nsSimdTransform& t1, const nsSimdTransform& t2);                   // [tested]
NS_ALWAYS_INLINE bool operator!=(const nsSimdTransform& t1, const nsSimdTransform& t2);                   // [tested]


#include <Foundation/SimdMath/Implementation/SimdTransform_inl.h>
