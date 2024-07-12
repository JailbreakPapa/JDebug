#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Utilities/GraphicsUtils.h>

NS_CREATE_SIMPLE_TEST(Utility, GraphicsUtils)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Perspective (-1/1): ConvertWorldPosToScreenPos / ConvertScreenPosToWorldPos")
  {
    nsMat4 mProj, mProjInv;

    mProj = nsGraphicsUtils::CreatePerspectiveProjectionMatrixFromFovX(
      nsAngle::MakeFromDegree(85.0f), 2.0f, 1.0f, 1000.0f, nsClipSpaceDepthRange::MinusOneToOne, nsClipSpaceYMode::Regular, nsHandedness::LeftHanded);
    mProjInv = mProj.GetInverse();

    for (nsUInt32 y = 0; y < 25; ++y)
    {
      for (nsUInt32 x = 0; x < 50; ++x)
      {
        nsVec3 vPoint, vDir;
        NS_TEST_BOOL(nsGraphicsUtils::ConvertScreenPosToWorldPos(
          mProjInv, 0, 0, 50, 25, nsVec3((float)x, (float)y, 0.5f), vPoint, &vDir, nsClipSpaceDepthRange::MinusOneToOne)
                       .Succeeded());

        NS_TEST_VEC3(vDir, vPoint.GetNormalized(), 0.01f);

        nsVec3 vScreen;
        NS_TEST_BOOL(
          nsGraphicsUtils::ConvertWorldPosToScreenPos(mProj, 0, 0, 50, 25, vPoint, vScreen, nsClipSpaceDepthRange::MinusOneToOne).Succeeded());

        NS_TEST_VEC3(vScreen, nsVec3((float)x, (float)y, 0.5f), 0.01f);
      }
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Perspective (0/1): ConvertWorldPosToScreenPos / ConvertScreenPosToWorldPos")
  {
    nsMat4 mProj, mProjInv;
    mProj = nsGraphicsUtils::CreatePerspectiveProjectionMatrixFromFovX(
      nsAngle::MakeFromDegree(85.0f), 2.0f, 1.0f, 1000.0f, nsClipSpaceDepthRange::ZeroToOne, nsClipSpaceYMode::Regular, nsHandedness::LeftHanded);
    mProjInv = mProj.GetInverse();

    for (nsUInt32 y = 0; y < 25; ++y)
    {
      for (nsUInt32 x = 0; x < 50; ++x)
      {
        nsVec3 vPoint, vDir;
        NS_TEST_BOOL(nsGraphicsUtils::ConvertScreenPosToWorldPos(
          mProjInv, 0, 0, 50, 25, nsVec3((float)x, (float)y, 0.5f), vPoint, &vDir, nsClipSpaceDepthRange::ZeroToOne)
                       .Succeeded());

        NS_TEST_VEC3(vDir, vPoint.GetNormalized(), 0.01f);

        nsVec3 vScreen;
        NS_TEST_BOOL(nsGraphicsUtils::ConvertWorldPosToScreenPos(mProj, 0, 0, 50, 25, vPoint, vScreen, nsClipSpaceDepthRange::ZeroToOne).Succeeded());

        NS_TEST_VEC3(vScreen, nsVec3((float)x, (float)y, 0.5f), 0.01f);
      }
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Ortho (-1/1): ConvertWorldPosToScreenPos / ConvertScreenPosToWorldPos")
  {
    nsMat4 mProj, mProjInv;
    mProj = nsGraphicsUtils::CreateOrthographicProjectionMatrix(
      50, 25, 1.0f, 1000.0f, nsClipSpaceDepthRange::MinusOneToOne, nsClipSpaceYMode::Regular, nsHandedness::LeftHanded);

    mProjInv = mProj.GetInverse();

    for (nsUInt32 y = 0; y < 25; ++y)
    {
      for (nsUInt32 x = 0; x < 50; ++x)
      {
        nsVec3 vPoint, vDir;
        NS_TEST_BOOL(nsGraphicsUtils::ConvertScreenPosToWorldPos(
          mProjInv, 0, 0, 50, 25, nsVec3((float)x, (float)y, 0.5f), vPoint, &vDir, nsClipSpaceDepthRange::MinusOneToOne)
                       .Succeeded());

        NS_TEST_VEC3(vDir, nsVec3(0, 0, 1.0f), 0.01f);

        nsVec3 vScreen;
        NS_TEST_BOOL(
          nsGraphicsUtils::ConvertWorldPosToScreenPos(mProj, 0, 0, 50, 25, vPoint, vScreen, nsClipSpaceDepthRange::MinusOneToOne).Succeeded());

        NS_TEST_VEC3(vScreen, nsVec3((float)x, (float)y, 0.5f), 0.01f);
      }
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Ortho (0/1): ConvertWorldPosToScreenPos / ConvertScreenPosToWorldPos")
  {
    nsMat4 mProj, mProjInv;
    mProj = nsGraphicsUtils::CreateOrthographicProjectionMatrix(
      50, 25, 1.0f, 1000.0f, nsClipSpaceDepthRange::ZeroToOne, nsClipSpaceYMode::Regular, nsHandedness::LeftHanded);
    mProjInv = mProj.GetInverse();

    for (nsUInt32 y = 0; y < 25; ++y)
    {
      for (nsUInt32 x = 0; x < 50; ++x)
      {
        nsVec3 vPoint, vDir;
        NS_TEST_BOOL(nsGraphicsUtils::ConvertScreenPosToWorldPos(
          mProjInv, 0, 0, 50, 25, nsVec3((float)x, (float)y, 0.5f), vPoint, &vDir, nsClipSpaceDepthRange::ZeroToOne)
                       .Succeeded());

        NS_TEST_VEC3(vDir, nsVec3(0, 0, 1.0f), 0.01f);

        nsVec3 vScreen;
        NS_TEST_BOOL(nsGraphicsUtils::ConvertWorldPosToScreenPos(mProj, 0, 0, 50, 25, vPoint, vScreen, nsClipSpaceDepthRange::ZeroToOne).Succeeded());

        NS_TEST_VEC3(vScreen, nsVec3((float)x, (float)y, 0.5f), 0.01f);
      }
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ConvertProjectionMatrixDepthRange")
  {
    nsMat4 mProj1, mProj2;
    mProj1 = nsGraphicsUtils::CreatePerspectiveProjectionMatrixFromFovX(
      nsAngle::MakeFromDegree(85.0f), 2.0f, 1.0f, 1000.0f, nsClipSpaceDepthRange::ZeroToOne, nsClipSpaceYMode::Regular, nsHandedness::LeftHanded);
    mProj2 = nsGraphicsUtils::CreatePerspectiveProjectionMatrixFromFovX(
      nsAngle::MakeFromDegree(85.0f), 2.0f, 1.0f, 1000.0f, nsClipSpaceDepthRange::MinusOneToOne, nsClipSpaceYMode::Regular, nsHandedness::LeftHanded);

    nsMat4 mProj1b = mProj1;
    nsMat4 mProj2b = mProj2;
    nsGraphicsUtils::ConvertProjectionMatrixDepthRange(mProj1b, nsClipSpaceDepthRange::ZeroToOne, nsClipSpaceDepthRange::MinusOneToOne);
    nsGraphicsUtils::ConvertProjectionMatrixDepthRange(mProj2b, nsClipSpaceDepthRange::MinusOneToOne, nsClipSpaceDepthRange::ZeroToOne);

    NS_TEST_BOOL(mProj1.IsEqual(mProj2b, 0.001f));
    NS_TEST_BOOL(mProj2.IsEqual(mProj1b, 0.001f));
  }

  struct DepthRange
  {
    float fNear = 0.0f;
    float fFar = 0.0f;
  };

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ExtractPerspectiveMatrixFieldOfView")
  {
    DepthRange depthRanges[] = {{1.0f, 1000.0f}, {1000.0f, 1.0f}, {0.5f, 20.0f}, {20.0f, 0.5f}};
    nsClipSpaceDepthRange::Enum clipRanges[] = {nsClipSpaceDepthRange::ZeroToOne, nsClipSpaceDepthRange::MinusOneToOne};
    nsHandedness::Enum handednesses[] = {nsHandedness::LeftHanded, nsHandedness::RightHanded};
    nsClipSpaceYMode::Enum clipSpaceYModes[] = {nsClipSpaceYMode::Regular, nsClipSpaceYMode::Flipped};

    for (auto clipSpaceYMode : clipSpaceYModes)
    {
      for (auto handedness : handednesses)
      {
        for (auto depthRange : depthRanges)
        {
          for (auto clipRange : clipRanges)
          {
            for (nsUInt32 angle = 10; angle < 180; angle += 10)
            {
              {
                nsMat4 mProj;
                mProj = nsGraphicsUtils::CreatePerspectiveProjectionMatrixFromFovX(
                  nsAngle::MakeFromDegree((float)angle), 2.0f, depthRange.fNear, depthRange.fFar, clipRange, clipSpaceYMode, handedness);

                nsAngle fovx, fovy;
                nsGraphicsUtils::ExtractPerspectiveMatrixFieldOfView(mProj, fovx, fovy);

                NS_TEST_FLOAT(fovx.GetDegree(), (float)angle, 0.5f);
              }

              {
                nsMat4 mProj;
                mProj = nsGraphicsUtils::CreatePerspectiveProjectionMatrixFromFovY(
                  nsAngle::MakeFromDegree((float)angle), 1.0f / 3.0f, depthRange.fNear, depthRange.fFar, clipRange, clipSpaceYMode, handedness);

                nsAngle fovx, fovy;
                nsGraphicsUtils::ExtractPerspectiveMatrixFieldOfView(mProj, fovx, fovy);

                NS_TEST_FLOAT(fovy.GetDegree(), (float)angle, 0.5f);
              }

              {
                const float fMinDepth = nsMath::Min(depthRange.fNear, depthRange.fFar);
                const nsAngle right = nsAngle::MakeFromDegree((float)angle) / 2;
                const nsAngle top = nsAngle::MakeFromDegree((float)angle) / 2;
                const float fLeft = nsMath::Tan(-right) * fMinDepth;
                const float fRight = nsMath::Tan(right) * fMinDepth * 0.8f;
                const float fBottom = nsMath::Tan(-top) * fMinDepth;
                const float fTop = nsMath::Tan(top) * fMinDepth * 0.7f;

                nsMat4 mProj;
                mProj = nsGraphicsUtils::CreatePerspectiveProjectionMatrix(fLeft, fRight, fBottom, fTop, depthRange.fNear, depthRange.fFar, clipRange, clipSpaceYMode, handedness);

                float fNearOut, fFarOut;
                NS_TEST_BOOL(nsGraphicsUtils::ExtractNearAndFarClipPlaneDistances(fNearOut, fFarOut, mProj, clipRange).Succeeded());
                NS_TEST_FLOAT(depthRange.fNear, fNearOut, 0.1f);
                NS_TEST_FLOAT(depthRange.fFar, fFarOut, 0.1f);

                float fLeftOut, fRightOut, fBottomOut, fTopOut;
                NS_TEST_BOOL(nsGraphicsUtils::ExtractPerspectiveMatrixFieldOfView(mProj, fLeftOut, fRightOut, fBottomOut, fTopOut, clipRange, clipSpaceYMode).Succeeded());
                NS_TEST_FLOAT(fLeft, fLeftOut, nsMath::LargeEpsilon<float>());
                NS_TEST_FLOAT(fRight, fRightOut, nsMath::LargeEpsilon<float>());
                NS_TEST_FLOAT(fBottom, fBottomOut, nsMath::LargeEpsilon<float>());
                NS_TEST_FLOAT(fTop, fTopOut, nsMath::LargeEpsilon<float>());

                nsAngle fFovLeft;
                nsAngle fFovRight;
                nsAngle fFovBottom;
                nsAngle fFovTop;
                nsGraphicsUtils::ExtractPerspectiveMatrixFieldOfView(mProj, fFovLeft, fFovRight, fFovBottom, fFovTop, clipSpaceYMode);

                NS_TEST_FLOAT(fLeft, nsMath::Tan(fFovLeft) * fMinDepth, nsMath::LargeEpsilon<float>());
                NS_TEST_FLOAT(fRight, nsMath::Tan(fFovRight) * fMinDepth, nsMath::LargeEpsilon<float>());
                NS_TEST_FLOAT(fBottom, nsMath::Tan(fFovBottom) * fMinDepth, nsMath::LargeEpsilon<float>());
                NS_TEST_FLOAT(fTop, nsMath::Tan(fFovTop) * fMinDepth, nsMath::LargeEpsilon<float>());
              }
            }
          }
        }
      }
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ExtractNearAndFarClipPlaneDistances")
  {
    DepthRange depthRanges[] = {{0.001f, 100.0f}, {0.01f, 10.0f}, {10.0f, 0.01f}, {1.01f, 110.0f}, {110.0f, 1.01f}};
    nsClipSpaceDepthRange::Enum clipRanges[] = {nsClipSpaceDepthRange::ZeroToOne, nsClipSpaceDepthRange::MinusOneToOne};
    nsHandedness::Enum handednesses[] = {nsHandedness::LeftHanded, nsHandedness::RightHanded};
    nsClipSpaceYMode::Enum clipSpaceYModes[] = {nsClipSpaceYMode::Regular, nsClipSpaceYMode::Flipped};
    nsAngle fovs[] = {nsAngle::MakeFromDegree(10.0f), nsAngle::MakeFromDegree(70.0f)};

    for (auto clipSpaceYMode : clipSpaceYModes)
    {
      for (auto handedness : handednesses)
      {
        for (auto depthRange : depthRanges)
        {
          for (auto clipRange : clipRanges)
          {
            for (auto fov : fovs)
            {
              nsMat4 mProj;
              mProj = nsGraphicsUtils::CreatePerspectiveProjectionMatrixFromFovY(
                fov, 0.7f, depthRange.fNear, depthRange.fFar, clipRange, clipSpaceYMode, handedness);

              float fNearOut, fFarOut;
              NS_TEST_BOOL(nsGraphicsUtils::ExtractNearAndFarClipPlaneDistances(fNearOut, fFarOut, mProj, clipRange).Succeeded());

              NS_TEST_FLOAT(depthRange.fNear, fNearOut, 0.1f);
              NS_TEST_FLOAT(depthRange.fFar, fFarOut, 0.2f);
            }
          }
        }
      }
    }

    { // Test failure on broken projection matrix
      // This matrix has a 0 in the w-component of the third column (invalid perspective divide)
      float vals[] = {0.770734549f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.73205078f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, -1.00000000f, 0.00000000f, 0.000000000, 0.000000000f, -0.100000001f, 0.000000000f};
      nsMat4 mProj;
      memcpy(mProj.m_fElementsCM, vals, 16 * sizeof(float));
      float fNearOut = 0.f, fFarOut = 0.f;
      NS_TEST_BOOL(nsGraphicsUtils::ExtractNearAndFarClipPlaneDistances(fNearOut, fFarOut, mProj, nsClipSpaceDepthRange::MinusOneToOne).Failed());
      NS_TEST_BOOL(fNearOut == 0.0f);
      NS_TEST_BOOL(fFarOut == 0.0f);
    }

    { // Test failure on broken projection matrix
      // This matrix has a 0 in the z-component of the fourth column (one or both projection planes are zero)
      float vals[] = {0.770734549f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, 1.73205078f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f, -1.00000000f, -1.00000000f, 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f};
      nsMat4 mProj;
      memcpy(mProj.m_fElementsCM, vals, 16 * sizeof(float));
      float fNearOut = 0.f, fFarOut = 0.f;
      NS_TEST_BOOL(nsGraphicsUtils::ExtractNearAndFarClipPlaneDistances(fNearOut, fFarOut, mProj, nsClipSpaceDepthRange::MinusOneToOne).Failed());
      NS_TEST_BOOL(fNearOut == 0.0f);
      NS_TEST_BOOL(fFarOut == 0.0f);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ComputeInterpolatedFrustumPlane")
  {
    for (nsUInt32 i = 0; i <= 10; ++i)
    {
      float nearPlane = 1.0f;
      float farPlane = 1000.0f;

      nsMat4 mProj;
      mProj = nsGraphicsUtils::CreatePerspectiveProjectionMatrixFromFovY(
        nsAngle::MakeFromDegree(90.0f), 1.0f, nearPlane, farPlane, nsClipSpaceDepthRange::ZeroToOne, nsClipSpaceYMode::Regular, nsHandedness::LeftHanded);

      const nsPlane horz = nsGraphicsUtils::ComputeInterpolatedFrustumPlane(
        nsGraphicsUtils::FrustumPlaneInterpolation::LeftToRight, i * 0.1f, mProj, nsClipSpaceDepthRange::ZeroToOne);
      const nsPlane vert = nsGraphicsUtils::ComputeInterpolatedFrustumPlane(
        nsGraphicsUtils::FrustumPlaneInterpolation::BottomToTop, i * 0.1f, mProj, nsClipSpaceDepthRange::ZeroToOne);
      const nsPlane forw = nsGraphicsUtils::ComputeInterpolatedFrustumPlane(
        nsGraphicsUtils::FrustumPlaneInterpolation::NearToFar, i * 0.1f, mProj, nsClipSpaceDepthRange::ZeroToOne);

      // Generate clip space point at intersection of the 3 planes and project to worldspace
      nsVec4 clipSpacePoint = nsVec4(0.1f * i * 2 - 1, 0.1f * i * 2 - 1, 0.1f * i, 1);

      nsVec4 worldSpacePoint = mProj.GetInverse() * clipSpacePoint;
      worldSpacePoint /= worldSpacePoint.w;

      NS_TEST_FLOAT(horz.GetDistanceTo(nsVec3::MakeZero()), 0.0f, 0.01f);
      NS_TEST_FLOAT(vert.GetDistanceTo(nsVec3::MakeZero()), 0.0f, 0.01f);

      if (i == 0)
      {
        NS_TEST_FLOAT(forw.GetDistanceTo(nsVec3::MakeZero()), -nearPlane, 0.01f);
      }
      else if (i == 10)
      {
        NS_TEST_FLOAT(forw.GetDistanceTo(nsVec3::MakeZero()), -farPlane, 0.01f);
      }

      NS_TEST_FLOAT(horz.GetDistanceTo(worldSpacePoint.GetAsVec3()), 0.0f, 0.02f);
      NS_TEST_FLOAT(vert.GetDistanceTo(worldSpacePoint.GetAsVec3()), 0.0f, 0.02f);
      NS_TEST_FLOAT(forw.GetDistanceTo(worldSpacePoint.GetAsVec3()), 0.0f, 0.02f);

      // this isn't interpolated linearly across the angle (rotated), so the epsilon has to be very large (just an approx test)
      NS_TEST_FLOAT(horz.m_vNormal.GetAngleBetween(nsVec3(1, 0, 0)).GetDegree(), nsMath::Abs(-45.0f + 90.0f * i * 0.1f), 4.0f);
      NS_TEST_FLOAT(vert.m_vNormal.GetAngleBetween(nsVec3(0, 1, 0)).GetDegree(), nsMath::Abs(-45.0f + 90.0f * i * 0.1f), 4.0f);
      NS_TEST_VEC3(forw.m_vNormal, nsVec3(0, 0, 1), 0.01f);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CreateLookAtViewMatrix / CreateInverseLookAtViewMatrix")
  {
    for (int h = 0; h < 2; ++h)
    {
      const nsHandedness::Enum handedness = (h == 0) ? nsHandedness::LeftHanded : nsHandedness::RightHanded;

      {
        nsMat3 mLook3 = nsGraphicsUtils::CreateLookAtViewMatrix(nsVec3(1, 0, 0), nsVec3(0, 0, 1), handedness);
        nsMat3 mLookInv3 = nsGraphicsUtils::CreateInverseLookAtViewMatrix(nsVec3(1, 0, 0), nsVec3(0, 0, 1), handedness);

        NS_TEST_BOOL((mLook3 * mLookInv3).IsIdentity(0.01f));

        nsMat4 mLook4 = nsGraphicsUtils::CreateLookAtViewMatrix(nsVec3(0), nsVec3(1, 0, 0), nsVec3(0, 0, 1), handedness);
        nsMat4 mLookInv4 = nsGraphicsUtils::CreateInverseLookAtViewMatrix(nsVec3(0), nsVec3(1, 0, 0), nsVec3(0, 0, 1), handedness);

        NS_TEST_BOOL((mLook4 * mLookInv4).IsIdentity(0.01f));

        NS_TEST_BOOL(mLook3.IsEqual(mLook4.GetRotationalPart(), 0.01f));
        NS_TEST_BOOL(mLookInv3.IsEqual(mLookInv4.GetRotationalPart(), 0.01f));
      }

      {
        nsMat4 mLook4 = nsGraphicsUtils::CreateLookAtViewMatrix(nsVec3(1, 2, 0), nsVec3(4, 5, 0), nsVec3(0, 0, 1), handedness);
        nsMat4 mLookInv4 = nsGraphicsUtils::CreateInverseLookAtViewMatrix(nsVec3(1, 2, 0), nsVec3(4, 5, 0), nsVec3(0, 0, 1), handedness);

        NS_TEST_BOOL((mLook4 * mLookInv4).IsIdentity(0.01f));
      }
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CreateViewMatrix / DecomposeViewMatrix / CreateInverseViewMatrix")
  {
    for (int h = 0; h < 2; ++h)
    {
      const nsHandedness::Enum handedness = (h == 0) ? nsHandedness::LeftHanded : nsHandedness::RightHanded;

      const nsVec3 vEye(0);
      const nsVec3 vTarget(0, 0, 1);
      const nsVec3 vUp0(0, 1, 0);
      const nsVec3 vFwd = (vTarget - vEye).GetNormalized();
      nsVec3 vRight = vUp0.CrossRH(vFwd).GetNormalized();
      const nsVec3 vUp = vFwd.CrossRH(vRight).GetNormalized();

      if (handedness == nsHandedness::RightHanded)
        vRight = -vRight;

      const nsMat4 mLookAt = nsGraphicsUtils::CreateLookAtViewMatrix(vEye, vTarget, vUp0, handedness);

      nsVec3 decFwd, decRight, decUp, decPos;
      nsGraphicsUtils::DecomposeViewMatrix(decPos, decFwd, decRight, decUp, mLookAt, handedness);

      NS_TEST_VEC3(decPos, vEye, 0.01f);
      NS_TEST_VEC3(decFwd, vFwd, 0.01f);
      NS_TEST_VEC3(decUp, vUp, 0.01f);
      NS_TEST_VEC3(decRight, vRight, 0.01f);

      const nsMat4 mView = nsGraphicsUtils::CreateViewMatrix(decPos, decFwd, decRight, decUp, handedness);
      const nsMat4 mViewInv = nsGraphicsUtils::CreateInverseViewMatrix(decPos, decFwd, decRight, decUp, handedness);

      NS_TEST_BOOL(mLookAt.IsEqual(mView, 0.01f));

      NS_TEST_BOOL((mLookAt * mViewInv).IsIdentity());
    }
  }
}
