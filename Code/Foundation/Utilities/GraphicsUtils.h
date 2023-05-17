#pragma once

#include <Foundation/Math/Mat4.h>

namespace wdGraphicsUtils
{
  /// \brief Projects the given point from 3D world space into screen space, if possible.
  ///
  /// \param ModelViewProjection
  ///   The Model-View-Projection matrix that is used by the camera.
  /// \param DepthRange
  ///   The depth range that is used by this projection matrix. \see wdClipSpaceDepthRange
  ///
  /// Returns WD_FAILURE, if the point could not be projected into screen space.
  /// \note The function reports WD_SUCCESS, when the point could be projected, however, that does not mean that the point actually lies
  /// within the viewport, it might still be outside the viewport.
  ///
  /// out_vScreenPos.z is the depth of the point in [0;1] range. The z value is always 'normalized' to this range
  /// (as long as the DepthRange parameter is correct), to make it easier to make subsequent code platform independent.
  WD_FOUNDATION_DLL wdResult ConvertWorldPosToScreenPos(const wdMat4& mModelViewProjection, const wdUInt32 uiViewportX, const wdUInt32 uiViewportY,
    const wdUInt32 uiViewportWidth, const wdUInt32 uiViewportHeight, const wdVec3& vPoint, wdVec3& out_vScreenPos,
    wdClipSpaceDepthRange::Enum depthRange = wdClipSpaceDepthRange::Default); // [tested]

  /// \brief Takes the screen space position (including depth in [0;1] range) and converts it into a world space position.
  ///
  /// \param InverseModelViewProjection
  ///   The inverse of the Model-View-Projection matrix that is used by the camera.
  /// \param DepthRange
  ///   The depth range that is used by this projection matrix. \see wdClipSpaceDepthRange
  ///
  /// Returns WD_FAILURE when the screen coordinate could not be converted to a world position,
  /// which should generally not be possible as long as the coordinate is actually inside the viewport.
  ///
  /// Optionally this function also computes the direction vector through the world space position, that should be used for picking
  /// operations. Note that for perspective cameras this is the same as the direction from the camera position to the computed point,
  /// but for orthographic cameras it is not (it's simply the forward vector of the camera).
  /// This function handles both cases properly.
  ///
  /// The z value of vScreenPos is always expected to be in [0; 1] range (meaning 0 is at the near plane, 1 at the far plane),
  /// even on platforms that use [-1; +1] range for clip-space z values. The DepthRange parameter needs to be correct to handle this case
  /// properly.
  WD_FOUNDATION_DLL wdResult ConvertScreenPosToWorldPos(const wdMat4& mInverseModelViewProjection, const wdUInt32 uiViewportX,
    const wdUInt32 uiViewportY, const wdUInt32 uiViewportWidth, const wdUInt32 uiViewportHeight, const wdVec3& vScreenPos, wdVec3& out_vPoint,
    wdVec3* out_pDirection = nullptr,
    wdClipSpaceDepthRange::Enum depthRange = wdClipSpaceDepthRange::Default); // [tested]

  /// \brief A double-precision version of ConvertScreenPosToWorldPos()
  WD_FOUNDATION_DLL wdResult ConvertScreenPosToWorldPos(const wdMat4d& mInverseModelViewProjection, const wdUInt32 uiViewportX,
    const wdUInt32 uiViewportY, const wdUInt32 uiViewportWidth, const wdUInt32 uiViewportHeight, const wdVec3& vScreenPos, wdVec3& out_vPoint,
    wdVec3* out_pDirection = nullptr,
    wdClipSpaceDepthRange::Enum depthRange = wdClipSpaceDepthRange::Default); // [tested]

  /// \brief Checks whether the given transformation matrix would change the winding order of a triangle's vertices and thus requires that
  /// the vertex order gets reversed to compensate.
  WD_FOUNDATION_DLL bool IsTriangleFlipRequired(const wdMat3& mTransformation);

  /// \brief Converts a projection or view-projection matrix from one depth-range convention to another
  WD_FOUNDATION_DLL void ConvertProjectionMatrixDepthRange(
    wdMat4& inout_mMatrix, wdClipSpaceDepthRange::Enum srcDepthRange, wdClipSpaceDepthRange::Enum dstDepthRange); // [tested]

  /// \brief Retrieves the horizontal and vertical field-of-view angles from the perspective matrix.
  ///
  /// \note If an orthographic projection matrix is passed in, the returned angle values will be zero.
  WD_FOUNDATION_DLL void ExtractPerspectiveMatrixFieldOfView(const wdMat4& mProjectionMatrix, wdAngle& out_fovX, wdAngle& out_fovY); // [tested]

