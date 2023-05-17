#include <Foundation/FoundationPCH.h>

#include <Foundation/Utilities/GraphicsUtils.h>

wdResult wdGraphicsUtils::ConvertWorldPosToScreenPos(const wdMat4& mModelViewProjection, const wdUInt32 uiViewportX, const wdUInt32 uiViewportY, const wdUInt32 uiViewportWidth, const wdUInt32 uiViewportHeight, const wdVec3& vPoint, wdVec3& out_vScreenPos, wdClipSpaceDepthRange::Enum depthRange)
{
  const wdVec4 vToProject = vPoint.GetAsVec4(1.0f);

  wdVec4 vClipSpace = mModelViewProjection * vToProject;

  if (vClipSpace.w == 0.0f)
    return WD_FAILURE;

  wdVec3 vProjected = vClipSpace.GetAsVec3() / vClipSpace.w;
  if (vClipSpace.w < 0.0f)
    vProjected.z = -vProjected.z;

  out_vScreenPos.x = uiViewportX + uiViewportWidth * ((vProjected.x * 0.5f) + 0.5f);
  out_vScreenPos.y = uiViewportY + uiViewportHeight * ((vProjected.y * 0.5f) + 0.5f);

  // normalize the output z value to always be in [0; 1] range
  // That means when the projection matrix spits out values between -1 and +1, rescale those values
  if (depthRange == wdClipSpaceDepthRange::MinusOneToOne)
    out_vScreenPos.z = vProjected.z * 0.5f + 0.5f;
  else
    out_vScreenPos.z = vProjected.z;

  return WD_SUCCESS;
}

wdResult wdGraphicsUtils::ConvertScreenPosToWorldPos(
  const wdMat4& mInverseModelViewProjection, const wdUInt32 uiViewportX, const wdUInt32 uiViewportY, const wdUInt32 uiViewportWidth, const wdUInt32 uiViewportHeight, const wdVec3& vScreenPos, wdVec3& out_vPoint, wdVec3* out_pDirection, wdClipSpaceDepthRange::Enum depthRange)
{
  wdVec3 vClipSpace = vScreenPos;

  // From window coordinates to [0; 1] range
  vClipSpace.x = (vClipSpace.x - uiViewportX) / uiViewportWidth;
  vClipSpace.y = (vClipSpace.y - uiViewportY) / uiViewportHeight;

  // Map to range [-1; 1]
  vClipSpace.x = vClipSpace.x * 2.0f - 1.0f;
  vClipSpace.y = vClipSpace.y * 2.0f - 1.0f;

  // The OpenGL matrix expects the z values to be between -1 and +1, so rescale the incoming value to that range
  if (depthRange == wdClipSpaceDepthRange::MinusOneToOne)
    vClipSpace.z = vClipSpace.z * 2.0f - 1.0f;

  wdVec4 vToUnProject = vClipSpace.GetAsVec4(1.0f);

  wdVec4 vWorldSpacePoint = mInverseModelViewProjection * vToUnProject;

  if (vWorldSpacePoint.w == 0.0f)
    return WD_FAILURE;

  out_vPoint = vWorldSpacePoint.GetAsVec3() / vWorldSpacePoint.w;

  if (out_pDirection != nullptr)
  {
    vToUnProject.z += 0.1f; // a point that is a bit further away

    const wdVec4 vWorldSpacePoint2 = mInverseModelViewProjection * vToUnProject;

    WD_ASSERT_DEV(vWorldSpacePoint2.w != 0.0f, "It should not be possible that the first projected point has a w other than zero, but the second one has!");

    const wdVec3 vPoint2 = vWorldSpacePoint2.GetAsVec3() / vWorldSpacePoint2.w;

    *out_pDirection = (vPoint2 - out_vPoint).GetNormalized();
  }

  return WD_SUCCESS;
}

