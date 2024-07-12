#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Math/Frustum.h>
#include <Foundation/SimdMath/SimdConversion.h>
#include <Foundation/Utilities/GraphicsUtils.h>

NS_CREATE_SIMPLE_TEST(Math, Frustum)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeFromPlanes")
  {
    nsFrustum f;

    nsPlane p[6];
    p[nsFrustum::PlaneType::LeftPlane] = nsPlane::MakeFromNormalAndPoint(nsVec3(-1, 0, 0), nsVec3(-2, 0, 0));
    p[nsFrustum::PlaneType::RightPlane] = nsPlane::MakeFromNormalAndPoint(nsVec3(+1, 0, 0), nsVec3(+2, 0, 0));
    p[nsFrustum::PlaneType::BottomPlane] = nsPlane::MakeFromNormalAndPoint(nsVec3(0, -1, 0), nsVec3(0, -2, 0));
    p[nsFrustum::PlaneType::TopPlane] = nsPlane::MakeFromNormalAndPoint(nsVec3(0, +1, 0), nsVec3(0, +2, 0));
    p[nsFrustum::PlaneType::NearPlane] = nsPlane::MakeFromNormalAndPoint(nsVec3(0, 0, -1), nsVec3(0, 0, 0));
    p[nsFrustum::PlaneType::FarPlane] = nsPlane::MakeFromNormalAndPoint(nsVec3(0, 0, 1), nsVec3(0, 0, 100));

    f = nsFrustum::MakeFromPlanes(p);

    NS_TEST_BOOL(f.GetPlane(0) == p[0]);
    NS_TEST_BOOL(f.GetPlane(1) == p[1]);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "TransformFrustum/GetTransformedFrustum")
  {
    nsFrustum f;

    nsPlane p[6];
    p[nsFrustum::PlaneType::LeftPlane] = nsPlane::MakeFromNormalAndPoint(nsVec3(-1, 0, 0), nsVec3(-2, 0, 0));
    p[nsFrustum::PlaneType::RightPlane] = nsPlane::MakeFromNormalAndPoint(nsVec3(+1, 0, 0), nsVec3(+2, 0, 0));
    p[nsFrustum::PlaneType::BottomPlane] = nsPlane::MakeFromNormalAndPoint(nsVec3(0, -1, 0), nsVec3(0, -2, 0));
    p[nsFrustum::PlaneType::TopPlane] = nsPlane::MakeFromNormalAndPoint(nsVec3(0, +1, 0), nsVec3(0, +2, 0));
    p[nsFrustum::PlaneType::NearPlane] = nsPlane::MakeFromNormalAndPoint(nsVec3(0, 0, -1), nsVec3(0, 0, 0));
    p[nsFrustum::PlaneType::FarPlane] = nsPlane::MakeFromNormalAndPoint(nsVec3(0, 0, 1), nsVec3(0, 0, 100));

    f = nsFrustum::MakeFromPlanes(p);

    nsMat4 mTransform;
    mTransform = nsMat4::MakeRotationY(nsAngle::MakeFromDegree(90.0f));
    mTransform.SetTranslationVector(nsVec3(2, 3, 4));

    nsFrustum tf = f;
    tf.TransformFrustum(mTransform);

    p[0].Transform(mTransform);
    p[1].Transform(mTransform);

    for (int planeIndex = 0; planeIndex < 6; ++planeIndex)
    {
      NS_TEST_BOOL(f.GetTransformedFrustum(mTransform).GetPlane(planeIndex) == tf.GetPlane(planeIndex));
    }

    NS_TEST_BOOL(tf.GetPlane(0).IsEqual(p[0], 0.001f));
    NS_TEST_BOOL(tf.GetPlane(1).IsEqual(p[1], 0.001f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "InvertFrustum")
  {
    nsFrustum f;

    nsPlane p[6];
    p[nsFrustum::PlaneType::LeftPlane] = nsPlane::MakeFromNormalAndPoint(nsVec3(-1, 0, 0), nsVec3(-2, 0, 0));
    p[nsFrustum::PlaneType::RightPlane] = nsPlane::MakeFromNormalAndPoint(nsVec3(+1, 0, 0), nsVec3(+2, 0, 0));
    p[nsFrustum::PlaneType::BottomPlane] = nsPlane::MakeFromNormalAndPoint(nsVec3(0, -1, 0), nsVec3(0, -2, 0));
    p[nsFrustum::PlaneType::TopPlane] = nsPlane::MakeFromNormalAndPoint(nsVec3(0, +1, 0), nsVec3(0, +2, 0));
    p[nsFrustum::PlaneType::NearPlane] = nsPlane::MakeFromNormalAndPoint(nsVec3(0, 0, -1), nsVec3(0, 0, 0));
    p[nsFrustum::PlaneType::FarPlane] = nsPlane::MakeFromNormalAndPoint(nsVec3(0, 0, 1), nsVec3(0, 0, 100));

    f = nsFrustum::MakeFromPlanes(p);

    f.InvertFrustum();

    p[0].Flip();
    p[1].Flip();

    NS_TEST_BOOL(f.GetPlane(0) == p[0]);
    NS_TEST_BOOL(f.GetPlane(1) == p[1]);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetFrustum")
  {
    // check that the extracted frustum planes are always the same, no matter the handedness or depth-range

    // test the different depth ranges
    for (int r = 0; r < 2; ++r)
    {
      const nsClipSpaceDepthRange::Enum range = (r == 0) ? nsClipSpaceDepthRange::MinusOneToOne : nsClipSpaceDepthRange::ZeroToOne;

      // test rotated model-view matrices
      for (int rot = 0; rot < 360; rot += 45)
      {
        nsVec3 vLookDir;
        vLookDir.Set(nsMath::Sin(nsAngle::MakeFromDegree((float)rot)), 0, -nsMath::Cos(nsAngle::MakeFromDegree((float)rot)));

        nsVec3 vRightDir;
        vRightDir.Set(nsMath::Sin(nsAngle::MakeFromDegree(rot + 90.0f)), 0, -nsMath::Cos(nsAngle::MakeFromDegree(rot + 90.0f)));

        const nsVec3 vCamPos(rot * 1.0f, rot * 0.5f, rot * -0.3f);

        // const nsMat4 mViewLH = nsGraphicsUtils::CreateViewMatrix(vCamPos, vLookDir, -vRightDir, nsVec3(0, 1, 0), nsHandedness::LeftHanded);
        // const nsMat4 mViewRH = nsGraphicsUtils::CreateViewMatrix(vCamPos, vLookDir, vRightDir, nsVec3(0, 1, 0), nsHandedness::RightHanded);
        const nsMat4 mViewLH = nsGraphicsUtils::CreateLookAtViewMatrix(vCamPos, vCamPos + vLookDir, nsVec3(0, 1, 0), nsHandedness::LeftHanded);
        const nsMat4 mViewRH = nsGraphicsUtils::CreateLookAtViewMatrix(vCamPos, vCamPos + vLookDir, nsVec3(0, 1, 0), nsHandedness::RightHanded);

        const nsMat4 mProjLH = nsGraphicsUtils::CreatePerspectiveProjectionMatrixFromFovY(
          nsAngle::MakeFromDegree(90), 1.0f, 1.0f, 100.0f, range, nsClipSpaceYMode::Regular, nsHandedness::LeftHanded);
        const nsMat4 mProjRH = nsGraphicsUtils::CreatePerspectiveProjectionMatrixFromFovY(
          nsAngle::MakeFromDegree(90), 1.0f, 1.0f, 100.0f, range, nsClipSpaceYMode::Regular, nsHandedness::RightHanded);

        const nsMat4 mViewProjLH = mProjLH * mViewLH;
        const nsMat4 mViewProjRH = mProjRH * mViewRH;

        nsFrustum fB;
        const nsFrustum fLH = nsFrustum::MakeFromMVP(mViewProjLH, range, nsHandedness::LeftHanded);
        const nsFrustum fRH = nsFrustum::MakeFromMVP(mViewProjRH, range, nsHandedness::RightHanded);

        fB = nsFrustum::MakeFromFOV(vCamPos, vLookDir, nsVec3(0, 1, 0), nsAngle::MakeFromDegree(90), nsAngle::MakeFromDegree(90), 1.0f, 100.0f);

        NS_TEST_BOOL(fRH.GetPlane(nsFrustum::NearPlane).IsEqual(fB.GetPlane(nsFrustum::NearPlane), 0.1f));
        NS_TEST_BOOL(fRH.GetPlane(nsFrustum::LeftPlane).IsEqual(fB.GetPlane(nsFrustum::LeftPlane), 0.1f));
        NS_TEST_BOOL(fRH.GetPlane(nsFrustum::RightPlane).IsEqual(fB.GetPlane(nsFrustum::RightPlane), 0.1f));
        NS_TEST_BOOL(fRH.GetPlane(nsFrustum::FarPlane).IsEqual(fB.GetPlane(nsFrustum::FarPlane), 0.1f));
        NS_TEST_BOOL(fRH.GetPlane(nsFrustum::BottomPlane).IsEqual(fB.GetPlane(nsFrustum::BottomPlane), 0.1f));
        NS_TEST_BOOL(fRH.GetPlane(nsFrustum::TopPlane).IsEqual(fB.GetPlane(nsFrustum::TopPlane), 0.1f));

        NS_TEST_BOOL(fLH.GetPlane(nsFrustum::NearPlane).IsEqual(fB.GetPlane(nsFrustum::NearPlane), 0.1f));
        NS_TEST_BOOL(fLH.GetPlane(nsFrustum::LeftPlane).IsEqual(fB.GetPlane(nsFrustum::LeftPlane), 0.1f));
        NS_TEST_BOOL(fLH.GetPlane(nsFrustum::RightPlane).IsEqual(fB.GetPlane(nsFrustum::RightPlane), 0.1f));
        NS_TEST_BOOL(fLH.GetPlane(nsFrustum::FarPlane).IsEqual(fB.GetPlane(nsFrustum::FarPlane), 0.1f));
        NS_TEST_BOOL(fLH.GetPlane(nsFrustum::BottomPlane).IsEqual(fB.GetPlane(nsFrustum::BottomPlane), 0.1f));
        NS_TEST_BOOL(fLH.GetPlane(nsFrustum::TopPlane).IsEqual(fB.GetPlane(nsFrustum::TopPlane), 0.1f));
      }
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Culling")
  {
    const nsVec3 offsetPos(23, 17, -9);
    const nsVec3 camDir[6] = {nsVec3(-1, 0, 0), nsVec3(1, 0, 0), nsVec3(0, -1, 0), nsVec3(0, 1, 0), nsVec3(0, 0, -1), nsVec3(0, 0, 1)};
    const nsVec3 objPos[6] = {nsVec3(-9, 0, 0), nsVec3(9, 0, 0), nsVec3(0, -9, 0), nsVec3(0, 9, 0), nsVec3(0, 0, -9), nsVec3(0, 0, 9)};

    for (nsUInt32 dir = 0; dir < 6; ++dir)
    {
      nsFrustum fDir;
      fDir = nsFrustum::MakeFromFOV(offsetPos, camDir[dir], camDir[dir].GetOrthogonalVector() /*arbitrary*/, nsAngle::MakeFromDegree(90), nsAngle::MakeFromDegree(90), 1.0f, 100.0f);

      for (nsUInt32 obj = 0; obj < 6; ++obj)
      {
        // box
        {
          nsBoundingBox boundingObj;
          boundingObj = nsBoundingBox::MakeFromCenterAndHalfExtents(offsetPos + objPos[obj], nsVec3(1.0f));

          const nsVolumePosition::Enum res = fDir.GetObjectPosition(boundingObj);

          if (obj == dir)
            NS_TEST_BOOL(res == nsVolumePosition::Inside);
          else
            NS_TEST_BOOL(res == nsVolumePosition::Outside);
        }

        // sphere
        {
          nsBoundingSphere boundingObj = nsBoundingSphere::MakeFromCenterAndRadius(offsetPos + objPos[obj], 0.93f);

          const nsVolumePosition::Enum res = fDir.GetObjectPosition(boundingObj);

          if (obj == dir)
            NS_TEST_BOOL(res == nsVolumePosition::Inside);
          else
            NS_TEST_BOOL(res == nsVolumePosition::Outside);
        }

        // vertices
        {
          nsBoundingBox boundingObj;
          boundingObj = nsBoundingBox::MakeFromCenterAndHalfExtents(offsetPos + objPos[obj], nsVec3(1.0f));

          nsVec3 vertices[8];
          boundingObj.GetCorners(vertices);

          const nsVolumePosition::Enum res = fDir.GetObjectPosition(vertices, 8);

          if (obj == dir)
            NS_TEST_BOOL(res == nsVolumePosition::Inside);
          else
            NS_TEST_BOOL(res == nsVolumePosition::Outside);
        }

        // vertices + transform
        {
          nsBoundingBox boundingObj;
          boundingObj = nsBoundingBox::MakeFromCenterAndHalfExtents(objPos[obj], nsVec3(1.0f));

          nsVec3 vertices[8];
          boundingObj.GetCorners(vertices);

          nsMat4 transform = nsMat4::MakeTranslation(offsetPos);

          const nsVolumePosition::Enum res = fDir.GetObjectPosition(vertices, 8, transform);

          if (obj == dir)
            NS_TEST_BOOL(res == nsVolumePosition::Inside);
          else
            NS_TEST_BOOL(res == nsVolumePosition::Outside);
        }

        // SIMD box
        {
          nsBoundingBox boundingObj;
          boundingObj = nsBoundingBox::MakeFromCenterAndHalfExtents(offsetPos + objPos[obj], nsVec3(1.0f));

          const bool res = fDir.Overlaps(nsSimdConversion::ToBBox(boundingObj));

          if (obj == dir)
            NS_TEST_BOOL(res == true);
          else
            NS_TEST_BOOL(res == false);
        }

        // SIMD sphere
        {
          nsBoundingSphere boundingObj = nsBoundingSphere::MakeFromCenterAndRadius(offsetPos + objPos[obj], 0.93f);

          const bool res = fDir.Overlaps(nsSimdConversion::ToBSphere(boundingObj));

          if (obj == dir)
            NS_TEST_BOOL(res == true);
          else
            NS_TEST_BOOL(res == false);
        }
      }
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ComputeCornerPoints")
  {
    const nsMat4 mProj = nsGraphicsUtils::CreatePerspectiveProjectionMatrixFromFovY(
      nsAngle::MakeFromDegree(90), 1.0f, 1.0f, 10.0f, nsClipSpaceDepthRange::MinusOneToOne, nsClipSpaceYMode::Regular, nsHandedness::RightHanded);

    nsFrustum frustum[2];
    frustum[0] = nsFrustum::MakeFromMVP(mProj, nsClipSpaceDepthRange::MinusOneToOne, nsHandedness::RightHanded);
    frustum[1] = nsFrustum::MakeFromFOV(nsVec3::MakeZero(), nsVec3(0, 0, -1), nsVec3(0, 1, 0), nsAngle::MakeFromDegree(90), nsAngle::MakeFromDegree(90), 1.0f, 10.0f);

    for (int f = 0; f < 2; ++f)
    {
      nsVec3 corner[8];
      frustum[f].ComputeCornerPoints(corner).AssertSuccess();

      nsPositionOnPlane::Enum results[8][6];

      for (int c = 0; c < 8; ++c)
      {
        for (int p = 0; p < 6; ++p)
        {
          results[c][p] = nsPositionOnPlane::Back;
        }
      }

      results[nsFrustum::FrustumCorner::NearTopLeft][nsFrustum::PlaneType::NearPlane] = nsPositionOnPlane::OnPlane;
      results[nsFrustum::FrustumCorner::NearTopLeft][nsFrustum::PlaneType::TopPlane] = nsPositionOnPlane::OnPlane;
      results[nsFrustum::FrustumCorner::NearTopLeft][nsFrustum::PlaneType::LeftPlane] = nsPositionOnPlane::OnPlane;

      results[nsFrustum::FrustumCorner::NearTopRight][nsFrustum::PlaneType::NearPlane] = nsPositionOnPlane::OnPlane;
      results[nsFrustum::FrustumCorner::NearTopRight][nsFrustum::PlaneType::TopPlane] = nsPositionOnPlane::OnPlane;
      results[nsFrustum::FrustumCorner::NearTopRight][nsFrustum::PlaneType::RightPlane] = nsPositionOnPlane::OnPlane;

      results[nsFrustum::FrustumCorner::NearBottomLeft][nsFrustum::PlaneType::NearPlane] = nsPositionOnPlane::OnPlane;
      results[nsFrustum::FrustumCorner::NearBottomLeft][nsFrustum::PlaneType::BottomPlane] = nsPositionOnPlane::OnPlane;
      results[nsFrustum::FrustumCorner::NearBottomLeft][nsFrustum::PlaneType::LeftPlane] = nsPositionOnPlane::OnPlane;

      results[nsFrustum::FrustumCorner::NearBottomRight][nsFrustum::PlaneType::NearPlane] = nsPositionOnPlane::OnPlane;
      results[nsFrustum::FrustumCorner::NearBottomRight][nsFrustum::PlaneType::BottomPlane] = nsPositionOnPlane::OnPlane;
      results[nsFrustum::FrustumCorner::NearBottomRight][nsFrustum::PlaneType::RightPlane] = nsPositionOnPlane::OnPlane;

      results[nsFrustum::FrustumCorner::FarTopLeft][nsFrustum::PlaneType::FarPlane] = nsPositionOnPlane::OnPlane;
      results[nsFrustum::FrustumCorner::FarTopLeft][nsFrustum::PlaneType::TopPlane] = nsPositionOnPlane::OnPlane;
      results[nsFrustum::FrustumCorner::FarTopLeft][nsFrustum::PlaneType::LeftPlane] = nsPositionOnPlane::OnPlane;

      results[nsFrustum::FrustumCorner::FarTopRight][nsFrustum::PlaneType::FarPlane] = nsPositionOnPlane::OnPlane;
      results[nsFrustum::FrustumCorner::FarTopRight][nsFrustum::PlaneType::TopPlane] = nsPositionOnPlane::OnPlane;
      results[nsFrustum::FrustumCorner::FarTopRight][nsFrustum::PlaneType::RightPlane] = nsPositionOnPlane::OnPlane;

      results[nsFrustum::FrustumCorner::FarBottomLeft][nsFrustum::PlaneType::FarPlane] = nsPositionOnPlane::OnPlane;
      results[nsFrustum::FrustumCorner::FarBottomLeft][nsFrustum::PlaneType::BottomPlane] = nsPositionOnPlane::OnPlane;
      results[nsFrustum::FrustumCorner::FarBottomLeft][nsFrustum::PlaneType::LeftPlane] = nsPositionOnPlane::OnPlane;

      results[nsFrustum::FrustumCorner::FarBottomRight][nsFrustum::PlaneType::FarPlane] = nsPositionOnPlane::OnPlane;
      results[nsFrustum::FrustumCorner::FarBottomRight][nsFrustum::PlaneType::BottomPlane] = nsPositionOnPlane::OnPlane;
      results[nsFrustum::FrustumCorner::FarBottomRight][nsFrustum::PlaneType::RightPlane] = nsPositionOnPlane::OnPlane;

      for (int c = 0; c < 8; ++c)
      {
        nsFrustum::FrustumCorner cornerName = (nsFrustum::FrustumCorner)c;

        for (int p = 0; p < 6; ++p)
        {
          nsFrustum::PlaneType planeName = (nsFrustum::PlaneType)p;

          nsPlane plane = frustum[f].GetPlane(planeName);
          nsPositionOnPlane::Enum expected = results[cornerName][planeName];
          nsPositionOnPlane::Enum result = plane.GetPointPosition(corner[cornerName], 0.1f);
          // float fDistToPlane = plane.GetDistanceTo(corner[cornerName]);
          NS_TEST_BOOL(result == expected);
        }
      }
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeFromCorners")
  {
    const nsFrustum fOrg = nsFrustum::MakeFromFOV(nsVec3(1, 2, 3), nsVec3(1, 1, 0).GetNormalized(), nsVec3(0, 0, 1).GetNormalized(), nsAngle::MakeFromDegree(110), nsAngle::MakeFromDegree(70), 0.1f, 100.0f);

    nsVec3 corners[8];
    fOrg.ComputeCornerPoints(corners).AssertSuccess();

    const nsFrustum fNew = nsFrustum::MakeFromCorners(corners);

    for (nsUInt32 i = 0; i < 6; ++i)
    {
      nsPlane p1 = fOrg.GetPlane(i);
      nsPlane p2 = fNew.GetPlane(i);

      NS_TEST_BOOL(p1.IsEqual(p2, nsMath::LargeEpsilon<float>()));
    }

    nsVec3 corners2[8];
    fNew.ComputeCornerPoints(corners2).AssertSuccess();

    for (nsUInt32 i = 0; i < 8; ++i)
    {
      NS_TEST_BOOL(corners[i].IsEqual(corners2[i], 0.01f));
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeFromMVPInfiniteFarPlane")
  {
    nsMat4 perspective = nsGraphicsUtils::CreatePerspectiveProjectionMatrixFromFovY(nsAngle::MakeFromDegree(90), 1.0f, nsMath::Infinity<float>(), 100.0f, nsClipSpaceDepthRange::ZeroToOne, nsClipSpaceYMode::Regular, nsHandedness::RightHanded);

    auto frustum = nsFrustum::MakeFromMVP(perspective);
    NS_TEST_BOOL(frustum.IsValid());
  }
}
