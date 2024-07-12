#pragma once

#include <Foundation/Math/Vec3.h>

/// \brief An axis-aligned bounding box implementation.
///
/// This class allows to construct AABBs and also provides a large set of functions to work with them,
/// e.g. for overlap queries and ray casts.

template <typename Type>
class nsBoundingBoxTemplate
{
public:
  // Means this object can be copied using memcpy instead of copy construction.
  NS_DECLARE_POD_TYPE();

  using ComponentType = Type;

public:
  /// \brief Default constructor does not initialize anything.
  nsBoundingBoxTemplate();

  /// \brief Constructs the box with the given minimum and maximum values.
  nsBoundingBoxTemplate(const nsVec3Template<Type>& vMin, const nsVec3Template<Type>& vMax); // [tested]

#if NS_ENABLED(NS_MATH_CHECK_FOR_NAN)
  void AssertNotNaN() const
  {
    NS_ASSERT_ALWAYS(!IsNaN(), "This object contains NaN values. This can happen when you forgot to initialize it before using it. Please check that "
                               "all code-paths properly initialize this object.");
  }
#endif

  /// \brief Creates a box that is located at the origin and has zero size. This is a 'valid' box.
  [[nodiscard]] static nsBoundingBoxTemplate<Type> MakeZero();

  /// \brief Creates a box that is in an invalid state. ExpandToInclude can then be used to make it into a bounding box for objects.
  [[nodiscard]] static nsBoundingBoxTemplate<Type> MakeInvalid(); // [tested]

  /// \brief Creates a box from a center point and half-extents for each axis.
  [[nodiscard]] static nsBoundingBoxTemplate<Type> MakeFromCenterAndHalfExtents(const nsVec3Template<Type>& vCenter, const nsVec3Template<Type>& vHalfExtents); // [tested]

  /// \brief Creates a box with the given minimum and maximum values.
  [[nodiscard]] static nsBoundingBoxTemplate<Type> MakeFromMinMax(const nsVec3Template<Type>& vMin, const nsVec3Template<Type>& vMax); // [tested]

  /// \brief Creates a box around the given set of points. If uiNumPoints is zero, the returned box is invalid (same as MakeInvalid() returns).
  [[nodiscard]] static nsBoundingBoxTemplate<Type> MakeFromPoints(const nsVec3Template<Type>* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride = sizeof(nsVec3Template<Type>)); // [tested]

  /// \brief Checks whether the box is in an invalid state.
  bool IsValid() const; // [tested]

  /// \brief Checks whether any component is NaN.
  bool IsNaN() const; // [tested]

  /// \brief Writes the 8 different corners of the box to the given array.
  void GetCorners(nsVec3Template<Type>* out_pCorners) const; // [tested]

  /// \brief Returns the center position of the box.
  const nsVec3Template<Type> GetCenter() const; // [tested]

  /// \brief Returns the extents of the box along each axis.
  const nsVec3Template<Type> GetExtents() const; // [tested]

  /// \brief Returns the half extents of the box along each axis.
  const nsVec3Template<Type> GetHalfExtents() const; // [tested]

  /// \brief Expands the box such that the given point is inside it.
  void ExpandToInclude(const nsVec3Template<Type>& vPoint); // [tested]

  /// \brief Expands the box such that the given box is inside it.
  void ExpandToInclude(const nsBoundingBoxTemplate& rhs); // [tested]

  /// \brief Expands the box such that all the given points are inside it.
  void ExpandToInclude(const nsVec3Template<Type>* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride = sizeof(nsVec3Template<Type>)); // [tested]

  /// \brief If the box is not cubic all extents are set to the value of the maximum extent, such that the box becomes cubic.
  void ExpandToCube(); // [tested]

  /// \brief Will increase the size of the box in all directions by the given amount (per axis).
  void Grow(const nsVec3Template<Type>& vDiff); // [tested]

  /// \brief Checks whether the given point is inside the box.
  bool Contains(const nsVec3Template<Type>& vPoint) const; // [tested]

  /// \brief Checks whether the given box is completely inside this box.
  bool Contains(const nsBoundingBoxTemplate& rhs) const; // [tested]

  /// \brief Checks whether all the given points are inside this box.
  bool Contains(const nsVec3Template<Type>* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride = sizeof(nsVec3Template<Type>)) const; // [tested]

  /// \brief Checks whether the given sphere is completely inside this box.
  bool Contains(const nsBoundingSphereTemplate<Type>& sphere) const; // [tested]

