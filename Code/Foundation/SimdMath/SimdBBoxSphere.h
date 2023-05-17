#pragma once

#include <Foundation/SimdMath/SimdBBox.h>

class WD_FOUNDATION_DLL wdSimdBBoxSphere
{
public:
  WD_DECLARE_POD_TYPE();

  /// \brief Default constructor does not initialize anything.
  wdSimdBBoxSphere(); // [tested]

  /// \brief Constructs the bounds from the center position, the box half extends and the sphere radius.
  wdSimdBBoxSphere(const wdSimdVec4f& vCenter, const wdSimdVec4f& vBoxHalfExtents, const wdSimdFloat& fSphereRadius); // [tested]

  /// \brief Constructs the bounds from the given box and sphere.
  wdSimdBBoxSphere(const wdSimdBBox& box, const wdSimdBSphere& sphere); // [tested]

  /// \brief Constructs the bounds from the given box. The sphere radius is calculated from the box extends.
  wdSimdBBoxSphere(const wdSimdBBox& box); // [tested]

  /// \brief Constructs the bounds from the given sphere. The box extends are calculated from the sphere radius.
  wdSimdBBoxSphere(const wdSimdBSphere& sphere); // [tested]

public:
  /// \brief Resets the bounds to an invalid state.
  void SetInvalid(); // [tested]

  /// \brief Checks whether the bounds is in an invalid state.
  bool IsValid() const; // [tested]

  /// \brief Checks whether any component is NaN.
  bool IsNaN() const; // [tested]

  /// \brief Calculates the bounds from given set of points.
  void SetFromPoints(const wdSimdVec4f* pPoints, wdUInt32 uiNumPoints, wdUInt32 uiStride = sizeof(wdSimdVec4f)); // [tested]

  /// \brief Returns the bounding box.
  wdSimdBBox GetBox() const; // [tested]

  /// \brief Returns the bounding sphere.
  wdSimdBSphere GetSphere() const; // [tested]

  /// \brief Expands the bounds such that the given bounds are inside it.
  void ExpandToInclude(const wdSimdBBoxSphere& rhs); // [tested]

  /// \brief Transforms the bounds in its local space.
  void Transform(const wdSimdTransform& t); // [tested]

  /// \brief Transforms the bounds in its local space.
  void Transform(const wdSimdMat4f& mMat); // [tested]


  bool operator==(const wdSimdBBoxSphere& rhs) const; // [tested]
  bool operator!=(const wdSimdBBoxSphere& rhs) const; // [tested]

public:
  wdSimdVec4f m_CenterAndRadius;
  wdSimdVec4f m_BoxHalfExtents;
};

#include <Foundation/SimdMath/Implementation/SimdBBoxSphere_inl.h>
