#pragma once

#include <Foundation/SimdMath/SimdTransform.h>

class NS_FOUNDATION_DLL nsSimdBSphere
{
public:
  NS_DECLARE_POD_TYPE();

  /// \brief Default constructor does not initialize any data.
  nsSimdBSphere();

  /// \brief Creates a sphere with the given radius around the given center.
  nsSimdBSphere(const nsSimdVec4f& vCenter, const nsSimdFloat& fRadius); // [tested]

  /// \brief Creates a sphere at the origin with radius zero.
  [[nodiscard]] static nsSimdBSphere MakeZero();

  /// \brief Creates an 'invalid' sphere, with its center at the given position and a negative radius.
  ///
  /// Such a sphere can be made 'valid' through ExpandToInclude(), but be aware that the originally provided center position
  /// will always be part of the sphere.
  [[nodiscard]] static nsSimdBSphere MakeInvalid(const nsSimdVec4f& vCenter = nsSimdVec4f::MakeZero()); // [tested]

  /// \brief Creates a sphere with the provided center and radius.
  [[nodiscard]] static nsSimdBSphere MakeFromCenterAndRadius(const nsSimdVec4f& vCenter, const nsSimdFloat& fRadius); // [tested]

  /// \brief Creates a bounding sphere around the provided points.
  ///
  /// The center of the sphere will be at the 'center of mass' of all the points, and the radius will be the distance to the
  /// farthest point from there.
  [[nodiscard]] static nsSimdBSphere MakeFromPoints(const nsSimdVec4f* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride = sizeof(nsSimdVec4f));


public:
  /// \brief Sets the bounding sphere to invalid values.
  [[deprecated("Use MakeInvalid() instead.")]] void SetInvalid(); // [tested]

  /// \brief Returns whether the sphere has valid values.
  bool IsValid() const; // [tested]

  /// \brief Returns whether any value is NaN.
  bool IsNaN() const; // [tested]

  /// \brief Returns the center
  nsSimdVec4f GetCenter() const; // [tested]

  /// \brief Returns the radius
  nsSimdFloat GetRadius() const; // [tested]

  /// \brief Initializes the sphere to be the bounding sphere of all the given points.
  [[deprecated("Use MakeFromPoints() instead.")]] void SetFromPoints(const nsSimdVec4f* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride = sizeof(nsSimdVec4f));

  /// \brief Increases the sphere's radius to include this point.
  void ExpandToInclude(const nsSimdVec4f& vPoint); // [tested]

  /// \brief Increases the sphere's radius to include all given points. Does NOT change its position, thus the resulting sphere might be not
  /// a very tight fit. More efficient than calling this for every point individually.
  void ExpandToInclude(const nsSimdVec4f* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride = sizeof(nsSimdVec4f)); // [tested]

  /// \brief Increases this sphere's radius, such that it encloses the other sphere.
  void ExpandToInclude(const nsSimdBSphere& rhs); // [tested]

public:
  /// \brief Transforms the sphere in its local space.
  void Transform(const nsSimdTransform& t); // [tested]

  /// \brief Transforms the sphere in its local space.
  void Transform(const nsSimdMat4f& mMat); // [tested]

public:
  /// \brief Computes the distance of the point to the sphere's surface. Returns negative values for points inside the sphere.
  nsSimdFloat GetDistanceTo(const nsSimdVec4f& vPoint) const; // [tested]

  /// \brief Returns the distance between the two spheres. Zero for spheres that are exactly touching each other, negative values for
  /// overlapping spheres.
  nsSimdFloat GetDistanceTo(const nsSimdBSphere& rhs) const; // [tested]

  /// \brief Returns true if the given point is inside the sphere.
  bool Contains(const nsSimdVec4f& vPoint) const; // [tested]

  /// \brief Returns whether the other sphere is completely inside this sphere.
  bool Contains(const nsSimdBSphere& rhs) const; // [tested]

  /// \brief Checks whether the two objects overlap.
  bool Overlaps(const nsSimdBSphere& rhs) const; // [tested]

  /// \brief Clamps the given position to the volume of the sphere. The resulting point will always be inside the sphere, but have the
  /// closest distance to the original point.
  [[nodiscard]] nsSimdVec4f GetClampedPoint(const nsSimdVec4f& vPoint); // [tested]

  [[nodiscard]] bool operator==(const nsSimdBSphere& rhs) const;        // [tested]
  [[nodiscard]] bool operator!=(const nsSimdBSphere& rhs) const;        // [tested]

public:
  nsSimdVec4f m_CenterAndRadius;
};

#include <Foundation/SimdMath/Implementation/SimdBSphere_inl.h>