  /// \brief Extracts the field of view angles from a perspective matrix.
  /// \param ProjectionMatrix Perspective projection matrix to be decomposed.
  /// \param out_fFovLeft Left angle of the frustum. Negative in symmetric projection.
  /// \param out_fFovRight Right angle of the frustum.
  /// \param out_fFovBottom Bottom angle of the frustum. Negative in symmetric projection.
  /// \param out_fFovTop Top angle of the frustum.
  /// \param yRange The Y range used to construct the perspective matrix.
  WD_FOUNDATION_DLL void ExtractPerspectiveMatrixFieldOfView(const wdMat4& mProjectionMatrix, wdAngle& out_fovLeft, wdAngle& out_fovRight, wdAngle& out_fovBottom, wdAngle& out_fovTop, wdClipSpaceYMode::Enum range = wdClipSpaceYMode::Regular); // [tested]

  /// \brief Extracts the field of view distances on the near plane from a perspective matrix.
  ///
  /// Convenience function that also extracts near / far values and returns the distances on the near plane to be the inverse of wdGraphicsUtils::CreatePerspectiveProjectionMatrix.
  /// \sa wdGraphicsUtils::CreatePerspectiveProjectionMatrix
  WD_FOUNDATION_DLL wdResult ExtractPerspectiveMatrixFieldOfView(const wdMat4& mProjectionMatrix, float& out_fLeft, float& out_fRight, float& out_fBottom, float& out_fTop, wdClipSpaceDepthRange::Enum depthRange = wdClipSpaceDepthRange::Default, wdClipSpaceYMode::Enum range = wdClipSpaceYMode::Regular); // [tested]

  /// \brief Computes the distances of the near and far clip planes from the given perspective projection matrix.
  ///
  /// Returns WD_FAILURE when one of the values could not be computed, because it would result in a "division by zero".
  WD_FOUNDATION_DLL wdResult ExtractNearAndFarClipPlaneDistances(float& out_fNear, float& out_fFar, const wdMat4& mProjectionMatrix,
    wdClipSpaceDepthRange::Enum depthRange = wdClipSpaceDepthRange::Default); // [tested]


  enum class FrustumPlaneInterpolation
  {
    LeftToRight,
    BottomToTop,
    NearToFar,
  };

  /// \brief Computes an interpolated frustum plane by using linear interpolation in normalized clip space.
  ///
  /// Along left/right, up/down this makes it easy to create a regular grid of planes.
  /// Along near/far creating planes at regular intervals will result in planes in world-space that represent
  /// the same amount of depth-precision.
  ///
  /// \param dir Specifies which planes to interpolate.
  /// \param fLerpFactor The interpolation coefficient (usually in the interval [0;1]).
  WD_FOUNDATION_DLL wdPlane ComputeInterpolatedFrustumPlane(FrustumPlaneInterpolation dir, float fLerpFactor, const wdMat4& mProjectionMatrix,
    wdClipSpaceDepthRange::Enum depthRange = wdClipSpaceDepthRange::Default); // [tested]

  /// \brief Creates a perspective projection matrix with Left = -fViewWidth/2, Right = +fViewWidth/2, Bottom = -fViewHeight/2, Top =
  /// +fViewHeight/2.
  WD_FOUNDATION_DLL wdMat4 CreatePerspectiveProjectionMatrix(float fViewWidth, float fViewHeight, float fNearZ, float fFarZ,
    wdClipSpaceDepthRange::Enum depthRange = wdClipSpaceDepthRange::Default, wdClipSpaceYMode::Enum range = wdClipSpaceYMode::Regular,
    wdHandedness::Enum handedness = wdHandedness::Default); // [tested]

  /// \brief Creates a perspective projection matrix.
  WD_FOUNDATION_DLL wdMat4 CreatePerspectiveProjectionMatrix(float fLeft, float fRight, float fBottom, float fTop, float fNearZ, float fFarZ,
    wdClipSpaceDepthRange::Enum depthRange = wdClipSpaceDepthRange::Default, wdClipSpaceYMode::Enum range = wdClipSpaceYMode::Regular,
    wdHandedness::Enum handedness = wdHandedness::Default); // [tested]

  /// \brief Creates a perspective projection matrix.
  /// \param fFieldOfViewX    Horizontal field of view.
  WD_FOUNDATION_DLL wdMat4 CreatePerspectiveProjectionMatrixFromFovX(wdAngle fieldOfViewX, float fAspectRatioWidthDivHeight, float fNearZ,
    float fFarZ, wdClipSpaceDepthRange::Enum depthRange = wdClipSpaceDepthRange::Default, wdClipSpaceYMode::Enum range = wdClipSpaceYMode::Regular,
    wdHandedness::Enum handedness = wdHandedness::Default); // [tested]

  /// \brief Creates a perspective projection matrix.
  /// \param fFieldOfViewY    Vertical field of view.
  WD_FOUNDATION_DLL wdMat4 CreatePerspectiveProjectionMatrixFromFovY(wdAngle fieldOfViewY, float fAspectRatioWidthDivHeight, float fNearZ,
    float fFarZ, wdClipSpaceDepthRange::Enum depthRange = wdClipSpaceDepthRange::Default, wdClipSpaceYMode::Enum range = wdClipSpaceYMode::Regular,
    wdHandedness::Enum handedness = wdHandedness::Default); // [tested]