  /// \brief Checks whether this box overlaps with the given box.
  bool Overlaps(const nsBoundingBoxTemplate& rhs) const; // [tested]

  /// \brief Checks whether any of the given points is inside this box.
  bool Overlaps(const nsVec3Template<Type>* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride = sizeof(nsVec3Template<Type>)) const; // [tested]

  /// \brief Checks whether the given sphere overlaps with this box.
  bool Overlaps(const nsBoundingSphereTemplate<Type>& sphere) const; // [tested]

  /// \brief Checks whether this box and the other box are exactly identical.
  bool IsIdentical(const nsBoundingBoxTemplate& rhs) const; // [tested]

  /// \brief Checks whether this box and the other box are equal within some threshold.
  bool IsEqual(const nsBoundingBoxTemplate& rhs, Type fEpsilon = nsMath::DefaultEpsilon<Type>()) const; // [tested]

  /// \brief Moves the box by the given vector.
  void Translate(const nsVec3Template<Type>& vDiff); // [tested]

  /// \brief Scales the box along each axis, but keeps its center constant.
  void ScaleFromCenter(const nsVec3Template<Type>& vScale); // [tested]

  /// \brief Scales the box's corners by the given factors, thus also moves the box around.
  void ScaleFromOrigin(const nsVec3Template<Type>& vScale); // [tested]

  /// \brief Transforms the corners of the box in its local space. The center of the box does not change, unless the transform contains a translation.
  void TransformFromCenter(const nsMat4Template<Type>& mTransform); // [tested]

  /// \brief Transforms the corners of the box and recomputes the AABB of those transformed points. Rotations and scalings will influence the center position of the box.
  void TransformFromOrigin(const nsMat4Template<Type>& mTransform); // [tested]

  /// \brief The given point is clamped to the volume of the box, i.e. it will be either inside the box or on its surface and it will have the closest
  /// possible distance to the original point.
  const nsVec3Template<Type> GetClampedPoint(const nsVec3Template<Type>& vPoint) const; // [tested]

  /// \brief Returns the squared minimum distance from the box's surface to the point. Zero if the point is inside the box.
  Type GetDistanceSquaredTo(const nsVec3Template<Type>& vPoint) const; // [tested]

  /// \brief Returns the minimum squared distance between the two boxes. Zero if the boxes overlap.
  Type GetDistanceSquaredTo(const nsBoundingBoxTemplate& rhs) const; // [tested]

  /// \brief Returns the minimum distance from the box's surface to the point. Zero if the point is inside the box.
  Type GetDistanceTo(const nsVec3Template<Type>& vPoint) const; // [tested]

  /// \brief Returns the minimum distance between the box and the sphere. Zero or negative if both overlap.
  Type GetDistanceTo(const nsBoundingSphereTemplate<Type>& sphere) const; // [tested]

  /// \brief Returns the minimum distance between the two boxes. Zero if the boxes overlap.
  Type GetDistanceTo(const nsBoundingBoxTemplate& rhs) const; // [tested]

  /// \brief Returns whether the given ray intersects the box. Optionally returns the intersection distance and position.
  /// Note that vRayDir is not required to be normalized.
  bool GetRayIntersection(const nsVec3Template<Type>& vStartPos, const nsVec3Template<Type>& vRayDir, Type* out_pIntersectionDistance = nullptr,
    nsVec3Template<Type>* out_pIntersection = nullptr) const; // [tested]

  /// \brief Checks whether the line segment intersects the box. Optionally returns the intersection point and the fraction along the line segment
  /// where the intersection occurred.
  bool GetLineSegmentIntersection(const nsVec3Template<Type>& vStartPos, const nsVec3Template<Type>& vEndPos, Type* out_pLineFraction = nullptr,
    nsVec3Template<Type>* out_pIntersection = nullptr) const; // [tested]

  /// \brief Returns a bounding sphere that encloses this box.
  const nsBoundingSphereTemplate<Type> GetBoundingSphere() const; // [tested]


public:
  nsVec3Template<Type> m_vMin;
  nsVec3Template<Type> m_vMax;
};

/// \brief Checks whether this box and the other are identical.
template <typename Type>
bool operator==(const nsBoundingBoxTemplate<Type>& lhs, const nsBoundingBoxTemplate<Type>& rhs); // [tested]

/// \brief Checks whether this box and the other are not identical.
template <typename Type>
bool operator!=(const nsBoundingBoxTemplate<Type>& lhs, const nsBoundingBoxTemplate<Type>& rhs); // [tested]


#include <Foundation/Math/Implementation/BoundingBox_inl.h>
