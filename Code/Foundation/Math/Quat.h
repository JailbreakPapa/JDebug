#pragma once

#include <Foundation/Math/Vec3.h>

/// \brief Quaternions can be used to represent rotations in 3D space.
///
/// Quaternions are useful to represent 3D rotations, as they are smaller and more efficient than matrices
/// and can be concatenated easily, without having the 'Gimbal Lock' problem of Euler Angles.
/// Either use a full blown transformation (e.g. a 4x4 matrix) to represent a object, or use a Quaternion
/// bundled with a position vector, if (non-uniform) scale is not required.
/// Quaternions can also easily be interpolated (via Slerp).
/// This implementation also allows to convert back and forth between Quaternions and Matrices easily.
///
/// Quaternions have no 'IsIdentical' or 'IsEqual' function, as there can be different representations for the
/// same rotation, and it is rather difficult to check this. So to not convey any false notion of being equal
/// (or rather unequal), those functions are not provided.
template <typename Type>
class wdQuatTemplate
{
public:
  // Means this object can be copied using memcpy instead of copy construction.
  WD_DECLARE_POD_TYPE();

  using ComponentType = Type;

  // *** Data ***
public:
  wdVec3Template<Type> v;
  Type w;

  // *** Constructors ***
public:
  wdQuatTemplate(); // [tested]

  /// \brief For internal use. You should never construct quaternions this way.
  wdQuatTemplate(Type x, Type y, Type z, Type w); // [tested]

#if WD_ENABLED(WD_MATH_CHECK_FOR_NAN)
  void AssertNotNaN() const
  {
    WD_ASSERT_ALWAYS(!IsNaN(), "This object contains NaN values. This can happen when you forgot to initialize it before using it. Please check that "
                               "all code-paths properly initialize this object.");
  }
#endif

  /// \brief Static function that returns a quaternion that represents the identity rotation (none).
  static const wdQuatTemplate<Type> IdentityQuaternion(); // [tested]

  // *** Functions to create a quaternion ***
public:
  /// \brief Sets the Quaternion to the identity.
  void SetIdentity(); // [tested]

  /// \brief Sets the individual elements of the quaternion directly. Note that x,y,z do NOT represent a rotation axis, and w does NOT represent an
  /// angle.
  ///
  /// Use this function only if you have good understanding of quaternion math and know exactly what you are doing.
  void SetElements(Type x, Type y, Type z, Type w); // [tested]

  /// \brief Creates a quaternion from a rotation-axis and an angle.
  void SetFromAxisAndAngle(const wdVec3Template<Type>& vRotationAxis, wdAngle angle); // [tested]

  /// \brief Creates a quaternion, that rotates through the shortest arc from "vDirFrom" to "vDirTo".
  void SetShortestRotation(const wdVec3Template<Type>& vDirFrom, const wdVec3Template<Type>& vDirTo); // [tested]

  /// \brief Creates a quaternion from the given matrix.
  void SetFromMat3(const wdMat3Template<Type>& m); // [tested]

  /// \brief Reconstructs a rotation quaternion from a matrix that may contain scaling and mirroring.
  ///
  /// In skeletal animation it is possible that matrices with mirroring are used, that need to be converted to a
  /// proper quaternion, even though a rotation with mirroring can't be represented by a quaternion.
  /// This function reconstructs a valid quaternion from such matrices. Obviously the mirroring information gets lost,
  /// but it is typically not needed any further anway.
  void ReconstructFromMat3(const wdMat3Template<Type>& m);

  /// \brief Reconstructs a rotation quaternion from a matrix that may contain scaling and mirroring.
  ///
  /// \sa ReconstructFromMat3()
  void ReconstructFromMat4(const wdMat4Template<Type>& m);

  /// \brief Sets this quaternion to be the spherical linear interpolation of the other two.
  void SetSlerp(const wdQuatTemplate& qFrom, const wdQuatTemplate& qTo, Type t); // [tested]

  // *** Common Functions ***
public:
  /// \brief Normalizes the quaternion to unit length. ALL rotation-quaternions should be normalized at all times (automatically).
  void Normalize(); // [tested]

  /// \brief Returns the rotation-axis and angle, that this quaternion rotates around.
  wdResult GetRotationAxisAndAngle(wdVec3Template<Type>& out_vAxis, wdAngle& out_angle, Type fEpsilon = wdMath::DefaultEpsilon<Type>()) const; // [tested]

  /// \brief Returns the Quaternion as a matrix.
  const wdMat3Template<Type> GetAsMat3() const; // [tested]

  /// \brief Returns the Quaternion as a matrix.
  const wdMat4Template<Type> GetAsMat4() const; // [tested]

  /// \brief Checks whether all components are neither NaN nor infinite and that the quaternion is normalized.
  bool IsValid(Type fEpsilon = wdMath::DefaultEpsilon<Type>()) const; // [tested]

  /// \brief Checks whether any component is NaN.
  bool IsNaN() const; // [tested]

  /// \brief Determines whether \a this and \a qOther represent the same rotation. This is a rather slow operation.
  ///
  /// Currently it fails when one of the given quaternions is identity (so no rotation, at all), as it tries to
  /// compare rotation axis' and angles, which is undefined for the identity quaternion (also there are infinite
  /// representations for 'identity', so it's difficult to check for it).
  bool IsEqualRotation(const wdQuatTemplate& qOther, Type fEpsilon) const; // [tested]

  /// \brief Inverts the rotation, so instead of rotating N degrees around its axis, the quaternion will rotate -N degrees around that axis.
  ///
  /// This modifies the quaternion in place. If you want to get the inverse as a copy, use the negation operator (-).
  void Invert();

  // *** Operators ***
public:
  /// \brief Returns a Quaternion that represents the negative / inverted rotation.
  const wdQuatTemplate operator-() const; // [tested]

  // *** Common Quaternion operations ***
public:
  /// \brief Returns the dot-product of the two quaternions (commutative, order does not matter).
  Type Dot(const wdQuatTemplate& rhs) const; // [tested]

  // *** Euler Angle Conversions ***
public:
  /// \brief Converts the quaternion to Euler angles
  void GetAsEulerAngles(wdAngle& out_x, wdAngle& out_y, wdAngle& out_z) const;

  /// \brief Sets the quaternion from Euler angles
  void SetFromEulerAngles(const wdAngle& x, const wdAngle& y, const wdAngle& z);
};

/// \brief Rotates v by q
template <typename Type>
const wdVec3Template<Type> operator*(const wdQuatTemplate<Type>& q, const wdVec3Template<Type>& v); // [tested]

/// \brief Concatenates the rotations of q1 and q2
template <typename Type>
const wdQuatTemplate<Type> operator*(const wdQuatTemplate<Type>& q1, const wdQuatTemplate<Type>& q2); // [tested]

template <typename Type>
bool operator==(const wdQuatTemplate<Type>& q1, const wdQuatTemplate<Type>& q2); // [tested]

template <typename Type>
bool operator!=(const wdQuatTemplate<Type>& q1, const wdQuatTemplate<Type>& q2); // [tested]

#include <Foundation/Math/Implementation/Quat_inl.h>