wdResult wdGraphicsUtils::ConvertScreenPosToWorldPos(const wdMat4d& mInverseModelViewProjection, const wdUInt32 uiViewportX, const wdUInt32 uiViewportY, const wdUInt32 uiViewportWidth, const wdUInt32 uiViewportHeight, const wdVec3& vScreenPos, wdVec3& out_vPoint, wdVec3* out_pDirection /*= nullptr*/,
  wdClipSpaceDepthRange::Enum depthRange /*= wdClipSpaceDepthRange::Default*/)
{
  wdVec3 vClipSpace = vScreenPos;

  // From window coordinates to [0; 1] range
  vClipSpace.x = (vClipSpace.x - uiViewportX) / uiViewportWidth;
  vClipSpace.y = (vClipSpace.y - uiViewportY) / uiViewportHeight;

  // Map to range [-1; 1]
  vClipSpace.x = vClipSpace.x * 2.0f - 1.0f;
  vClipSpace.y = vClipSpace.y * 2.0f - 1.0f;

  // The OpenGL matrix expects the z values to be between -1 and +1, so rescale the incoming value to that range
  if (depthRange == wdClipSpaceDepthRange::MinusOneToOne)
    vClipSpace.z = vClipSpace.z * 2.0f - 1.0f;

  wdVec4d vToUnProject = wdVec4d(vClipSpace.x, vClipSpace.y, vClipSpace.z, 1.0);

  wdVec4d vWorldSpacePoint = mInverseModelViewProjection * vToUnProject;

  if (vWorldSpacePoint.w == 0.0)
    return WD_FAILURE;

  wdVec3d outTemp = vWorldSpacePoint.GetAsVec3() / vWorldSpacePoint.w;
  out_vPoint.Set((float)outTemp.x, (float)outTemp.y, (float)outTemp.z);

  if (out_pDirection != nullptr)
  {
    vToUnProject.z += 0.1f; // a point that is a bit further away

    const wdVec4d vWorldSpacePoint2 = mInverseModelViewProjection * vToUnProject;

    WD_ASSERT_DEV(vWorldSpacePoint2.w != 0.0, "It should not be possible that the first projected point has a w other than zero, but the second one has!");

    const wdVec3d vPoint2 = vWorldSpacePoint2.GetAsVec3() / vWorldSpacePoint2.w;

    wdVec3d outDir = (vPoint2 - outTemp).GetNormalized();
    out_pDirection->Set((float)outDir.x, (float)outDir.y, (float)outDir.z);
  }

  return WD_SUCCESS;
}

bool wdGraphicsUtils::IsTriangleFlipRequired(const wdMat3& mTransformation)
{
  return (mTransformation.GetColumn(0).CrossRH(mTransformation.GetColumn(1)).Dot(mTransformation.GetColumn(2)) < 0.0f);
}

void wdGraphicsUtils::ConvertProjectionMatrixDepthRange(wdMat4& inout_mMatrix, wdClipSpaceDepthRange::Enum srcDepthRange, wdClipSpaceDepthRange::Enum dstDepthRange)
{
  // exclude identity transformations
  if (srcDepthRange == dstDepthRange)
    return;

  wdVec4 row2 = inout_mMatrix.GetRow(2);
  wdVec4 row3 = inout_mMatrix.GetRow(3);

  // only need to check SrcDepthRange, the rest is the logical conclusion from being not equal
  if (srcDepthRange == wdClipSpaceDepthRange::MinusOneToOne /*&& DstDepthRange == wdClipSpaceDepthRange::ZeroToOne*/)
  {
    // map z => (z + w)/2
    row2 += row3;
    row2 *= 0.5f;
  }
  else // if (SrcDepthRange == wdClipSpaceDepthRange::ZeroToOne && DstDepthRange == wdClipSpaceDepthRange::MinusOneToOne)
  {
    // map z => 2z - w
    row2 += row2;
    row2 -= row3;
  }


  inout_mMatrix.SetRow(2, row2);
  inout_mMatrix.SetRow(3, row3);
}

void wdGraphicsUtils::ExtractPerspectiveMatrixFieldOfView(const wdMat4& mProjectionMatrix, wdAngle& out_fovX, wdAngle& out_fovY)
{

  const wdVec3 row0 = mProjectionMatrix.GetRow(0).GetAsVec3();
  const wdVec3 row1 = mProjectionMatrix.GetRow(1).GetAsVec3();
  const wdVec3 row3 = mProjectionMatrix.GetRow(3).GetAsVec3();

  const wdVec3 leftPlane = (row3 + row0).GetNormalized();
  const wdVec3 rightPlane = (row3 - row0).GetNormalized();
  const wdVec3 bottomPlane = (row3 + row1).GetNormalized();
  const wdVec3 topPlane = (row3 - row1).GetNormalized();

  out_fovX = wdAngle::Radian(wdMath::Pi<float>()) - wdMath::ACos(leftPlane.Dot(rightPlane));
  out_fovY = wdAngle::Radian(wdMath::Pi<float>()) - wdMath::ACos(topPlane.Dot(bottomPlane));
}

