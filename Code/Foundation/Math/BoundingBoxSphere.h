#pragma once

#include <Foundation/Math/Mat4.h>
#include <Foundation/Math/Vec4.h>

/// \brief A combination of a bounding box and a bounding sphere with the same center.
///
/// This class uses less memory than storying a bounding box and sphere separate.

template <typename Type>
class wdBoundingBoxSphereTemplate
{
public:
  // Means this object can be copied using memcpy instead of copy construction.
  WD_DECLARE_POD_TYPE();

  typedef Type ComponentType;

public:
  /// \brief Default constructor does not initialize anything.
  wdBoundingBoxSphereTemplate(); // [tested]

  /// \brief Constructs the bounds from the center position, the box half extends and the sphere radius.
  wdBoundingBoxSphereTemplate(const wdVec3Template<Type>& vCenter, const wdVec3Template<Type>& vBoxHalfExtents, Type fSphereRadius); // [tested]

  /// \brief Constructs the bounds from the given box and sphere.
  wdBoundingBoxSphereTemplate(const wdBoundingBoxTemplate<Type>& box, const wdBoundingSphereTemplate<Type>& sphere); // [tested]

  /// \brief Constructs the bounds from the given box. The sphere radius is calculated from the box extends.
  wdBoundingBoxSphereTemplate(const wdBoundingBoxTemplate<Type>& box); // [tested]

  /// \brief Constructs the bounds from the given sphere. The box extends are calculated from the sphere radius.
  wdBoundingBoxSphereTemplate(const wdBoundingSphereTemplate<Type>& sphere); // [tested]

#if WD_ENABLED(WD_MATH_CHECK_FOR_NAN)
  void AssertNotNaN() const
  {
    WD_ASSERT_ALWAYS(!IsNaN(), "This object contains NaN values. This can happen when you forgot to initialize it before using it. Please check that "
                               "all code-paths properly initialize this object.");
  }
#endif

  /// \brief Resets the bounds to an invalid state.
  void SetInvalid(); // [tested]

  /// \brief Checks whether the bounds is in an invalid state.
  bool IsValid() const; // [tested]

  /// \brief Checks whether any component is NaN.
  bool IsNaN() const; // [tested]

  /// \brief Calculates the bounds from given set of points.
  void SetFromPoints(const wdVec3Template<Type>* pPoints, wdUInt32 uiNumPoints, wdUInt32 uiStride = sizeof(wdVec3Template<Type>)); // [tested]

  /// \brief Returns the bounding box.
  const wdBoundingBoxTemplate<Type> GetBox() const; // [tested]

  /// \brief Returns the bounding sphere.
  const wdBoundingSphereTemplate<Type> GetSphere() const; // [tested]

  /// \brief Expands the bounds such that the given bounds are inside it.
  void ExpandToInclude(const wdBoundingBoxSphereTemplate& rhs); // [tested]

  /// \brief Transforms the bounds in its local space.
  void Transform(const wdMat4Template<Type>& mTransform); // [tested]

public:
  wdVec3Template<Type> m_vCenter;
  Type m_fSphereRadius;
  wdVec3Template<Type> m_vBoxHalfExtends;
};

/// \brief Checks whether this bounds and the other are identical.
template <typename Type>
bool operator==(const wdBoundingBoxSphereTemplate<Type>& lhs, const wdBoundingBoxSphereTemplate<Type>& rhs); // [tested]

/// \brief Checks whether this bounds and the other are not identical.
template <typename Type>
bool operator!=(const wdBoundingBoxSphereTemplate<Type>& lhs, const wdBoundingBoxSphereTemplate<Type>& rhs); // [tested]


#include <Foundation/Math/Implementation/BoundingBoxSphere_inl.h>