  /// \brief Creates an orthographic projection matrix with Left = -fViewWidth/2, Right = +fViewWidth/2, Bottom = -fViewHeight/2, Top =
  /// +fViewHeight/2.
  WD_FOUNDATION_DLL wdMat4 CreateOrthographicProjectionMatrix(float fViewWidth, float fViewHeight, float fNearZ, float fFarZ,
    wdClipSpaceDepthRange::Enum depthRange = wdClipSpaceDepthRange::Default, wdClipSpaceYMode::Enum range = wdClipSpaceYMode::Regular,
    wdHandedness::Enum handedness = wdHandedness::Default); // [tested]

  /// \brief Creates an orthographic projection matrix.
  WD_FOUNDATION_DLL wdMat4 CreateOrthographicProjectionMatrix(float fLeft, float fRight, float fBottom, float fTop, float fNearZ, float fFarZ,
    wdClipSpaceDepthRange::Enum depthRange = wdClipSpaceDepthRange::Default, wdClipSpaceYMode::Enum range = wdClipSpaceYMode::Regular,
    wdHandedness::Enum handedness = wdHandedness::Default); // [tested]

  /// \brief Returns a look-at matrix (only direction, no translation).
  ///
  /// Since this only creates a rotation matrix, vTarget can be interpreted both as a position or a direction.
  WD_FOUNDATION_DLL wdMat3 CreateLookAtViewMatrix(
    const wdVec3& vTarget, const wdVec3& vUpDir, wdHandedness::Enum handedness = wdHandedness::Default); // [tested]

  /// \brief Same as CreateLookAtViewMatrix() but returns the inverse matrix
  WD_FOUNDATION_DLL wdMat3 CreateInverseLookAtViewMatrix(
    const wdVec3& vTarget, const wdVec3& vUpDir, wdHandedness::Enum handedness = wdHandedness::Default); // [tested]


  /// \brief Returns a look-at matrix with both rotation and translation
  WD_FOUNDATION_DLL wdMat4 CreateLookAtViewMatrix(const wdVec3& vEyePos, const wdVec3& vLookAtPos, const wdVec3& vUpDir,
    wdHandedness::Enum handedness = wdHandedness::Default); // [tested]

  /// \brief Same as CreateLookAtViewMatrix() but returns the inverse matrix
  WD_FOUNDATION_DLL wdMat4 CreateInverseLookAtViewMatrix(const wdVec3& vEyePos, const wdVec3& vLookAtPos, const wdVec3& vUpDir,
    wdHandedness::Enum handedness = wdHandedness::Default); // [tested]

  /// \brief Creates a view matrix from the given camera vectors.
  ///
  /// The vectors are put into the appropriate matrix rows and depending on the handedness negated where necessary.
  WD_FOUNDATION_DLL wdMat4 CreateViewMatrix(const wdVec3& vPosition, const wdVec3& vForwardDir, const wdVec3& vRightDir, const wdVec3& vUpDir,
    wdHandedness::Enum handedness = wdHandedness::Default); // [tested]

  /// \brief Similar to CreateViewMatrix() but creates the inverse matrix.
  WD_FOUNDATION_DLL wdMat4 CreateInverseViewMatrix(const wdVec3& vPosition, const wdVec3& vForwardDir, const wdVec3& vRightDir, const wdVec3& vUpDir,
    wdHandedness::Enum handedness = wdHandedness::Default); // [tested]

  /// \brief Extracts the forward, right and up dir and camera position from the given view matrix.
  ///
  /// The handedness should be the same as used in CreateViewMatrix() or CreateLookAtViewMatrix().
  WD_FOUNDATION_DLL void DecomposeViewMatrix(wdVec3& out_vPosition, wdVec3& out_vForwardDir, wdVec3& out_vRightDir, wdVec3& out_vUpDir,
    const wdMat4& mViewMatrix, wdHandedness::Enum handedness = wdHandedness::Default); // [tested]

  /// \brief Computes the barycentric coordinates of a point in a 3D triangle.
  ///
  /// \return If the triangle is degenerate (all points on a line, or two points identical), the function returns WD_FAILURE.
  WD_FOUNDATION_DLL wdResult ComputeBarycentricCoordinates(wdVec3& out_vCoordinates, const wdVec3& v0, const wdVec3& v1, const wdVec3& v2, const wdVec3& vPos);

  /// \brief Computes the barycentric coordinates of a point in a 2D triangle.
  ///
  /// \return If the triangle is degenerate (all points on a line, or two points identical), the function returns WD_FAILURE.
  WD_FOUNDATION_DLL wdResult ComputeBarycentricCoordinates(wdVec3& out_vCoordinates, const wdVec2& v0, const wdVec2& v1, const wdVec2& v2, const wdVec2& vPos);

} // namespace wdGraphicsUtils
