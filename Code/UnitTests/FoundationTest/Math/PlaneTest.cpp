#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Math/BoundingBox.h>
#include <Foundation/Math/BoundingSphere.h>
#include <Foundation/Math/Plane.h>
#include <Foundation/Math/Random.h>

NS_CREATE_SIMPLE_TEST(Math, Plane)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Default Constructor")
  {
#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
    if (nsMath::SupportsNaN<nsMat3T::ComponentType>())
    {
      // In debug the default constructor initializes everything with NaN.
      nsPlaneT p;
      NS_TEST_BOOL(nsMath::IsNaN(p.m_vNormal.x) && nsMath::IsNaN(p.m_vNormal.y) && nsMath::IsNaN(p.m_vNormal.z) && nsMath::IsNaN(p.m_fNegDistance));
    }
#else
    // Placement new of the default constructor should not have any effect on the previous data.
    nsPlaneT::ComponentType testBlock[4] = {(nsPlaneT::ComponentType)1, (nsPlaneT::ComponentType)2, (nsPlaneT::ComponentType)3, (nsPlaneT::ComponentType)4};
    nsPlaneT* p = ::new ((void*)&testBlock[0]) nsPlaneT;
    NS_TEST_BOOL(p->m_vNormal.x == (nsPlaneT::ComponentType)1 && p->m_vNormal.y == (nsPlaneT::ComponentType)2 && p->m_vNormal.z == (nsPlaneT::ComponentType)3 && p->m_fNegDistance == (nsPlaneT::ComponentType)4);
#endif
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor(Normal, Point)")
  {
    nsPlaneT p = nsPlane::MakeFromNormalAndPoint(nsVec3T(1, 0, 0), nsVec3T(5, 3, 1));

    NS_TEST_BOOL(p.m_vNormal == nsVec3T(1, 0, 0));
    NS_TEST_FLOAT(p.m_fNegDistance, -5.0f, 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor(Point, Point, Point)")
  {
    nsPlaneT p = nsPlaneT::MakeFromPoints(nsVec3T(-1, 5, 1), nsVec3T(1, 5, 1), nsVec3T(0, 5, -5));

    NS_TEST_VEC3(p.m_vNormal, nsVec3T(0, 1, 0), 0.0001f);
    NS_TEST_FLOAT(p.m_fNegDistance, -5.0f, 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor(Points)")
  {
    nsVec3T v[3] = {nsVec3T(-1, 5, 1), nsVec3T(1, 5, 1), nsVec3T(0, 5, -5)};

    nsPlaneT p;
    p.SetFromPoints(v).AssertSuccess();

    NS_TEST_VEC3(p.m_vNormal, nsVec3T(0, 1, 0), 0.0001f);
    NS_TEST_FLOAT(p.m_fNegDistance, -5.0f, 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor(Points, numpoints)")
  {
    nsVec3T v[6] = {nsVec3T(-1, 5, 1), nsVec3T(-1, 5, 1), nsVec3T(1, 5, 1), nsVec3T(1, 5, 1), nsVec3T(0, 5, -5), nsVec3T(0, 5, -5)};

    nsPlaneT p;
    p.SetFromPoints(v, 6).AssertSuccess();

    NS_TEST_VEC3(p.m_vNormal, nsVec3T(0, 1, 0), 0.0001f);
    NS_TEST_FLOAT(p.m_fNegDistance, -5.0f, 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetFromNormalAndPoint")
  {
    nsPlaneT p;
    p = nsPlane::MakeFromNormalAndPoint(nsVec3T(1, 0, 0), nsVec3T(5, 3, 1));

    NS_TEST_BOOL(p.m_vNormal == nsVec3T(1, 0, 0));
    NS_TEST_FLOAT(p.m_fNegDistance, -5.0f, 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetFromPoints")
  {
    nsPlaneT p;
    p.SetFromPoints(nsVec3T(-1, 5, 1), nsVec3T(1, 5, 1), nsVec3T(0, 5, -5)).IgnoreResult();

    NS_TEST_VEC3(p.m_vNormal, nsVec3T(0, 1, 0), 0.0001f);
    NS_TEST_FLOAT(p.m_fNegDistance, -5.0f, 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetFromPoints")
  {
    nsVec3T v[3] = {nsVec3T(-1, 5, 1), nsVec3T(1, 5, 1), nsVec3T(0, 5, -5)};

    nsPlaneT p;
    p.SetFromPoints(v).IgnoreResult();

    NS_TEST_VEC3(p.m_vNormal, nsVec3T(0, 1, 0), 0.0001f);
    NS_TEST_FLOAT(p.m_fNegDistance, -5.0f, 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetFromPoints")
  {
    nsVec3T v[6] = {nsVec3T(-1, 5, 1), nsVec3T(-1, 5, 1), nsVec3T(1, 5, 1), nsVec3T(1, 5, 1), nsVec3T(0, 5, -5), nsVec3T(0, 5, -5)};

    nsPlaneT p;
    p.SetFromPoints(v, 6).IgnoreResult();

    NS_TEST_VEC3(p.m_vNormal, nsVec3T(0, 1, 0), 0.0001f);
    NS_TEST_FLOAT(p.m_fNegDistance, -5.0f, 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetFromDirections")
  {
    nsPlaneT p;
    p.SetFromDirections(nsVec3T(1, 0, 0), nsVec3T(1, 0, -1), nsVec3T(3, 5, 9)).IgnoreResult();

    NS_TEST_VEC3(p.m_vNormal, nsVec3T(0, 1, 0), 0.0001f);
    NS_TEST_FLOAT(p.m_fNegDistance, -5.0f, 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetInvalid")
  {
    nsPlaneT p;
    p.SetFromDirections(nsVec3T(1, 0, 0), nsVec3T(1, 0, -1), nsVec3T(3, 5, 9)).IgnoreResult();

    p = nsPlaneT::MakeInvalid();

    NS_TEST_VEC3(p.m_vNormal, nsVec3T(0, 0, 0), 0.0001f);
    NS_TEST_FLOAT(p.m_fNegDistance, 0.0f, 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetDistanceTo")
  {
    nsPlaneT p = nsPlane::MakeFromNormalAndPoint(nsVec3T(1, 0, 0), nsVec3T(5, 0, 0));

    NS_TEST_FLOAT(p.GetDistanceTo(nsVec3T(10, 3, 5)), 5.0f, 0.0001f);
    NS_TEST_FLOAT(p.GetDistanceTo(nsVec3T(0, 7, 123)), -5.0f, 0.0001f);
    NS_TEST_FLOAT(p.GetDistanceTo(nsVec3T(5, 12, 23)), 0.0f, 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetMinimumDistanceTo")
  {
    nsVec3T v1[3] = {nsVec3T(15, 3, 5), nsVec3T(6, 7, 123), nsVec3T(10, 12, 23)};
    nsVec3T v2[3] = {nsVec3T(3, 3, 5), nsVec3T(5, 7, 123), nsVec3T(10, 12, 23)};

    nsPlaneT p = nsPlane::MakeFromNormalAndPoint(nsVec3T(1, 0, 0), nsVec3T(5, 0, 0));

    NS_TEST_FLOAT(p.GetMinimumDistanceTo(v1, 3), 1.0f, 0.0001f);
    NS_TEST_FLOAT(p.GetMinimumDistanceTo(v2, 3), -2.0f, 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetMinMaxDistanceTo")
  {
    nsVec3T v1[3] = {nsVec3T(15, 3, 5), nsVec3T(5, 7, 123), nsVec3T(0, 12, 23)};
    nsVec3T v2[3] = {nsVec3T(8, 3, 5), nsVec3T(6, 7, 123), nsVec3T(10, 12, 23)};

    nsPlaneT p = nsPlane::MakeFromNormalAndPoint(nsVec3T(1, 0, 0), nsVec3T(5, 0, 0));

    nsMathTestType fmin, fmax;

    p.GetMinMaxDistanceTo(fmin, fmax, v1, 3);
    NS_TEST_FLOAT(fmin, -5.0f, 0.0001f);
    NS_TEST_FLOAT(fmax, 10.0f, 0.0001f);

    p.GetMinMaxDistanceTo(fmin, fmax, v2, 3);
    NS_TEST_FLOAT(fmin, 1, 0.0001f);
    NS_TEST_FLOAT(fmax, 5, 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetPointPosition")
  {
    nsPlaneT p = nsPlane::MakeFromNormalAndPoint(nsVec3T(0, 1, 0), nsVec3T(0, 10, 0));

    NS_TEST_BOOL(p.GetPointPosition(nsVec3T(0, 15, 0)) == nsPositionOnPlane::Front);
    NS_TEST_BOOL(p.GetPointPosition(nsVec3T(0, 5, 0)) == nsPositionOnPlane::Back);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetPointPosition(planewidth)")
  {
    nsPlaneT p = nsPlane::MakeFromNormalAndPoint(nsVec3T(0, 1, 0), nsVec3T(0, 10, 0));

    NS_TEST_BOOL(p.GetPointPosition(nsVec3T(0, 15, 0), 0.01f) == nsPositionOnPlane::Front);
    NS_TEST_BOOL(p.GetPointPosition(nsVec3T(0, 5, 0), 0.01f) == nsPositionOnPlane::Back);
    NS_TEST_BOOL(p.GetPointPosition(nsVec3T(0, 10, 0), 0.01f) == nsPositionOnPlane::OnPlane);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetObjectPosition")
  {
    nsPlaneT p = nsPlane::MakeFromNormalAndPoint(nsVec3T(1, 0, 0), nsVec3T(10, 0, 0));

    nsVec3T v0[3] = {nsVec3T(12, 0, 0), nsVec3T(15, 0, 0), nsVec3T(20, 0, 0)};
    nsVec3T v1[3] = {nsVec3T(8, 0, 0), nsVec3T(6, 0, 0), nsVec3T(4, 0, 0)};
    nsVec3T v2[3] = {nsVec3T(12, 0, 0), nsVec3T(6, 0, 0), nsVec3T(4, 0, 0)};

    NS_TEST_BOOL(p.GetObjectPosition(v0, 3) == nsPositionOnPlane::Front);
    NS_TEST_BOOL(p.GetObjectPosition(v1, 3) == nsPositionOnPlane::Back);
    NS_TEST_BOOL(p.GetObjectPosition(v2, 3) == nsPositionOnPlane::Spanning);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetObjectPosition(fPlaneHalfWidth)")
  {
    nsPlaneT p = nsPlane::MakeFromNormalAndPoint(nsVec3T(1, 0, 0), nsVec3T(10, 0, 0));

    nsVec3T v0[3] = {nsVec3T(12, 0, 0), nsVec3T(15, 0, 0), nsVec3T(20, 0, 0)};
    nsVec3T v1[3] = {nsVec3T(8, 0, 0), nsVec3T(6, 0, 0), nsVec3T(4, 0, 0)};
    nsVec3T v2[3] = {nsVec3T(12, 0, 0), nsVec3T(6, 0, 0), nsVec3T(4, 0, 0)};
    nsVec3T v3[3] = {nsVec3T(10, 1, 0), nsVec3T(10, 5, 7), nsVec3T(10, 3, -5)};

    NS_TEST_BOOL(p.GetObjectPosition(v0, 3, 0.001f) == nsPositionOnPlane::Front);
    NS_TEST_BOOL(p.GetObjectPosition(v1, 3, 0.001f) == nsPositionOnPlane::Back);
    NS_TEST_BOOL(p.GetObjectPosition(v2, 3, 0.001f) == nsPositionOnPlane::Spanning);
    NS_TEST_BOOL(p.GetObjectPosition(v3, 3, 0.001f) == nsPositionOnPlane::OnPlane);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetObjectPosition(sphere)")
  {
    nsPlaneT p = nsPlane::MakeFromNormalAndPoint(nsVec3T(1, 0, 0), nsVec3T(10, 0, 0));

    NS_TEST_BOOL(p.GetObjectPosition(nsBoundingSphereT::MakeFromCenterAndRadius(nsVec3T(15, 2, 3), 3.0f)) == nsPositionOnPlane::Front);
    NS_TEST_BOOL(p.GetObjectPosition(nsBoundingSphereT::MakeFromCenterAndRadius(nsVec3T(5, 2, 3), 3.0f)) == nsPositionOnPlane::Back);
    NS_TEST_BOOL(p.GetObjectPosition(nsBoundingSphereT::MakeFromCenterAndRadius(nsVec3T(15, 2, 4.999f), 3.0f)) == nsPositionOnPlane::Front);
    NS_TEST_BOOL(p.GetObjectPosition(nsBoundingSphereT::MakeFromCenterAndRadius(nsVec3T(5, 2, 3), 4.999f)) == nsPositionOnPlane::Back);
    NS_TEST_BOOL(p.GetObjectPosition(nsBoundingSphereT::MakeFromCenterAndRadius(nsVec3T(8, 2, 3), 3.0f)) == nsPositionOnPlane::Spanning);
    NS_TEST_BOOL(p.GetObjectPosition(nsBoundingSphereT::MakeFromCenterAndRadius(nsVec3T(12, 2, 3), 3.0f)) == nsPositionOnPlane::Spanning);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetObjectPosition(box)")
  {
    {
      nsPlaneT p = nsPlane::MakeFromNormalAndPoint(nsVec3T(1, 0, 0), nsVec3T(10, 0, 0));
      NS_TEST_BOOL(p.GetObjectPosition(nsBoundingBoxT::MakeFromMinMax(nsVec3T(10.1f), nsVec3T(15))) == nsPositionOnPlane::Front);
      NS_TEST_BOOL(p.GetObjectPosition(nsBoundingBoxT::MakeFromMinMax(nsVec3T(7), nsVec3T(9.9f))) == nsPositionOnPlane::Back);
      NS_TEST_BOOL(p.GetObjectPosition(nsBoundingBoxT::MakeFromMinMax(nsVec3T(7), nsVec3T(15))) == nsPositionOnPlane::Spanning);
    }
    {
      nsPlaneT p = nsPlane::MakeFromNormalAndPoint(nsVec3T(0, 1, 0), nsVec3T(0, 10, 0));
      NS_TEST_BOOL(p.GetObjectPosition(nsBoundingBoxT::MakeFromMinMax(nsVec3T(10.1f), nsVec3T(15))) == nsPositionOnPlane::Front);
      NS_TEST_BOOL(p.GetObjectPosition(nsBoundingBoxT::MakeFromMinMax(nsVec3T(7), nsVec3T(9.9f))) == nsPositionOnPlane::Back);
      NS_TEST_BOOL(p.GetObjectPosition(nsBoundingBoxT::MakeFromMinMax(nsVec3T(7), nsVec3T(15))) == nsPositionOnPlane::Spanning);
    }
    {
      nsPlaneT p = nsPlane::MakeFromNormalAndPoint(nsVec3T(0, 0, 1), nsVec3T(0, 0, 10));
      NS_TEST_BOOL(p.GetObjectPosition(nsBoundingBoxT::MakeFromMinMax(nsVec3T(10.1f), nsVec3T(15))) == nsPositionOnPlane::Front);
      NS_TEST_BOOL(p.GetObjectPosition(nsBoundingBoxT::MakeFromMinMax(nsVec3T(7), nsVec3T(9.9f))) == nsPositionOnPlane::Back);
      NS_TEST_BOOL(p.GetObjectPosition(nsBoundingBoxT::MakeFromMinMax(nsVec3T(7), nsVec3T(15))) == nsPositionOnPlane::Spanning);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ProjectOntoPlane")
  {
    nsPlaneT p = nsPlane::MakeFromNormalAndPoint(nsVec3T(0, 1, 0), nsVec3T(0, 10, 0));

    NS_TEST_VEC3(p.ProjectOntoPlane(nsVec3T(3, 15, 2)), nsVec3T(3, 10, 2), 0.001f);
    NS_TEST_VEC3(p.ProjectOntoPlane(nsVec3T(-1, 5, -5)), nsVec3T(-1, 10, -5), 0.001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Mirror")
  {
    nsPlaneT p = nsPlane::MakeFromNormalAndPoint(nsVec3T(0, 1, 0), nsVec3T(0, 10, 0));

    NS_TEST_VEC3(p.Mirror(nsVec3T(3, 15, 2)), nsVec3T(3, 5, 2), 0.001f);
    NS_TEST_VEC3(p.Mirror(nsVec3T(-1, 5, -5)), nsVec3T(-1, 15, -5), 0.001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetCoplanarDirection")
  {
    nsPlaneT p = nsPlane::MakeFromNormalAndPoint(nsVec3T(0, 1, 0), nsVec3T(0, 10, 0));

    NS_TEST_VEC3(p.GetCoplanarDirection(nsVec3T(0, 1, 0)), nsVec3T(0, 0, 0), 0.001f);
    NS_TEST_VEC3(p.GetCoplanarDirection(nsVec3T(1, 1, 0)).GetNormalized(), nsVec3T(1, 0, 0), 0.001f);
    NS_TEST_VEC3(p.GetCoplanarDirection(nsVec3T(-1, 1, 0)).GetNormalized(), nsVec3T(-1, 0, 0), 0.001f);
    NS_TEST_VEC3(p.GetCoplanarDirection(nsVec3T(0, 1, 1)).GetNormalized(), nsVec3T(0, 0, 1), 0.001f);
    NS_TEST_VEC3(p.GetCoplanarDirection(nsVec3T(0, 1, -1)).GetNormalized(), nsVec3T(0, 0, -1), 0.001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsIdentical / operator== / operator!=")
  {
    nsPlaneT p1 = nsPlane::MakeFromNormalAndPoint(nsVec3T(0, 1, 0), nsVec3T(0, 10, 0));
    nsPlaneT p2 = nsPlane::MakeFromNormalAndPoint(nsVec3T(0, 1, 0), nsVec3T(0, 10, 0));
    nsPlaneT p3 = nsPlane::MakeFromNormalAndPoint(nsVec3T(0, 1, 0), nsVec3T(0, 10.00001f, 0));

    NS_TEST_BOOL(p1.IsIdentical(p1));
    NS_TEST_BOOL(p2.IsIdentical(p2));
    NS_TEST_BOOL(p3.IsIdentical(p3));

    NS_TEST_BOOL(p1.IsIdentical(p2));
    NS_TEST_BOOL(p2.IsIdentical(p1));

    NS_TEST_BOOL(!p1.IsIdentical(p3));
    NS_TEST_BOOL(!p2.IsIdentical(p3));


    NS_TEST_BOOL(p1 == p2);
    NS_TEST_BOOL(p2 == p1);

    NS_TEST_BOOL(p1 != p3);
    NS_TEST_BOOL(p2 != p3);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsEqual")
  {
    nsPlaneT p1 = nsPlane::MakeFromNormalAndPoint(nsVec3T(0, 1, 0), nsVec3T(0, 10, 0));
    nsPlaneT p2 = nsPlane::MakeFromNormalAndPoint(nsVec3T(0, 1, 0), nsVec3T(0, 10, 0));
    nsPlaneT p3 = nsPlane::MakeFromNormalAndPoint(nsVec3T(0, 1, 0), nsVec3T(0, 10.00001f, 0));

    NS_TEST_BOOL(p1.IsEqual(p1));
    NS_TEST_BOOL(p2.IsEqual(p2));
    NS_TEST_BOOL(p3.IsEqual(p3));

    NS_TEST_BOOL(p1.IsEqual(p2));
    NS_TEST_BOOL(p2.IsEqual(p1));

    NS_TEST_BOOL(p1.IsEqual(p3));
    NS_TEST_BOOL(p2.IsEqual(p3));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsValid")
  {
    nsPlaneT p1 = nsPlane::MakeFromNormalAndPoint(nsVec3T(0, 1, 0), nsVec3T(0, 10, 0));

    NS_TEST_BOOL(p1.IsValid());

    p1 = nsPlaneT::MakeInvalid();
    NS_TEST_BOOL(!p1.IsValid());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Transform(Mat3)")
  {
    const float matrixScale[] = {1, 2, 99};

    for (nsUInt32 loopIndex = 0; loopIndex < NS_ARRAY_SIZE(matrixScale); ++loopIndex)
    {
      nsPlaneT p = nsPlane::MakeFromNormalAndPoint(nsVec3T(0, 1, 0), nsVec3T(0, 10, 0));

      nsMat3T m;
      {
        m = nsMat3::MakeRotationX(nsAngle::MakeFromDegree(90));

        nsMat3T rot = nsMat3T::MakeScaling(nsVec3(1) * matrixScale[loopIndex]);
        m = m * rot;
      }

      p.Transform(m);

      NS_TEST_VEC3(p.m_vNormal, nsVec3T(0, 0, 1), 0.0001f);
      NS_TEST_FLOAT(p.m_fNegDistance, -10.0f * matrixScale[loopIndex], 0.0001f);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Transform(Mat4)")
  {
    const float matrixScale[] = {1, 2, 99};

    for (nsUInt32 loopIndex = 0; loopIndex < NS_ARRAY_SIZE(matrixScale); ++loopIndex)
    {
      {
        nsPlaneT p = nsPlane::MakeFromNormalAndPoint(nsVec3T(0, 1, 0), nsVec3T(0, 10, 0));

        nsMat4T m;
        {
          m = nsMat4::MakeRotationX(nsAngle::MakeFromDegree(90));
          m.SetTranslationVector(nsVec3T(0, 5, 0));

          nsMat4T rot = nsMat4T::MakeScaling(nsVec3(1) * matrixScale[loopIndex]);
          m = m * rot;
        }

        p.Transform(m);

        NS_TEST_VEC3(p.m_vNormal, nsVec3T(0, 0, 1), 0.0001f);
        NS_TEST_FLOAT(p.m_fNegDistance, -10.0f * matrixScale[loopIndex], 0.0001f);
      }

      {
        nsPlaneT p = nsPlane::MakeFromNormalAndPoint(nsVec3T(0, 1, 0), nsVec3T(0, 10, 0));

        nsMat4T m;
        {
          m = nsMat4::MakeRotationX(nsAngle::MakeFromDegree(90));
          m.SetTranslationVector(nsVec3T(0, 0, 5));

          nsMat4T rot = nsMat4T::MakeScaling(nsVec3(1) * matrixScale[loopIndex]);
          m = m * rot;
        }

        p.Transform(m);

        NS_TEST_VEC3(p.m_vNormal, nsVec3T(0, 0, 1), 0.0001f);
        NS_TEST_FLOAT(p.m_fNegDistance, -10.0f * matrixScale[loopIndex] - 5.0f, 0.0001f);
      }
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Flip")
  {
    nsPlaneT p = nsPlane::MakeFromNormalAndPoint(nsVec3T(0, 1, 0), nsVec3T(0, 10, 0));

    NS_TEST_VEC3(p.m_vNormal, nsVec3T(0, 1, 0), 0.0001f);
    NS_TEST_FLOAT(p.m_fNegDistance, -10.0f, 0.0001f);

    p.Flip();

    NS_TEST_VEC3(p.m_vNormal, nsVec3T(0, -1, 0), 0.0001f);
    NS_TEST_FLOAT(p.m_fNegDistance, 10.0f, 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "FlipIfNecessary")
  {
    {
      nsPlaneT p = nsPlane::MakeFromNormalAndPoint(nsVec3T(0, 1, 0), nsVec3T(0, 10, 0));

      NS_TEST_VEC3(p.m_vNormal, nsVec3T(0, 1, 0), 0.0001f);
      NS_TEST_FLOAT(p.m_fNegDistance, -10.0f, 0.0001f);

      NS_TEST_BOOL(p.FlipIfNecessary(nsVec3T(0, 11, 0), true) == false);

      NS_TEST_VEC3(p.m_vNormal, nsVec3T(0, 1, 0), 0.0001f);
      NS_TEST_FLOAT(p.m_fNegDistance, -10.0f, 0.0001f);
    }

    {
      nsPlaneT p = nsPlane::MakeFromNormalAndPoint(nsVec3T(0, 1, 0), nsVec3T(0, 10, 0));

      NS_TEST_VEC3(p.m_vNormal, nsVec3T(0, 1, 0), 0.0001f);
      NS_TEST_FLOAT(p.m_fNegDistance, -10.0f, 0.0001f);

      NS_TEST_BOOL(p.FlipIfNecessary(nsVec3T(0, 11, 0), false) == true);

      NS_TEST_VEC3(p.m_vNormal, nsVec3T(0, -1, 0), 0.0001f);
      NS_TEST_FLOAT(p.m_fNegDistance, 10.0f, 0.0001f);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetRayIntersection")
  {
    nsPlaneT p = nsPlane::MakeFromNormalAndPoint(nsVec3T(0, 1, 0), nsVec3T(0, 10, 0));

    nsMathTestType f;
    nsVec3T v;

    NS_TEST_BOOL(p.GetRayIntersection(nsVec3T(3, 1, 7), nsVec3T(0, 1, 0), &f, &v));
    NS_TEST_FLOAT(f, 9, 0.0001f);
    NS_TEST_VEC3(v, nsVec3T(3, 10, 7), 0.0001f);

    NS_TEST_BOOL(p.GetRayIntersection(nsVec3T(3, 20, 7), nsVec3T(0, -1, 0), &f, &v));
    NS_TEST_FLOAT(f, 10, 0.0001f);
    NS_TEST_VEC3(v, nsVec3T(3, 10, 7), 0.0001f);

    NS_TEST_BOOL(!p.GetRayIntersection(nsVec3T(3, 1, 7), nsVec3T(1, 0, 0), &f, &v));
    NS_TEST_BOOL(!p.GetRayIntersection(nsVec3T(3, 1, 7), nsVec3T(0, -1, 0), &f, &v));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetRayIntersectionBiDirectional")
  {
    nsPlaneT p = nsPlane::MakeFromNormalAndPoint(nsVec3T(0, 1, 0), nsVec3T(0, 10, 0));

    nsMathTestType f;
    nsVec3T v;

    NS_TEST_BOOL(p.GetRayIntersectionBiDirectional(nsVec3T(3, 1, 7), nsVec3T(0, 1, 0), &f, &v));
    NS_TEST_FLOAT(f, 9, 0.0001f);
    NS_TEST_VEC3(v, nsVec3T(3, 10, 7), 0.0001f);

    NS_TEST_BOOL(!p.GetRayIntersectionBiDirectional(nsVec3T(3, 1, 7), nsVec3T(1, 0, 0), &f, &v));

    NS_TEST_BOOL(p.GetRayIntersectionBiDirectional(nsVec3T(3, 1, 7), nsVec3T(0, -1, 0), &f, &v));
    NS_TEST_FLOAT(f, -9, 0.0001f);
    NS_TEST_VEC3(v, nsVec3T(3, 10, 7), 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetLineSegmentIntersection")
  {
    nsPlaneT p = nsPlane::MakeFromNormalAndPoint(nsVec3T(0, 1, 0), nsVec3T(0, 10, 0));

    nsMathTestType f;
    nsVec3T v;

    NS_TEST_BOOL(p.GetLineSegmentIntersection(nsVec3T(3, 5, 7), nsVec3T(3, 15, 7), &f, &v));
    NS_TEST_FLOAT(f, 0.5f, 0.0001f);
    NS_TEST_VEC3(v, nsVec3T(3, 10, 7), 0.0001f);

    NS_TEST_BOOL(!p.GetLineSegmentIntersection(nsVec3T(3, 5, 7), nsVec3T(13, 5, 7), &f, &v));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetPlanesIntersectionPoint")
  {
    nsPlaneT p1 = nsPlane::MakeFromNormalAndPoint(nsVec3T(1, 0, 0), nsVec3T(0, 10, 0));
    nsPlaneT p2 = nsPlane::MakeFromNormalAndPoint(nsVec3T(0, 1, 0), nsVec3T(0, 10, 0));
    nsPlaneT p3 = nsPlane::MakeFromNormalAndPoint(nsVec3T(0, 0, 1), nsVec3T(0, 10, 0));

    nsVec3T r;

    NS_TEST_BOOL(nsPlaneT::GetPlanesIntersectionPoint(p1, p2, p3, r) == NS_SUCCESS);
    NS_TEST_VEC3(r, nsVec3T(0, 10, 0), 0.0001f);

    NS_TEST_BOOL(nsPlaneT::GetPlanesIntersectionPoint(p1, p1, p3, r) == NS_FAILURE);
    NS_TEST_BOOL(nsPlaneT::GetPlanesIntersectionPoint(p1, p2, p2, r) == NS_FAILURE);
    NS_TEST_BOOL(nsPlaneT::GetPlanesIntersectionPoint(p3, p2, p3, r) == NS_FAILURE);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "FindSupportPoints")
  {
    nsVec3T v[6] = {nsVec3T(-1, 5, 1), nsVec3T(-1, 5, 1), nsVec3T(1, 5, 1), nsVec3T(1, 5, 1), nsVec3T(0, 5, -5), nsVec3T(0, 5, -5)};

    nsInt32 i1, i2, i3;

    nsPlaneT::FindSupportPoints(v, 6, i1, i2, i3).IgnoreResult();

    NS_TEST_INT(i1, 0);
    NS_TEST_INT(i2, 2);
    NS_TEST_INT(i3, 4);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsNaN")
  {
    if (nsMath::SupportsNaN<nsMathTestType>())
    {
      nsPlaneT p;

      p = nsPlaneT::MakeInvalid();
      NS_TEST_BOOL(!p.IsNaN());

      p = nsPlaneT::MakeInvalid();
      p.m_fNegDistance = nsMath::NaN<nsMathTestType>();
      NS_TEST_BOOL(p.IsNaN());

      p = nsPlaneT::MakeInvalid();
      p.m_vNormal.x = nsMath::NaN<nsMathTestType>();
      NS_TEST_BOOL(p.IsNaN());

      p = nsPlaneT::MakeInvalid();
      p.m_vNormal.y = nsMath::NaN<nsMathTestType>();
      NS_TEST_BOOL(p.IsNaN());

      p = nsPlaneT::MakeInvalid();
      p.m_vNormal.z = nsMath::NaN<nsMathTestType>();
      NS_TEST_BOOL(p.IsNaN());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsFinite")
  {
    if (nsMath::SupportsInfinity<nsMathTestType>())
    {
      nsPlaneT p;

      p.m_vNormal = nsVec3(1, 2, 3).GetNormalized();
      p.m_fNegDistance = 42;
      NS_TEST_BOOL(p.IsValid());
      NS_TEST_BOOL(p.IsFinite());

      p = nsPlaneT::MakeInvalid();
      p.m_vNormal = nsVec3(1, 2, 3).GetNormalized();
      p.m_fNegDistance = nsMath::Infinity<nsMathTestType>();
      NS_TEST_BOOL(p.IsValid());
      NS_TEST_BOOL(!p.IsFinite());

      p = nsPlaneT::MakeInvalid();
      p.m_vNormal.x = nsMath::NaN<nsMathTestType>();
      p.m_fNegDistance = nsMath::Infinity<nsMathTestType>();
      NS_TEST_BOOL(!p.IsValid());
      NS_TEST_BOOL(!p.IsFinite());

      p = nsPlaneT::MakeInvalid();
      p.m_vNormal = nsVec3(1, 2, 3);
      p.m_fNegDistance = 42;
      NS_TEST_BOOL(!p.IsValid());
      NS_TEST_BOOL(p.IsFinite());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetMinimumDistanceTo/GetMaximumDistanceTo")
  {
    const nsUInt32 numTestLoops = 1000 * 1000;

    nsRandom randomGenerator;
    randomGenerator.Initialize(0x83482343);

    const auto randomNonZeroVec3T = [&randomGenerator]() -> nsVec3T
    {
      const float extent = 1000.f;
      const nsVec3T v(randomGenerator.FloatMinMax(-extent, extent), randomGenerator.FloatMinMax(-extent, extent), randomGenerator.FloatMinMax(-extent, extent));
      return v.GetLength() > 0.001f ? v : nsVec3T::MakeAxisX();
    };

    for (nsUInt32 loopIndex = 0; loopIndex < numTestLoops; ++loopIndex)
    {
      const nsPlaneT plane = nsPlane::MakeFromNormalAndPoint(randomNonZeroVec3T().GetNormalized(), randomNonZeroVec3T());

      nsVec3T boxCorners[8];
      nsBoundingBoxT box;
      {
        const nsVec3T boxPoint0 = randomNonZeroVec3T();
        const nsVec3T boxPoint1 = randomNonZeroVec3T();
        const nsVec3T boxMins(nsMath::Min(boxPoint0.x, boxPoint1.x), nsMath::Min(boxPoint0.y, boxPoint1.y), nsMath::Min(boxPoint0.z, boxPoint1.z));
        const nsVec3T boxMaxs(nsMath::Max(boxPoint0.x, boxPoint1.x), nsMath::Max(boxPoint0.y, boxPoint1.y), nsMath::Max(boxPoint0.z, boxPoint1.z));
        box = nsBoundingBoxT::MakeFromMinMax(boxMins, boxMaxs);
        box.GetCorners(boxCorners);
      }

      float distanceMin;
      float distanceMax;
      {
        distanceMin = plane.GetMinimumDistanceTo(box);
        distanceMax = plane.GetMaximumDistanceTo(box);
      }

      float referenceDistanceMin = FLT_MAX;
      float referenceDistanceMax = -FLT_MAX;
      {
        for (nsUInt32 cornerIndex = 0; cornerIndex < NS_ARRAY_SIZE(boxCorners); ++cornerIndex)
        {
          const float cornerDist = plane.GetDistanceTo(boxCorners[cornerIndex]);
          referenceDistanceMin = nsMath::Min(referenceDistanceMin, cornerDist);
          referenceDistanceMax = nsMath::Max(referenceDistanceMax, cornerDist);
        }
      }

      // Break at first error to not spam the log with other potential error (the loop here is very long)
      {
        bool currIterSucceeded = true;
        currIterSucceeded = currIterSucceeded && NS_TEST_FLOAT(distanceMin, referenceDistanceMin, 0.0001f);
        currIterSucceeded = currIterSucceeded && NS_TEST_FLOAT(distanceMax, referenceDistanceMax, 0.0001f);
        if (!currIterSucceeded)
        {
          break;
        }
      }
    }
  }
}
