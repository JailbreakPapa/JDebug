#pragma once

#include <Foundation/SimdMath/SimdBSphere.h>

class nsSimdBBox
{
public:
  NS_DECLARE_POD_TYPE();

  /// \brief Default constructor does not initialize anything.
  nsSimdBBox();

  /// \brief Constructs the box with the given minimum and maximum values.
  nsSimdBBox(const nsSimdVec4f& vMin, const nsSimdVec4f& vMax); // [tested]

  /// \brief Creates a box that is located at the origin and has zero size. This is a 'valid' box.
  [[nodiscard]] static nsSimdBBox MakeZero();

  /// \brief Creates a box that is in an invalid state. ExpandToInclude can then be used to make it into a bounding box for objects.
  [[nodiscard]] static nsSimdBBox MakeInvalid(); // [tested]

  /// \brief Creates a box from a center point and half-extents for each axis.
  [[nodiscard]] static nsSimdBBox MakeFromCenterAndHalfExtents(const nsSimdVec4f& vCenter, const nsSimdVec4f& vHalfExtents); // [tested]

  /// \brief Creates a box with the given minimum and maximum values.
  [[nodiscard]] static nsSimdBBox MakeFromMinMax(const nsSimdVec4f& vMin, const nsSimdVec4f& vMax); // [tested]

  /// \brief Creates a box around the given set of points. If uiNumPoints is zero, the returned box is invalid (same as MakeInvalid() returns).
  [[nodiscard]] static nsSimdBBox MakeFromPoints(const nsSimdVec4f* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride = sizeof(nsSimdVec4f)); // [tested]

public:
  /// \brief Resets the box to an invalid state. ExpandToInclude can then be used to make it into a bounding box for objects.
  [[deprecated("Use MakeInvalid() instead.")]] void SetInvalid(); // [tested]

  /// \brief Sets the box from a center point and half-extents for each axis.
  [[deprecated("Use MakeFromCenterAndHalfExtents() instead.")]] void SetCenterAndHalfExtents(const nsSimdVec4f& vCenter, const nsSimdVec4f& vHalfExtents); // [tested]

  /// \brief Creates a new bounding-box around the given set of points.
  [[deprecated("Use MakeFromPoints() instead.")]] void SetFromPoints(const nsSimdVec4f* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride = sizeof(nsSimdVec4f)); // [tested]

  /// \brief Checks whether the box is in an invalid state.
  bool IsValid() const; // [tested]

  /// \brief Checks whether any component is NaN.
  bool IsNaN() const; // [tested]

  /// \brief Returns the center position of the box.
  nsSimdVec4f GetCenter() const; // [tested]

  /// \brief Returns the extents of the box along each axis.
  nsSimdVec4f GetExtents() const; // [tested]

  /// \brief Returns the half extents of the box along each axis.
  nsSimdVec4f GetHalfExtents() const; // [tested]

  /// \brief Expands the box such that the given point is inside it.
  void ExpandToInclude(const nsSimdVec4f& vPoint); // [tested]

  /// \brief Expands the box such that all the given points are inside it.
  void ExpandToInclude(const nsSimdVec4f* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride = sizeof(nsSimdVec4f)); // [tested]

  /// \brief Expands the box such that the given box is inside it.
  void ExpandToInclude(const nsSimdBBox& rhs); // [tested]

  /// \brief If the box is not cubic all extents are set to the value of the maximum extent, such that the box becomes cubic.
  void ExpandToCube(); // [tested]


  /// \brief Checks whether the given point is inside the box.
  bool Contains(const nsSimdVec4f& vPoint) const; // [tested]

  /// \brief Checks whether the given box is completely inside this box.
  bool Contains(const nsSimdBBox& rhs) const; // [tested]

  /// \brief Checks whether the given sphere is completely inside this box.
  bool Contains(const nsSimdBSphere& sphere) const; // [tested]

  /// \brief Checks whether this box overlaps with the given box.
  bool Overlaps(const nsSimdBBox& rhs) const; // [tested]

  /// \brief Checks whether the given sphere overlaps with this box.
  bool Overlaps(const nsSimdBSphere& sphere) const; // [tested]


  /// \brief Will increase the size of the box in all directions by the given amount (per axis).
  void Grow(const nsSimdVec4f& vDiff); // [tested]

  /// \brief Moves the box by the given vector.
  void Translate(const nsSimdVec4f& vDiff); // [tested]

  /// \brief Transforms the corners of the box and recomputes the aabb of those transformed points.
  void Transform(const nsSimdTransform& transform); // [tested]

  /// \brief Transforms the corners of the box and recomputes the aabb of those transformed points.
  void Transform(const nsSimdMat4f& mMat); // [tested]


  /// \brief The given point is clamped to the volume of the box, i.e. it will be either inside the box or on its surface and it will have the closest
  /// possible distance to the original point.
  nsSimdVec4f GetClampedPoint(const nsSimdVec4f& vPoint) const; // [tested]

  /// \brief Returns the squared minimum distance from the box's surface to the point. Zero if the point is inside the box.
  nsSimdFloat GetDistanceSquaredTo(const nsSimdVec4f& vPoint) const; // [tested]

  /// \brief Returns the minimum distance from the box's surface to the point. Zero if the point is inside the box.
  nsSimdFloat GetDistanceTo(const nsSimdVec4f& vPoint) const; // [tested]


  bool operator==(const nsSimdBBox& rhs) const;               // [tested]
  bool operator!=(const nsSimdBBox& rhs) const;               // [tested]

public:
  nsSimdVec4f m_Min;
  nsSimdVec4f m_Max;
};

#include <Foundation/SimdMath/Implementation/SimdBBox_inl.h>