void wdGraphicsUtils::ExtractPerspectiveMatrixFieldOfView(const wdMat4& mProjectionMatrix, wdAngle& out_fovLeft, wdAngle& out_fovRight, wdAngle& out_fovBottom, wdAngle& out_fovTop, wdClipSpaceYMode::Enum range)
{
  const wdVec3 row0 = mProjectionMatrix.GetRow(0).GetAsVec3();
  const wdVec3 row1 = mProjectionMatrix.GetRow(1).GetAsVec3();
  const wdVec3 row3 = mProjectionMatrix.GetRow(3).GetAsVec3();

  const wdVec3 leftPlane = (row3 + row0).GetNormalized();
  const wdVec3 rightPlane = (row3 - row0).GetNormalized();
  const wdVec3 bottomPlane = (row3 + row1).GetNormalized();
  const wdVec3 topPlane = (row3 - row1).GetNormalized();

  out_fovLeft = -wdMath::ACos(leftPlane.Dot(wdVec3(1.0f, 0, 0)));
  out_fovRight = wdAngle::Radian(wdMath::Pi<float>()) - wdMath::ACos(rightPlane.Dot(wdVec3(1.0f, 0, 0)));
  out_fovBottom = -wdMath::ACos(bottomPlane.Dot(wdVec3(0, 1.0f, 0)));
  out_fovTop = wdAngle::Radian(wdMath::Pi<float>()) - wdMath::ACos(topPlane.Dot(wdVec3(0, 1.0f, 0)));

  if (range == wdClipSpaceYMode::Flipped)
    wdMath::Swap(out_fovBottom, out_fovTop);
}

wdResult wdGraphicsUtils::ExtractPerspectiveMatrixFieldOfView(const wdMat4& mProjectionMatrix, float& out_fLeft, float& out_fRight, float& out_fBottom, float& out_fTop, wdClipSpaceDepthRange::Enum depthRange, wdClipSpaceYMode::Enum range)
{
  float fNear, fFar;
  WD_SUCCEED_OR_RETURN(ExtractNearAndFarClipPlaneDistances(fNear, fFar, mProjectionMatrix, depthRange));
  // Compensate for inverse-Z.
  const float fMinDepth = wdMath::Min(fNear, fFar);

  wdAngle fFovLeft;
  wdAngle fFovRight;
  wdAngle fFovBottom;
  wdAngle fFovTop;
  ExtractPerspectiveMatrixFieldOfView(mProjectionMatrix, fFovLeft, fFovRight, fFovBottom, fFovTop, range);

  out_fLeft = wdMath::Tan(fFovLeft) * fMinDepth;
  out_fRight = wdMath::Tan(fFovRight) * fMinDepth;
  out_fBottom = wdMath::Tan(fFovBottom) * fMinDepth;
  out_fTop = wdMath::Tan(fFovTop) * fMinDepth;
  return WD_SUCCESS;
}

wdResult wdGraphicsUtils::ExtractNearAndFarClipPlaneDistances(float& out_fNear, float& out_fFar, const wdMat4& mProjectionMatrix, wdClipSpaceDepthRange::Enum depthRange)
{
  const wdVec4 row2 = mProjectionMatrix.GetRow(2);
  const wdVec4 row3 = mProjectionMatrix.GetRow(3);

  wdVec4 nearPlane = row2;

  if (depthRange == wdClipSpaceDepthRange::MinusOneToOne)
  {
    nearPlane += row3;
  }

  const wdVec4 farPlane = row3 - row2;

  const float nearLength = nearPlane.GetAsVec3().GetLength();
  const float farLength = farPlane.GetAsVec3().GetLength();

  const float nearW = wdMath::Abs(nearPlane.w);
  const float farW = wdMath::Abs(farPlane.w);

  if ((nearLength < wdMath::SmallEpsilon<float>() && farLength < wdMath::SmallEpsilon<float>()) ||
      nearW < wdMath::SmallEpsilon<float>() || farW < wdMath::SmallEpsilon<float>())
  {
    return WD_FAILURE;
  }

  const float fNear = nearW / nearLength;
  const float fFar = farW / farLength;

  if (wdMath::IsEqual(fNear, fFar, wdMath::SmallEpsilon<float>()))
  {
    return WD_FAILURE;
  }

  out_fNear = fNear;
  out_fFar = fFar;

  return WD_SUCCESS;
}

