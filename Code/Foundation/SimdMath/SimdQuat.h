#pragma once

#include <Foundation/SimdMath/SimdMat4f.h>

class NS_FOUNDATION_DLL nsSimdQuat
{
public:
  NS_DECLARE_POD_TYPE();

  nsSimdQuat();                              // [tested]

  explicit nsSimdQuat(const nsSimdVec4f& v); // [tested]

  /// \brief Static function that returns a quaternion that represents the identity rotation (none).
  [[nodiscard]] static const nsSimdQuat MakeIdentity(); // [tested]

  /// \brief Sets the individual elements of the quaternion directly. Note that x,y,z do NOT represent a rotation axis, and w does NOT represent an
  /// angle.
  ///
  /// Use this function only if you have good understanding of quaternion math and know exactly what you are doing.
  [[nodiscard]] static nsSimdQuat MakeFromElements(nsSimdFloat x, nsSimdFloat y, nsSimdFloat z, nsSimdFloat w); // [tested]

  /// \brief Creates a quaternion from a rotation-axis and an angle (angle is given in Radians or as an nsAngle)
  [[nodiscard]] static nsSimdQuat MakeFromAxisAndAngle(const nsSimdVec4f& vRotationAxis, const nsSimdFloat& fAngle); // [tested]

  /// \brief Creates a quaternion, that rotates through the shortest arc from "vDirFrom" to "vDirTo".
  [[nodiscard]] static nsSimdQuat MakeShortestRotation(const nsSimdVec4f& vDirFrom, const nsSimdVec4f& vDirTo); // [tested]

  /// \brief Returns a quaternion that is the spherical linear interpolation of the other two.
  [[nodiscard]] static nsSimdQuat MakeSlerp(const nsSimdQuat& qFrom, const nsSimdQuat& qTo, const nsSimdFloat& t); // [tested]

public:
  /// \brief Normalizes the quaternion to unit length. ALL rotation-quaternions should be normalized at all times (automatically).
  void Normalize(); // [tested]

  /// \brief Returns the rotation-axis and angle (in Radians), that this quaternion rotates around.
  nsResult GetRotationAxisAndAngle(nsSimdVec4f& ref_vAxis, nsSimdFloat& ref_fAngle, const nsSimdFloat& fEpsilon = nsMath::DefaultEpsilon<float>()) const; // [tested]

  /// \brief Returns the Quaternion as a matrix.
  nsSimdMat4f GetAsMat4() const; // [tested]

  /// \brief Checks whether all components are neither NaN nor infinite and that the quaternion is normalized.
  bool IsValid(const nsSimdFloat& fEpsilon = nsMath::DefaultEpsilon<float>()) const; // [tested]

  /// \brief Checks whether any component is NaN.
  bool IsNaN() const; // [tested]

  /// \brief Determines whether \a this and \a qOther represent the same rotation. This is a rather slow operation.
  ///
  /// Currently it fails when one of the given quaternions is identity (so no rotation, at all), as it tries to
  /// compare rotation axis' and angles, which is undefined for the identity quaternion (also there are infinite
  /// representations for 'identity', so it's difficult to check for it).
  bool IsEqualRotation(const nsSimdQuat& qOther, const nsSimdFloat& fEpsilon) const; // [tested]

public:
  /// \brief Returns a Quaternion that represents the negative / inverted rotation.
  [[nodiscard]] nsSimdQuat operator-() const; // [tested]

  /// \brief Rotates v by q
  [[nodiscard]] nsSimdVec4f operator*(const nsSimdVec4f& v) const; // [tested]

  /// \brief Concatenates the rotations of q1 and q2
  [[nodiscard]] nsSimdQuat operator*(const nsSimdQuat& q2) const; // [tested]

  bool operator==(const nsSimdQuat& q2) const;                    // [tested]
  bool operator!=(const nsSimdQuat& q2) const;                    // [tested]

public:
  nsSimdVec4f m_v;
};

#include <Foundation/SimdMath/Implementation/SimdQuat_inl.h>
