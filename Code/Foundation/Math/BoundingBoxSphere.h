#pragma once

#include <Foundation/Math/Mat4.h>
#include <Foundation/Math/Vec4.h>

/// \brief A combination of a bounding box and a bounding sphere with the same center.
///
/// This class uses less memory than storying a bounding box and sphere separate.

template <typename Type>
class nsBoundingBoxSphereTemplate
{
public:
  // Means this object can be copied using memcpy instead of copy construction.
  NS_DECLARE_POD_TYPE();

  using ComponentType = Type;

public:
  /// \brief Default constructor does not initialize anything.
  nsBoundingBoxSphereTemplate(); // [tested]

  nsBoundingBoxSphereTemplate(const nsBoundingBoxSphereTemplate& rhs);

  void operator=(const nsBoundingBoxSphereTemplate& rhs);

  /// \brief Constructs the bounds from the given box. The sphere radius is calculated from the box extends.
  nsBoundingBoxSphereTemplate(const nsBoundingBoxTemplate<Type>& box); // [tested]

  /// \brief Constructs the bounds from the given sphere. The box extends are calculated from the sphere radius.
  nsBoundingBoxSphereTemplate(const nsBoundingSphereTemplate<Type>& sphere); // [tested]

  /// \brief Creates an object with all zero values. These are valid bounds around the origin with no volume.
  [[nodiscard]] static nsBoundingBoxSphereTemplate<Type> MakeZero();

  /// \brief Creates an 'invalid' object, ie one with negative extents/radius. Invalid objects can be made valid through ExpandToInclude().
  [[nodiscard]] static nsBoundingBoxSphereTemplate<Type> MakeInvalid();

  /// \brief Creates an object from the given center point and extents.
  [[nodiscard]] static nsBoundingBoxSphereTemplate<Type> MakeFromCenterExtents(const nsVec3Template<Type>& vCenter, const nsVec3Template<Type>& vBoxHalfExtents, Type fSphereRadius);

  /// \brief Creates an object that contains all the provided points.
  [[nodiscard]] static nsBoundingBoxSphereTemplate<Type> MakeFromPoints(const nsVec3Template<Type>* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride = sizeof(nsVec3Template<Type>));

  /// \brief Creates an object from another bounding box.
  [[nodiscard]] static nsBoundingBoxSphereTemplate<Type> MakeFromBox(const nsBoundingBoxTemplate<Type>& box);

  /// \brief Creates an object from another bounding sphere.
  [[nodiscard]] static nsBoundingBoxSphereTemplate<Type> MakeFromSphere(const nsBoundingSphereTemplate<Type>& sphere);

  /// \brief Creates an object from another bounding box and a sphere.
  [[nodiscard]] static nsBoundingBoxSphereTemplate<Type> MakeFromBoxAndSphere(const nsBoundingBoxTemplate<Type>& box, const nsBoundingSphereTemplate<Type>& sphere);


#if NS_ENABLED(NS_MATH_CHECK_FOR_NAN)
  void AssertNotNaN() const
  {
    NS_ASSERT_ALWAYS(!IsNaN(), "This object contains NaN values. This can happen when you forgot to initialize it before using it. Please check that "
                               "all code-paths properly initialize this object.");
  }
#endif

  /// \brief Checks whether the bounds is in an invalid state.
  bool IsValid() const; // [tested]

  /// \brief Checks whether any component is NaN.
  bool IsNaN() const; // [tested]

  /// \brief Returns the bounding box.
  const nsBoundingBoxTemplate<Type> GetBox() const; // [tested]

  /// \brief Returns the bounding sphere.
  const nsBoundingSphereTemplate<Type> GetSphere() const; // [tested]

  /// \brief Expands the bounds such that the given bounds are inside it.
  void ExpandToInclude(const nsBoundingBoxSphereTemplate& rhs); // [tested]

  /// \brief Transforms the bounds in its local space.
  void Transform(const nsMat4Template<Type>& mTransform); // [tested]

public:
  nsVec3Template<Type> m_vCenter;
  Type m_fSphereRadius;
  nsVec3Template<Type> m_vBoxHalfExtends;
};

/// \brief Checks whether this bounds and the other are identical.
template <typename Type>
bool operator==(const nsBoundingBoxSphereTemplate<Type>& lhs, const nsBoundingBoxSphereTemplate<Type>& rhs); // [tested]

/// \brief Checks whether this bounds and the other are not identical.
template <typename Type>
bool operator!=(const nsBoundingBoxSphereTemplate<Type>& lhs, const nsBoundingBoxSphereTemplate<Type>& rhs); // [tested]


#include <Foundation/Math/Implementation/BoundingBoxSphere_inl.h>