wdPlane wdGraphicsUtils::ComputeInterpolatedFrustumPlane(FrustumPlaneInterpolation direction, float fLerpFactor, const wdMat4& mProjectionMatrix, wdClipSpaceDepthRange::Enum depthRange)
{
  wdVec4 rowA;
  wdVec4 rowB = mProjectionMatrix.GetRow(3);
  const float factorMinus1to1 = (fLerpFactor - 0.5f) * 2.0f; // bring into [-1; +1] range

  switch (direction)
  {
    case FrustumPlaneInterpolation::LeftToRight:
    {
      rowA = mProjectionMatrix.GetRow(0);
      rowB *= factorMinus1to1;
      break;
    }

    case FrustumPlaneInterpolation::BottomToTop:
    {
      rowA = mProjectionMatrix.GetRow(1);
      rowB *= factorMinus1to1;
      break;
    }

    case FrustumPlaneInterpolation::NearToFar:
      rowA = mProjectionMatrix.GetRow(2);

      if (depthRange == wdClipSpaceDepthRange::ZeroToOne)
        rowB *= fLerpFactor; // [0; 1] range
      else
        rowB *= factorMinus1to1;
      break;
  }

  wdPlane res;
  res.m_vNormal = rowA.GetAsVec3() - rowB.GetAsVec3();
  res.m_fNegDistance = (rowA.w - rowB.w) / res.m_vNormal.GetLengthAndNormalize();

  return res;
}

wdMat4 wdGraphicsUtils::CreatePerspectiveProjectionMatrix(float fViewWidth, float fViewHeight, float fNearZ, float fFarZ, wdClipSpaceDepthRange::Enum depthRange, wdClipSpaceYMode::Enum range, wdHandedness::Enum handedness)
{
  const float vw = fViewWidth * 0.5f;
  const float vh = fViewHeight * 0.5f;

  return CreatePerspectiveProjectionMatrix(-vw, vw, -vh, vh, fNearZ, fFarZ, depthRange, range, handedness);
}

wdMat4 wdGraphicsUtils::CreatePerspectiveProjectionMatrixFromFovX(wdAngle fieldOfViewX, float fAspectRatioWidthDivHeight, float fNearZ, float fFarZ, wdClipSpaceDepthRange::Enum depthRange, wdClipSpaceYMode::Enum range, wdHandedness::Enum handedness)
{
  // Taking the minimum allows the function to be used to create
  // inverse z matrices (fNearZ > fFarZ) as well.
  const float xm = wdMath::Min(fNearZ, fFarZ) * wdMath::Tan(fieldOfViewX * 0.5f);
  const float ym = xm / fAspectRatioWidthDivHeight;

  return CreatePerspectiveProjectionMatrix(-xm, xm, -ym, ym, fNearZ, fFarZ, depthRange, range, handedness);
}

wdMat4 wdGraphicsUtils::CreatePerspectiveProjectionMatrixFromFovY(wdAngle fieldOfViewY, float fAspectRatioWidthDivHeight, float fNearZ, float fFarZ, wdClipSpaceDepthRange::Enum depthRange, wdClipSpaceYMode::Enum range, wdHandedness::Enum handedness)
{
  // Taking the minimum allows the function to be used to create
  // inverse z matrices (fNearZ > fFarZ) as well.
  const float ym = wdMath::Min(fNearZ, fFarZ) * wdMath::Tan(fieldOfViewY * 0.5);
  const float xm = ym * fAspectRatioWidthDivHeight;

  return CreatePerspectiveProjectionMatrix(-xm, xm, -ym, ym, fNearZ, fFarZ, depthRange, range, handedness);
}

