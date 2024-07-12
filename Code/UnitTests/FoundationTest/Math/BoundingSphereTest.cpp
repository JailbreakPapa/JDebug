#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Math/BoundingBox.h>
#include <Foundation/Math/BoundingSphere.h>

NS_CREATE_SIMPLE_TEST(Math, BoundingSphere)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor")
  {
    nsBoundingSphereT s = nsBoundingSphereT::MakeFromCenterAndRadius(nsVec3T(1, 2, 3), 4);

    NS_TEST_BOOL(s.m_vCenter == nsVec3T(1, 2, 3));
    NS_TEST_BOOL(s.m_fRadius == 4.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetInvalid / IsValid")
  {
    nsBoundingSphereT s = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3T(1, 2, 3), 4);

    NS_TEST_BOOL(s.IsValid());

    s = nsBoundingSphereT::MakeInvalid();

    NS_TEST_BOOL(!s.IsValid());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeZero / IsZero")
  {
    nsBoundingSphereT s = nsBoundingSphereT::MakeZero();

    NS_TEST_BOOL(s.IsValid());
    NS_TEST_BOOL(s.m_vCenter.IsZero());
    NS_TEST_BOOL(s.m_fRadius == 0.0f);
    NS_TEST_BOOL(s.IsZero());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetElements")
  {
    nsBoundingSphereT s = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3T(1, 2, 3), 4);

    NS_TEST_BOOL(s.m_vCenter == nsVec3T(1, 2, 3));
    NS_TEST_BOOL(s.m_fRadius == 4);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetFromPoints")
  {


    nsVec3T p[4] = {nsVec3T(2, 6, 0), nsVec3T(4, 2, 0), nsVec3T(2, 0, 0), nsVec3T(0, 4, 0)};

    nsBoundingSphereT s = nsBoundingSphereT::MakeFromPoints(p, 4);

    NS_TEST_BOOL(s.m_vCenter == nsVec3T(2, 3, 0));
    NS_TEST_BOOL(s.m_fRadius == 3);

    for (int i = 0; i < NS_ARRAY_SIZE(p); ++i)
    {
      NS_TEST_BOOL(s.Contains(p[i]));
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ExpandToInclude(Point)")
  {
    nsBoundingSphereT s = nsBoundingSphereT::MakeZero();

    s.ExpandToInclude(nsVec3T(3, 0, 0));

    NS_TEST_BOOL(s.m_vCenter == nsVec3T(0, 0, 0));
    NS_TEST_BOOL(s.m_fRadius == 3);

    s = nsBoundingSphereT::MakeInvalid();

    s.ExpandToInclude(nsVec3T(0.25, 0.0, 0.0));

    NS_TEST_BOOL(s.m_vCenter == nsVec3T(0, 0, 0));
    NS_TEST_BOOL(s.m_fRadius == 0.25);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ExpandToInclude(array)")
  {
    nsBoundingSphereT s = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3T(2, 2, 0), 0.0f);

    nsVec3T p[4] = {nsVec3T(0, 2, 0), nsVec3T(4, 2, 0), nsVec3T(2, 0, 0), nsVec3T(2, 4, 0)};

    s.ExpandToInclude(p, 4);

    NS_TEST_BOOL(s.m_vCenter == nsVec3T(2, 2, 0));
    NS_TEST_BOOL(s.m_fRadius == 2);

    for (int i = 0; i < NS_ARRAY_SIZE(p); ++i)
    {
      NS_TEST_BOOL(s.Contains(p[i]));
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ExpandToInclude (sphere)")
  {
    nsBoundingSphereT s1, s2, s3;
    s1 = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3T(5, 0, 0), 1);
    s2 = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3T(6, 0, 0), 1);
    s3 = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3T(5, 0, 0), 2);

    s1.ExpandToInclude(s2);
    NS_TEST_BOOL(s1.m_vCenter == nsVec3T(5, 0, 0));
    NS_TEST_BOOL(s1.m_fRadius == 2);

    s1.ExpandToInclude(s3);
    NS_TEST_BOOL(s1.m_vCenter == nsVec3T(5, 0, 0));
    NS_TEST_BOOL(s1.m_fRadius == 2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ExpandToInclude (box)")
  {
    nsBoundingSphereT s = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3T(1, 2, 3), 1);

    nsBoundingBoxT b = nsBoundingBox::MakeFromCenterAndHalfExtents(nsVec3T(1, 2, 3), nsVec3T(2.0f));

    s.ExpandToInclude(b);

    NS_TEST_BOOL(s.m_vCenter == nsVec3T(1, 2, 3));
    NS_TEST_FLOAT(s.m_fRadius, nsMath::Sqrt((nsMathTestType)12), 0.000001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Grow")
  {
    nsBoundingSphereT s = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3T(1, 2, 3), 4);

    s.Grow(5);

    NS_TEST_BOOL(s.m_vCenter == nsVec3T(1, 2, 3));
    NS_TEST_BOOL(s.m_fRadius == 9);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsIdentical, ==, !=")
  {
    nsBoundingSphereT s1, s2, s3;

    s1 = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3T(1, 2, 3), 4);
    s2 = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3T(1, 2, 3), 4);
    s3 = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3T(1.001f, 2.001f, 3.001f), 4.001f);

    NS_TEST_BOOL(s1 == s1);
    NS_TEST_BOOL(s2 == s2);
    NS_TEST_BOOL(s3 == s3);

    NS_TEST_BOOL(s1 == s2);
    NS_TEST_BOOL(s2 == s1);

    NS_TEST_BOOL(s1 != s3);
    NS_TEST_BOOL(s2 != s3);
    NS_TEST_BOOL(s3 != s1);
    NS_TEST_BOOL(s3 != s2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsEqual")
  {
    nsBoundingSphereT s1, s2, s3;

    s1 = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3T(1, 2, 3), 4);
    s2 = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3T(1, 2, 3), 4);
    s3 = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3T(1.001f, 2.001f, 3.001f), 4.001f);

    NS_TEST_BOOL(s1.IsEqual(s1));
    NS_TEST_BOOL(s2.IsEqual(s2));
    NS_TEST_BOOL(s3.IsEqual(s3));

    NS_TEST_BOOL(s1.IsEqual(s2));
    NS_TEST_BOOL(s2.IsEqual(s1));

    NS_TEST_BOOL(!s1.IsEqual(s3, 0.0001f));
    NS_TEST_BOOL(!s2.IsEqual(s3, 0.0001f));
    NS_TEST_BOOL(!s3.IsEqual(s1, 0.0001f));
    NS_TEST_BOOL(!s3.IsEqual(s2, 0.0001f));

    NS_TEST_BOOL(s1.IsEqual(s3, 0.002f));
    NS_TEST_BOOL(s2.IsEqual(s3, 0.002f));
    NS_TEST_BOOL(s3.IsEqual(s1, 0.002f));
    NS_TEST_BOOL(s3.IsEqual(s2, 0.002f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Translate")
  {
    nsBoundingSphereT s = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3T(1, 2, 3), 4);

    s.Translate(nsVec3T(4, 5, 6));

    NS_TEST_BOOL(s.m_vCenter == nsVec3T(5, 7, 9));
    NS_TEST_BOOL(s.m_fRadius == 4);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ScaleFromCenter")
  {
    nsBoundingSphereT s = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3T(1, 2, 3), 4);

    s.ScaleFromCenter(5.0f);

    NS_TEST_BOOL(s.m_vCenter == nsVec3T(1, 2, 3));
    NS_TEST_BOOL(s.m_fRadius == 20);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ScaleFromOrigin")
  {
    nsBoundingSphereT s = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3T(1, 2, 3), 4);

    s.ScaleFromOrigin(nsVec3T(2, 3, 4));

    NS_TEST_BOOL(s.m_vCenter == nsVec3T(2, 6, 12));
    NS_TEST_BOOL(s.m_fRadius == 16);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetDistanceTo (point)")
  {
    nsBoundingSphereT s = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3T(5, 0, 0), 2);

    NS_TEST_BOOL(s.GetDistanceTo(nsVec3T(5, 0, 0)) == -2.0f);
    NS_TEST_BOOL(s.GetDistanceTo(nsVec3T(7, 0, 0)) == 0.0f);
    NS_TEST_BOOL(s.GetDistanceTo(nsVec3T(9, 0, 0)) == 2.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetDistanceTo (sphere)")
  {
    nsBoundingSphereT s1, s2, s3;
    s1 = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3T(5, 0, 0), 2);
    s2 = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3T(10, 0, 0), 3);
    s3 = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3T(10, 0, 0), 1);

    NS_TEST_BOOL(s1.GetDistanceTo(s2) == 0.0f);
    NS_TEST_BOOL(s1.GetDistanceTo(s3) == 2.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetDistanceTo (array)")
  {
    nsBoundingSphereT s = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3T(0.0f), 0.0f);

    nsVec3T p[4] = {
      nsVec3T(5),
      nsVec3T(10),
      nsVec3T(15),
      nsVec3T(7),
    };

    NS_TEST_FLOAT(s.GetDistanceTo(p, 4), nsVec3T(5).GetLength(), 0.001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Contains (point)")
  {
    nsBoundingSphereT s = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3T(5, 0, 0), 2.0f);

    NS_TEST_BOOL(s.Contains(nsVec3T(3, 0, 0)));
    NS_TEST_BOOL(s.Contains(nsVec3T(5, 0, 0)));
    NS_TEST_BOOL(s.Contains(nsVec3T(6, 0, 0)));
    NS_TEST_BOOL(s.Contains(nsVec3T(7, 0, 0)));

    NS_TEST_BOOL(!s.Contains(nsVec3T(2, 0, 0)));
    NS_TEST_BOOL(!s.Contains(nsVec3T(8, 0, 0)));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Contains (array)")
  {
    nsBoundingSphereT s = nsBoundingSphereT::MakeFromCenterAndRadius(nsVec3T(0.0f), 6.0f);

    nsVec3T p[4] = {
      nsVec3T(3),
      nsVec3T(10),
      nsVec3T(2),
      nsVec3T(7),
    };

    NS_TEST_BOOL(s.Contains(p, 2, sizeof(nsVec3T) * 2));
    NS_TEST_BOOL(!s.Contains(p + 1, 2, sizeof(nsVec3T) * 2));
    NS_TEST_BOOL(!s.Contains(p, 4, sizeof(nsVec3T)));
  }

  // Disabled because MSVC 2017 has code generation issues in Release builds
  NS_TEST_BLOCK(nsTestBlock::Disabled, "Contains (sphere)")
  {
    nsBoundingSphereT s1, s2, s3;
    s1 = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3T(5, 0, 0), 2);
    s2 = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3T(6, 0, 0), 1);
    s3 = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3T(6, 0, 0), 2);

    NS_TEST_BOOL(s1.Contains(s1));
    NS_TEST_BOOL(s2.Contains(s2));
    NS_TEST_BOOL(s3.Contains(s3));

    NS_TEST_BOOL(s1.Contains(s2));
    NS_TEST_BOOL(!s1.Contains(s3));

    NS_TEST_BOOL(!s2.Contains(s3));
    NS_TEST_BOOL(s3.Contains(s2));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Contains (box)")
  {
    nsBoundingSphereT s = nsBoundingSphereT::MakeFromCenterAndRadius(nsVec3T(1, 2, 3), 4);
    nsBoundingBoxT b1 = nsBoundingBoxT::MakeFromMinMax(nsVec3T(1, 2, 3) - nsVec3T(1), nsVec3T(1, 2, 3) + nsVec3T(1));
    nsBoundingBoxT b2 = nsBoundingBoxT::MakeFromMinMax(nsVec3T(1, 2, 3) - nsVec3T(1), nsVec3T(1, 2, 3) + nsVec3T(3));

    NS_TEST_BOOL(s.Contains(b1));
    NS_TEST_BOOL(!s.Contains(b2));

    nsVec3T vDir(1, 1, 1);
    vDir.SetLength(3.99f).IgnoreResult();
    nsBoundingBoxT b3 = nsBoundingBoxT::MakeFromMinMax(nsVec3T(1, 2, 3) - nsVec3T(1), nsVec3T(1, 2, 3) + vDir);

    NS_TEST_BOOL(s.Contains(b3));

    vDir.SetLength(4.01f).IgnoreResult();
    nsBoundingBoxT b4 = nsBoundingBoxT::MakeFromMinMax(nsVec3T(1, 2, 3) - nsVec3T(1), nsVec3T(1, 2, 3) + vDir);

    NS_TEST_BOOL(!s.Contains(b4));
  }


  NS_TEST_BLOCK(nsTestBlock::Enabled, "Overlaps (array)")
  {
    nsBoundingSphereT s = nsBoundingSphereT::MakeFromCenterAndRadius(nsVec3T(0.0f), 6.0f);

    nsVec3T p[4] = {
      nsVec3T(3),
      nsVec3T(10),
      nsVec3T(2),
      nsVec3T(7),
    };

    NS_TEST_BOOL(s.Overlaps(p, 2, sizeof(nsVec3T) * 2));
    NS_TEST_BOOL(!s.Overlaps(p + 1, 2, sizeof(nsVec3T) * 2));
    NS_TEST_BOOL(s.Overlaps(p, 4, sizeof(nsVec3T)));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Overlaps (sphere)")
  {
    nsBoundingSphereT s1, s2, s3;
    s1 = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3T(5, 0, 0), 2);
    s2 = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3T(6, 0, 0), 2);
    s3 = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3T(8, 0, 0), 1);

    NS_TEST_BOOL(s1.Overlaps(s1));
    NS_TEST_BOOL(s2.Overlaps(s2));
    NS_TEST_BOOL(s3.Overlaps(s3));

    NS_TEST_BOOL(s1.Overlaps(s2));
    NS_TEST_BOOL(!s1.Overlaps(s3));

    NS_TEST_BOOL(s2.Overlaps(s3));
    NS_TEST_BOOL(s3.Overlaps(s2));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Overlaps (box)")
  {
    nsBoundingSphereT s = nsBoundingSphereT::MakeFromCenterAndRadius(nsVec3T(1, 2, 3), 2);
    nsBoundingBoxT b1 = nsBoundingBoxT::MakeFromMinMax(nsVec3T(1, 2, 3), nsVec3T(1, 2, 3) + nsVec3T(2));
    nsBoundingBoxT b2 = nsBoundingBoxT::MakeFromMinMax(nsVec3T(1, 2, 3) + nsVec3T(2), nsVec3T(1, 2, 3) + nsVec3T(3));

    NS_TEST_BOOL(s.Overlaps(b1));
    NS_TEST_BOOL(!s.Overlaps(b2));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetBoundingBox")
  {
    nsBoundingSphereT s = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3T(1, 2, 3), 2.0f);

    nsBoundingBoxT b = s.GetBoundingBox();

    NS_TEST_BOOL(b.m_vMin == nsVec3T(-1, 0, 1));
    NS_TEST_BOOL(b.m_vMax == nsVec3T(3, 4, 5));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetClampedPoint")
  {
    nsBoundingSphereT s = nsBoundingSphereT::MakeFromCenterAndRadius(nsVec3T(1, 2, 3), 2.0f);

    NS_TEST_VEC3(s.GetClampedPoint(nsVec3T(2, 2, 3)), nsVec3T(2, 2, 3), 0.001);
    NS_TEST_VEC3(s.GetClampedPoint(nsVec3T(5, 2, 3)), nsVec3T(3, 2, 3), 0.001);
    NS_TEST_VEC3(s.GetClampedPoint(nsVec3T(1, 7, 3)), nsVec3T(1, 4, 3), 0.001);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetRayIntersection")
  {
    nsBoundingSphereT s = nsBoundingSphereT::MakeFromCenterAndRadius(nsVec3T(1, 2, 3), 4);

    for (nsUInt32 i = 0; i < 10000; ++i)
    {
      const nsVec3T vDir =
        nsVec3T(nsMath::Sin(nsAngle::MakeFromDegree(i * 1.0f)), nsMath::Cos(nsAngle::MakeFromDegree(i * 3.0f)), nsMath::Cos(nsAngle::MakeFromDegree(i * 1.0f)))
          .GetNormalized();
      const nsVec3T vTarget = vDir * s.m_fRadius + s.m_vCenter;
      const nsVec3T vSource = vTarget + vDir * (nsMathTestType)5;

      NS_TEST_FLOAT((vSource - vTarget).GetLength(), 5.0f, 0.001f);

      nsMathTestType fIntersection;
      nsVec3T vIntersection;
      NS_TEST_BOOL(s.GetRayIntersection(vSource, -vDir, &fIntersection, &vIntersection) == true);
      NS_TEST_FLOAT(fIntersection, (vSource - vTarget).GetLength(), 0.0001f);
      NS_TEST_BOOL(vIntersection.IsEqual(vTarget, 0.0001f));

      NS_TEST_BOOL(s.GetRayIntersection(vSource, vDir, &fIntersection, &vIntersection) == false);

      NS_TEST_BOOL(s.GetRayIntersection(vTarget - vDir, vDir, &fIntersection, &vIntersection) == true);
      NS_TEST_FLOAT(fIntersection, 1, 0.0001f);
      NS_TEST_BOOL(vIntersection.IsEqual(vTarget, 0.0001f));
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetLineSegmentIntersection")
  {
    nsBoundingSphereT s = nsBoundingSphereT::MakeFromCenterAndRadius(nsVec3T(1, 2, 3), 4);

    for (nsUInt32 i = 0; i < 10000; ++i)
    {
      const nsVec3T vDir = nsVec3T(nsMath::Sin(nsAngle::MakeFromDegree(i * (nsMathTestType)1)), nsMath::Cos(nsAngle::MakeFromDegree(i * (nsMathTestType)3)),
        nsMath::Cos(nsAngle::MakeFromDegree(i * (nsMathTestType)1)))
                             .GetNormalized();
      const nsVec3T vTarget = vDir * s.m_fRadius + s.m_vCenter - vDir;
      const nsVec3T vSource = vTarget + vDir * (nsMathTestType)5;

      nsMathTestType fIntersection;
      nsVec3T vIntersection;
      NS_TEST_BOOL(s.GetLineSegmentIntersection(vSource, vTarget, &fIntersection, &vIntersection) == true);
      NS_TEST_FLOAT(fIntersection, 4.0f / 5.0f, 0.0001f);
      NS_TEST_BOOL(vIntersection.IsEqual(vTarget + vDir, 0.0001f));

      NS_TEST_BOOL(s.GetLineSegmentIntersection(vTarget, vSource, &fIntersection, &vIntersection) == true);
      NS_TEST_FLOAT(fIntersection, 1.0f / 5.0f, 0.0001f);
      NS_TEST_BOOL(vIntersection.IsEqual(vTarget + vDir, 0.0001f));
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "TransformFromOrigin")
  {
    nsBoundingSphereT s = nsBoundingSphereT::MakeFromCenterAndRadius(nsVec3T(1, 2, 3), 4);
    nsMat4T mTransform;

    mTransform = nsMat4::MakeTranslation(nsVec3T(5, 6, 7));
    mTransform.SetScalingFactors(nsVec3T(4, 3, 2)).IgnoreResult();

    s.TransformFromOrigin(mTransform);

    NS_TEST_BOOL(s.m_vCenter == nsVec3T(9, 12, 13));
    NS_TEST_BOOL(s.m_fRadius == 16);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "TransformFromCenter")
  {
    nsBoundingSphereT s = nsBoundingSphereT::MakeFromCenterAndRadius(nsVec3T(1, 2, 3), 4);
    nsMat4T mTransform;

    mTransform = nsMat4::MakeTranslation(nsVec3T(5, 6, 7));
    mTransform.SetScalingFactors(nsVec3T(4, 3, 2)).IgnoreResult();

    s.TransformFromCenter(mTransform);

    NS_TEST_BOOL(s.m_vCenter == nsVec3T(6, 8, 10));
    NS_TEST_BOOL(s.m_fRadius == 16);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsNaN")
  {
    if (nsMath::SupportsNaN<nsMathTestType>())
    {
      nsBoundingSphereT s;

      s = nsBoundingSphereT::MakeInvalid();
      NS_TEST_BOOL(!s.IsNaN());

      s = nsBoundingSphereT::MakeInvalid();
      s.m_fRadius = nsMath::NaN<nsMathTestType>();
      NS_TEST_BOOL(s.IsNaN());

      s = nsBoundingSphereT::MakeInvalid();
      s.m_vCenter.x = nsMath::NaN<nsMathTestType>();
      NS_TEST_BOOL(s.IsNaN());

      s = nsBoundingSphereT::MakeInvalid();
      s.m_vCenter.y = nsMath::NaN<nsMathTestType>();
      NS_TEST_BOOL(s.IsNaN());

      s = nsBoundingSphereT::MakeInvalid();
      s.m_vCenter.z = nsMath::NaN<nsMathTestType>();
      NS_TEST_BOOL(s.IsNaN());
    }
  }
}
