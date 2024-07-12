#pragma once

#include <Foundation/Math/Mat4.h>
#include <Foundation/Math/Plane.h>
#include <Foundation/SimdMath/SimdBBox.h>
#include <Foundation/SimdMath/SimdBSphere.h>
#include <Foundation/SimdMath/SimdVec4b.h>
#include <Foundation/SimdMath/SimdVec4f.h>

/// \brief Enum that describes where in a volume another object is located.
struct nsVolumePosition
{
  /// \brief Enum that describes where in a volume another object is located.
  enum Enum
  {
    Outside,      //< means an object is ENTIRELY inside a volume
    Inside,       //< means an object is outside a volume
    Intersecting, //< means an object is PARTIALLY inside/outside a volume
  };
};

/// \brief Represents the frustum of some camera and can be used for culling objects.
///
/// The frustum always consists of exactly 6 planes (near, far, left, right, top, bottom).
///
/// The frustum planes point outwards, ie. when an object is in front of one of the planes, it is considered to be outside
/// the frustum.
///
/// Planes can be automatically extracted from a projection matrix or passed in manually.
/// In the latter case, make sure to pass them in in the order defined in the PlaneType enum.
class NS_FOUNDATION_DLL nsFrustum
{
public:
  enum PlaneType : nsUInt8
  {
    NearPlane,
    LeftPlane,
    RightPlane,
    FarPlane,
    BottomPlane,
    TopPlane,

    PLANE_COUNT
  };

  enum FrustumCorner : nsUInt8
  {
    NearTopLeft,
    NearTopRight,
    NearBottomLeft,
    NearBottomRight,
    FarTopLeft,
    FarTopRight,
    FarBottomLeft,
    FarBottomRight,

    CORNER_COUNT = 8
  };

  /// \brief The constructor does NOT initialize the frustum planes, make sure to call SetFrustum() before trying to use it.
  nsFrustum();
  ~nsFrustum();

  /// \brief Sets the frustum manually by specifying the planes directly.
  ///
  /// \note Make sure to pass in the planes in the order of the PlaneType enum, otherwise nsFrustum may not always work as expected.
  [[nodiscard]] static nsFrustum MakeFromPlanes(const nsPlane* pPlanes); // [tested]

  /// \brief Sets the frustum manually by specifying the planes directly.
  ///
  /// \note Make sure to pass in the planes in the order of the PlaneType enum, otherwise nsFrustum may not always work as expected.
  ///
  /// Returns NS_SUCESS with a valid outFrustum if the operation was successful and NS_FAILURE otherwise.
  [[nodiscard]] static nsResult TryMakeFromPlanes(nsFrustum& out_frustum, const nsPlane* pPlanes);

  /// \brief Creates the frustum by extracting the planes from the given (model-view / projection) matrix.
  ///
  /// If the matrix is just the projection matrix, the frustum will be in local space. Pass the full ModelViewProjection
  /// matrix to create the frustum in world-space. If the projection matrix contained in ModelViewProjection is an infinite
  /// plane projection matrix, the resulting frustum will yield a far plane with infinite distance.
  [[nodiscard]] static nsFrustum MakeFromMVP(const nsMat4& mModelViewProjection, nsClipSpaceDepthRange::Enum depthRange = nsClipSpaceDepthRange::Default, nsHandedness::Enum handedness = nsHandedness::Default); // [tested]

  /// \brief Creates the frustum by extracting the planes from the given (model-view / projection) matrix.
  ///
  /// If the matrix is just the projection matrix, the frustum will be in local space. Pass the full ModelViewProjection
  /// matrix to create the frustum in world-space. If the projection matrix contained in ModelViewProjection is an infinite
  /// plane projection matrix, the resulting frustum will yield a far plane with infinite distance.
  ///
  /// Returns NS_SUCESS with a valid outFrustum if the operation was successful and NS_FAILURE otherwise.
  [[nodiscard]] static nsResult TryMakeFromMVP(nsFrustum& out_frustum, const nsMat4& mModelViewProjection, nsClipSpaceDepthRange::Enum depthRange = nsClipSpaceDepthRange::Default, nsHandedness::Enum handedness = nsHandedness::Default);

  /// \brief Creates a frustum from the given camera position, direction vectors and the field-of-view along X and Y.
  ///
  /// The up vector does not need to be exactly orthogonal to the forwards vector, it will get recomputed properly.
  /// FOV X and Y define the entire field-of-view, so a FOV of 180 degree would mean the entire half-space in front of the camera.
  [[nodiscard]] static nsFrustum MakeFromFOV(const nsVec3& vPosition, const nsVec3& vForwards, const nsVec3& vUp, nsAngle fovX, nsAngle fovY, float fNearPlane, float fFarPlane); // [tested]