wdMat4 wdGraphicsUtils::CreateOrthographicProjectionMatrix(float fViewWidth, float fViewHeight, float fNearZ, float fFarZ, wdClipSpaceDepthRange::Enum depthRange, wdClipSpaceYMode::Enum range, wdHandedness::Enum handedness)
{
  return CreateOrthographicProjectionMatrix(-fViewWidth * 0.5f, fViewWidth * 0.5f, -fViewHeight * 0.5f, fViewHeight * 0.5f, fNearZ, fFarZ, depthRange, range, handedness);
}

wdMat4 wdGraphicsUtils::CreateOrthographicProjectionMatrix(float fLeft, float fRight, float fBottom, float fTop, float fNearZ, float fFarZ, wdClipSpaceDepthRange::Enum depthRange, wdClipSpaceYMode::Enum range, wdHandedness::Enum handedness)
{
  WD_ASSERT_DEBUG(wdMath::IsFinite(fNearZ) && wdMath::IsFinite(fFarZ), "Infinite plane values are not supported for orthographic projections!");

  wdMat4 res;
  res.SetIdentity();

  if (range == wdClipSpaceYMode::Flipped)
  {
    wdMath::Swap(fBottom, fTop);
  }

  const float fOneDivFarMinusNear = 1.0f / (fFarZ - fNearZ);
  const float fOneDivRightMinusLeft = 1.0f / (fRight - fLeft);
  const float fOneDivTopMinusBottom = 1.0f / (fTop - fBottom);

  res.Element(0, 0) = 2.0f / (fRight - fLeft);

  res.Element(1, 1) = 2.0f / (fTop - fBottom);

  res.Element(3, 0) = -(fLeft + fRight) * fOneDivRightMinusLeft;
  res.Element(3, 1) = -(fTop + fBottom) * fOneDivTopMinusBottom;


  if (depthRange == wdClipSpaceDepthRange::MinusOneToOne)
  {
    // The OpenGL Way: http://wiki.delphigl.com/index.php/glFrustum
    res.Element(2, 2) = -2.0f * fOneDivFarMinusNear;
    res.Element(3, 2) = -(fFarZ + fNearZ) * fOneDivFarMinusNear;
  }
  else
  {
    // The Left-Handed Direct3D Way: https://docs.microsoft.com/windows/win32/direct3d9/d3dxmatrixorthooffcenterlh
    // The Right-Handed Direct3D Way: https://docs.microsoft.com/windows/win32/direct3d9/d3dxmatrixorthooffcenterrh

    res.Element(2, 2) = -1.0f * fOneDivFarMinusNear;
    res.Element(3, 2) = -fNearZ * fOneDivFarMinusNear;
  }

  if (handedness == wdHandedness::LeftHanded)
  {
    res.SetColumn(2, -res.GetColumn(2));
  }

  return res;
}

