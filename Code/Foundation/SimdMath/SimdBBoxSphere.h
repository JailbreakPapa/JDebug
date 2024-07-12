#pragma once

#include <Foundation/SimdMath/SimdBBox.h>

class NS_FOUNDATION_DLL nsSimdBBoxSphere
{
public:
  NS_DECLARE_POD_TYPE();

  /// \brief Default constructor does not initialize anything.
  nsSimdBBoxSphere(); // [tested]

  /// \brief Constructs the bounds from the center position, the box half extends and the sphere radius.
  [[deprecated("Use MakeFromCenterExtents() instead.")]] nsSimdBBoxSphere(const nsSimdVec4f& vCenter, const nsSimdVec4f& vBoxHalfExtents, const nsSimdFloat& fSphereRadius); // [tested]

  /// \brief Constructs the bounds from the given box and sphere.
  [[deprecated("Use MakeFromBoxAndSphere() instead.")]] nsSimdBBoxSphere(const nsSimdBBox& box, const nsSimdBSphere& sphere); // [tested]

  /// \brief Constructs the bounds from the given box. The sphere radius is calculated from the box extends.
  nsSimdBBoxSphere(const nsSimdBBox& box); // [tested]

  /// \brief Constructs the bounds from the given sphere. The box extends are calculated from the sphere radius.
  nsSimdBBoxSphere(const nsSimdBSphere& sphere); // [tested]

  /// \brief Creates an object with all zero values. These are valid bounds around the origin with no volume.
  [[nodiscard]] static nsSimdBBoxSphere MakeZero();

  /// \brief Creates an 'invalid' object, ie one with negative extents/radius. Invalid objects can be made valid through ExpandToInclude().
  [[nodiscard]] static nsSimdBBoxSphere MakeInvalid(); // [tested]

  /// \brief Creates an object from the given center point and extents.
  [[nodiscard]] static nsSimdBBoxSphere MakeFromCenterExtents(const nsSimdVec4f& vCenter, const nsSimdVec4f& vBoxHalfExtents, const nsSimdFloat& fSphereRadius);

  /// \brief Creates an object that contains all the provided points.
  [[nodiscard]] static nsSimdBBoxSphere MakeFromPoints(const nsSimdVec4f* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride = sizeof(nsSimdVec4f));

  /// \brief Creates an object from another bounding box.
  [[nodiscard]] static nsSimdBBoxSphere MakeFromBox(const nsSimdBBox& box);

  /// \brief Creates an object from another bounding sphere.
  [[nodiscard]] static nsSimdBBoxSphere MakeFromSphere(const nsSimdBSphere& sphere);

  /// \brief Creates an object from another bounding box and a sphere.
  [[nodiscard]] static nsSimdBBoxSphere MakeFromBoxAndSphere(const nsSimdBBox& box, const nsSimdBSphere& sphere);


public:
  /// \brief Resets the bounds to an invalid state.
  [[deprecated("Use MakeInvalid() instead.")]] void SetInvalid(); // [tested]

  /// \brief Checks whether the bounds is in an invalid state.
  bool IsValid() const; // [tested]

  /// \brief Checks whether any component is NaN.
  bool IsNaN() const; // [tested]

  /// \brief Calculates the bounds from given set of points.
  [[deprecated("Use MakeFromPoints() instead.")]] void SetFromPoints(const nsSimdVec4f* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride = sizeof(nsSimdVec4f)); // [tested]

  /// \brief Returns the bounding box.
  nsSimdBBox GetBox() const; // [tested]

  /// \brief Returns the bounding sphere.
  nsSimdBSphere GetSphere() const; // [tested]

  /// \brief Expands the bounds such that the given bounds are inside it.
  void ExpandToInclude(const nsSimdBBoxSphere& rhs); // [tested]

  /// \brief Transforms the bounds in its local space.
  void Transform(const nsSimdTransform& t); // [tested]

  /// \brief Transforms the bounds in its local space.
  void Transform(const nsSimdMat4f& mMat);                          // [tested]

  [[nodiscard]] bool operator==(const nsSimdBBoxSphere& rhs) const; // [tested]
  [[nodiscard]] bool operator!=(const nsSimdBBoxSphere& rhs) const; // [tested]

public:
  nsSimdVec4f m_CenterAndRadius;
  nsSimdVec4f m_BoxHalfExtents;
};

#include <Foundation/SimdMath/Implementation/SimdBBoxSphere_inl.h>