  /// \brief Creates a frustum from the given camera position, direction vectors and the field-of-view along X and Y.
  ///
  /// The up vector does not need to be exactly orthogonal to the forwards vector, it will get recomputed properly.
  /// FOV X and Y define the entire field-of-view, so a FOV of 180 degree would mean the entire half-space in front of the camera.
  ///
  /// Returns NS_SUCESS with a valid outFrustum if the operation was successful and NS_FAILURE otherwise.
  [[nodiscard]] static nsResult TryMakeFromFOV(nsFrustum& out_frustum, const nsVec3& vPosition, const nsVec3& vForwards, const nsVec3& vUp, nsAngle fovX, nsAngle fovY, float fNearPlane, float fFarPlane);

  /// \brief Creates a frustum from 8 corner points.
  ///
  /// Asserts that the frustum is valid after construction. Thus the given points must form a proper frustum.
  [[nodiscard]] static nsFrustum MakeFromCorners(const nsVec3 pCorners[FrustumCorner::CORNER_COUNT]);

  /// \brief Creates a frustum from 8 corner points.
  ///
  /// Returns NS_SUCESS with a valid outFrustum if the operation was successful and NS_FAILURE otherwise.
  [[nodiscard]] static nsResult TryMakeFromCorners(nsFrustum& out_frustum, const nsVec3 pCorners[FrustumCorner::CORNER_COUNT]);

  /// \brief Returns the n-th plane of the frustum.
  const nsPlane& GetPlane(nsUInt8 uiPlane) const; // [tested]

  /// \brief Returns the n-th plane of the frustum and allows modification.
  nsPlane& AccessPlane(nsUInt8 uiPlane);

  /// \brief Checks that all planes are valid.
  bool IsValid() const;

  /// \brief Transforms the frustum by the given matrix. This allows to adjust the frustum to a new orientation when a camera is moved or
  /// when it is necessary to cull from a different position.
  void TransformFrustum(const nsMat4& mTransform); // [tested]

  /// \brief Returns frustum transformed by given matrix
  nsFrustum GetTransformedFrustum(const nsMat4& mTransform) const; // [tested]

  /// \brief Flips all frustum planes around. Might be necessary after creating the frustum from a mirror projection matrix.
  void InvertFrustum(); // [tested]

  /// \brief Computes the frustum corner points.
  ///
  /// Note: If the frustum contains an infinite far plane, the far plane corners (out_points[4..7])
  /// will be at infinity.
  nsResult ComputeCornerPoints(nsVec3 out_pPoints[FrustumCorner::CORNER_COUNT]) const; // [tested]

  /// \brief Checks whether the given object is inside or outside the frustum.
  ///
  /// A concave object might be classified as 'intersecting' although it is outside the frustum, if it overlaps the planes just right.
  /// However an object that overlaps the frustum is definitely never classified as 'outside'.
  nsVolumePosition::Enum GetObjectPosition(const nsVec3* pVertices, nsUInt32 uiNumVertices) const; // [tested]

  /// \brief Same as GetObjectPosition(), but applies a transformation to the given object first. This allows to do culling on instanced
  /// objects.
  nsVolumePosition::Enum GetObjectPosition(const nsVec3* pVertices, nsUInt32 uiNumVertices, const nsMat4& mObjectTransform) const; // [tested]

  /// \brief Checks whether the given object is inside or outside the frustum.
  nsVolumePosition::Enum GetObjectPosition(const nsBoundingSphere& sphere) const; // [tested]

  /// \brief Checks whether the given object is inside or outside the frustum.
  nsVolumePosition::Enum GetObjectPosition(const nsBoundingBox& box) const; // [tested]

  /// \brief Returns true if the object is fully inside the frustum or partially overlaps it. Returns false when the object is fully outside
  /// the frustum.
  ///
  /// This function is more efficient than GetObjectPosition() and should be preferred when possible.
  bool Overlaps(const nsSimdBBox& object) const; // [tested]

  /// \brief Returns true if the object is fully inside the frustum or partially overlaps it. Returns false when the object is fully outside
  /// the frustum.
  ///
  /// This function is more efficient than GetObjectPosition() and should be preferred when possible.
  bool Overlaps(const nsSimdBSphere& object) const; // [tested]

private:
  nsPlane m_Planes[PLANE_COUNT];
};

#include <Foundation/Math/Implementation/Frustum_inl.h>