wdMat4 wdGraphicsUtils::CreatePerspectiveProjectionMatrix(float fLeft, float fRight, float fBottom, float fTop, float fNearZ, float fFarZ, wdClipSpaceDepthRange::Enum depthRange, wdClipSpaceYMode::Enum range, wdHandedness::Enum handedness)
{
  WD_ASSERT_DEBUG(wdMath::IsFinite(fNearZ) || wdMath::IsFinite(fFarZ), "fNearZ and fFarZ cannot both be infinite at the same time!");

  wdMat4 res;
  res.SetZero();

  if (range == wdClipSpaceYMode::Flipped)
  {
    wdMath::Swap(fBottom, fTop);
  }

  // Taking the minimum of the two plane values allows
  // this function to also be used to create inverse-z
  // matrices by specifying values of fNearZ > fFarZ.
  // Otherwise the x and y scaling values will be wrong
  // in the final matrix.
  const float fMinPlane = wdMath::Min(fNearZ, fFarZ);
  const float fTwoNearZ = fMinPlane + fMinPlane;
  const float fOneDivRightMinusLeft = 1.0f / (fRight - fLeft);
  const float fOneDivTopMinusBottom = 1.0f / (fTop - fBottom);

  res.Element(0, 0) = fTwoNearZ * fOneDivRightMinusLeft;

  res.Element(1, 1) = fTwoNearZ * fOneDivTopMinusBottom;

  res.Element(2, 0) = (fLeft + fRight) * fOneDivRightMinusLeft;
  res.Element(2, 1) = (fTop + fBottom) * fOneDivTopMinusBottom;
  res.Element(2, 3) = -1.0f;

  // If either fNearZ or fFarZ is infinite, one can derive the resulting z-transformation by using limit math
  // and letting the respective variable approach infinity in the original expressions for P(2, 2) and P(3, 2).
  // The result is that a couple of terms from the original fraction get reduced to 0 by being divided by infinity,
  // which fortunately yields 1) finite and 2) much simpler expressions for P(2, 2) and P(3, 2).
  if (depthRange == wdClipSpaceDepthRange::MinusOneToOne)
  {
    // The OpenGL Way: http://wiki.delphigl.com/index.php/glFrustum
    // Algebraically reordering the z-row fractions from the above source in a way so infinite fNearZ or fFarZ will zero out
    // instead of producing NaNs due to inf/inf divisions will yield these generalized formulas which could be used instead
    // of the branching below. Insert infinity for either fNearZ or fFarZ to see that these will yield exactly these simplifications:
    //res.Element(2, 2) = 1.f / (fNearZ / fFarZ - 1.f) + 1.f / (1.f - fFarZ / fNearZ);
    //res.Element(3, 2) = 2.f / (1.f / fFarZ - 1.f / fNearZ);
    if (!wdMath::IsFinite(fNearZ))
    {
      res.Element(2, 2) = 1.f;
      res.Element(3, 2) = 2.f * fFarZ;
    }
    else if (!wdMath::IsFinite(fFarZ))
    {
      res.Element(2, 2) = -1.f;
      res.Element(3, 2) = -2.f * fNearZ;
    }
    else
    {
      const float fOneDivNearMinusFar = 1.0f / (fNearZ - fFarZ);
      res.Element(2, 2) = (fFarZ + fNearZ) * fOneDivNearMinusFar;
      res.Element(3, 2) = 2 * fFarZ * fNearZ * fOneDivNearMinusFar;
    }
  }
  else
  {
    // The Left-Handed Direct3D Way: https://docs.microsoft.com/windows/win32/direct3d9/d3dxmatrixperspectiveoffcenterlh
    // The Right-Handed Direct3D Way: https://docs.microsoft.com/windows/win32/direct3d9/d3dxmatrixperspectiveoffcenterrh
    // Algebraically reordering the z-row fractions from the above source in a way so infinite fNearZ or fFarZ will zero out
    // instead of producing NaNs due to inf/inf divisions will yield these generalized formulas which could be used instead
    // of the branching below. Insert infinity for either fNearZ or fFarZ to see that these will yield exactly these simplifications:
    //res.Element(2, 2) = 1.f / (fNearZ / fFarZ - 1.f);
    //res.Element(3, 2) = 1.f / (1.f / fFarZ - 1.f / fNearZ);
    if (!wdMath::IsFinite(fNearZ))
    {
      res.Element(2, 2) = 0.f;
      res.Element(3, 2) = fFarZ;
    }
    else if (!wdMath::IsFinite(fFarZ))
    {
      res.Element(2, 2) = -1.f;
      res.Element(3, 2) = -fNearZ;
    }
    else
    {
      const float fOneDivNearMinusFar = 1.0f / (fNearZ - fFarZ);
      res.Element(2, 2) = fFarZ * fOneDivNearMinusFar;
      res.Element(3, 2) = fFarZ * fNearZ * fOneDivNearMinusFar;
    }
  }

  if (handedness == wdHandedness::LeftHanded)
  {
    res.SetColumn(2, -res.GetColumn(2));
  }

  return res;
}

wdMat3 wdGraphicsUtils::CreateLookAtViewMatrix(const wdVec3& vTarget, const wdVec3& vUpDir, wdHandedness::Enum handedness)
{
  WD_ASSERT_DEBUG(!vTarget.IsZero(), "The target must not be at the origin.");

  wdVec3 vLookDir = vTarget;
  vLookDir.NormalizeIfNotZero(wdVec3::UnitXAxis()).IgnoreResult();

  wdVec3 vNormalizedUpDir = vUpDir.GetNormalized();

  if (wdMath::Abs(vLookDir.Dot(vNormalizedUpDir)) > 0.9999f) // less than 1 degree difference -> problem
  {
    // use some arbitrary other orthogonal vector as UP
    vNormalizedUpDir = vLookDir.GetOrthogonalVector();
  }

  wdMat3 res;

  const wdVec3 zaxis = (handedness == wdHandedness::RightHanded) ? -vLookDir : vLookDir;
  const wdVec3 xaxis = vNormalizedUpDir.CrossRH(zaxis).GetNormalized();
  const wdVec3 yaxis = zaxis.CrossRH(xaxis);

  res.SetRow(0, xaxis);
  res.SetRow(1, yaxis);
  res.SetRow(2, zaxis);

  return res;
}

wdMat3 wdGraphicsUtils::CreateInverseLookAtViewMatrix(const wdVec3& vTarget, const wdVec3& vUpDir, wdHandedness::Enum handedness)
{
  WD_ASSERT_DEBUG(!vTarget.IsZero(), "The target must not be at the origin.");

  wdVec3 vLookDir = vTarget;
  vLookDir.NormalizeIfNotZero(wdVec3::UnitXAxis()).IgnoreResult();

  wdVec3 vNormalizedUpDir = vUpDir.GetNormalized();

  if (wdMath::Abs(vLookDir.Dot(vNormalizedUpDir)) > 0.9999f) // less than 1 degree difference -> problem
  {
    // use some arbitrary other orthogonal vector as UP
    vNormalizedUpDir = vLookDir.GetOrthogonalVector();
  }

  wdMat3 res;

  const wdVec3 zaxis = (handedness == wdHandedness::RightHanded) ? -vLookDir : vLookDir;
  const wdVec3 xaxis = vNormalizedUpDir.CrossRH(zaxis).GetNormalized();
  const wdVec3 yaxis = zaxis.CrossRH(xaxis);

  res.SetColumn(0, xaxis);
  res.SetColumn(1, yaxis);
  res.SetColumn(2, zaxis);

  return res;
}

wdMat4 wdGraphicsUtils::CreateLookAtViewMatrix(const wdVec3& vEyePos, const wdVec3& vLookAtPos, const wdVec3& vUpDir, wdHandedness::Enum handedness)
{
  const wdMat3 rotation = CreateLookAtViewMatrix(vLookAtPos - vEyePos, vUpDir, handedness);

  wdMat4 res;
  res.SetRotationalPart(rotation);
  res.SetTranslationVector(rotation * -vEyePos);
  res.SetRow(3, wdVec4(0, 0, 0, 1));
  return res;
}

wdMat4 wdGraphicsUtils::CreateInverseLookAtViewMatrix(const wdVec3& vEyePos, const wdVec3& vLookAtPos, const wdVec3& vUpDir, wdHandedness::Enum handedness)
{
  const wdMat3 rotation = CreateInverseLookAtViewMatrix(vLookAtPos - vEyePos, vUpDir, handedness);

  wdMat4 res;
  res.SetRotationalPart(rotation);
  res.SetTranslationVector(vEyePos);
  res.SetRow(3, wdVec4(0, 0, 0, 1));
  return res;
}

wdMat4 wdGraphicsUtils::CreateViewMatrix(const wdVec3& vPosition, const wdVec3& vForwardDir, const wdVec3& vRightDir, const wdVec3& vUpDir, wdHandedness::Enum handedness)
{
  wdMat4 res;
  res.SetIdentity();

  wdVec3 xaxis, yaxis, zaxis;

  if (handedness == wdHandedness::LeftHanded)
  {
    xaxis = vRightDir;
    yaxis = vUpDir;
    zaxis = vForwardDir;
  }
  else
  {
    xaxis = vRightDir;
    yaxis = vUpDir;
    zaxis = -vForwardDir;
  }

  res.SetRow(0, xaxis.GetAsVec4(0));
  res.SetRow(1, yaxis.GetAsVec4(0));
  res.SetRow(2, zaxis.GetAsVec4(0));
  res.SetTranslationVector(wdVec3(-xaxis.Dot(vPosition), -yaxis.Dot(vPosition), -zaxis.Dot(vPosition)));

  return res;
}

wdMat4 wdGraphicsUtils::CreateInverseViewMatrix(const wdVec3& vPosition, const wdVec3& vForwardDir, const wdVec3& vRightDir, const wdVec3& vUpDir, wdHandedness::Enum handedness)
{
  wdMat4 res;
  res.SetIdentity();

  wdVec3 xaxis, yaxis, zaxis;

  if (handedness == wdHandedness::LeftHanded)
  {
    xaxis = vRightDir;
    yaxis = vUpDir;
    zaxis = vForwardDir;
  }
  else
  {
    xaxis = vRightDir;
    yaxis = vUpDir;
    zaxis = -vForwardDir;
  }

  res.SetColumn(0, xaxis.GetAsVec4(0));
  res.SetColumn(1, yaxis.GetAsVec4(0));
  res.SetColumn(2, zaxis.GetAsVec4(0));
  res.SetTranslationVector(vPosition);

  return res;
}

void wdGraphicsUtils::DecomposeViewMatrix(wdVec3& ref_vPosition, wdVec3& ref_vForwardDir, wdVec3& ref_vRightDir, wdVec3& ref_vUpDir, const wdMat4& mViewMatrix, wdHandedness::Enum handedness)
{
  const wdMat3 rotation = mViewMatrix.GetRotationalPart();

  if (handedness == wdHandedness::LeftHanded)
  {
    ref_vRightDir = rotation.GetRow(0);
    ref_vUpDir = rotation.GetRow(1);
    ref_vForwardDir = rotation.GetRow(2);
  }
  else
  {
    ref_vRightDir = rotation.GetRow(0);
    ref_vUpDir = rotation.GetRow(1);
    ref_vForwardDir = -rotation.GetRow(2);
  }

  ref_vPosition = rotation.GetTranspose() * -mViewMatrix.GetTranslationVector();
}

wdResult wdGraphicsUtils::ComputeBarycentricCoordinates(wdVec3& out_vCoordinates, const wdVec3& a, const wdVec3& b, const wdVec3& c, const wdVec3& p)
{
  // implementation copied from https://gamedev.stackexchange.com/a/49370

  const wdVec3 v0 = b - a;
  const wdVec3 v1 = c - a;
  const wdVec3 v2 = p - a;

  const float d00 = v0.Dot(v0);
  const float d01 = v0.Dot(v1);
  const float d11 = v1.Dot(v1);
  const float d20 = v2.Dot(v0);
  const float d21 = v2.Dot(v1);
  const float denom = d00 * d11 - d01 * d01;

  if (wdMath::IsZero(denom, wdMath::SmallEpsilon<float>()))
    return WD_FAILURE;

  const float invDenom = 1.0f / denom;

  const float v = (d11 * d20 - d01 * d21) * invDenom;
  const float w = (d00 * d21 - d01 * d20) * invDenom;
  const float u = 1.0f - v - w;

  out_vCoordinates.Set(u, v, w);

  return WD_SUCCESS;
}

wdResult wdGraphicsUtils::ComputeBarycentricCoordinates(wdVec3& out_vCoordinates, const wdVec2& a, const wdVec2& b, const wdVec2& c, const wdVec2& p)
{
  // implementation copied from https://gamedev.stackexchange.com/a/63203

  const wdVec2 v0 = b - a;
  const wdVec2 v1 = c - a;
  const wdVec2 v2 = p - a;

  const float denom = v0.x * v1.y - v1.x * v0.y;

  if (wdMath::IsZero(denom, wdMath::SmallEpsilon<float>()))
    return WD_FAILURE;

  const float invDenom = 1.0f / denom;
  const float v = (v2.x * v1.y - v1.x * v2.y) * invDenom;
  const float w = (v0.x * v2.y - v2.x * v0.y) * invDenom;
  const float u = 1.0f - v - w;

  out_vCoordinates.Set(u, v, w);

  return WD_SUCCESS;
}

WD_STATICLINK_FILE(Foundation, Foundation_Utilities_Implementation_GraphicsUtils);
