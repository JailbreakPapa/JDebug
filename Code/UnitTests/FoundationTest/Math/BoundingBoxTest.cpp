#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Math/BoundingBox.h>

NS_CREATE_SIMPLE_TEST(Math, BoundingBox)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeFromMinMax")
  {
    nsBoundingBoxT b = nsBoundingBoxT::MakeFromMinMax(nsVec3T(-1, -2, -3), nsVec3T(1, 2, 3));

    NS_TEST_BOOL(b.m_vMin == nsVec3T(-1, -2, -3));
    NS_TEST_BOOL(b.m_vMax == nsVec3T(1, 2, 3));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeFromMinMax")
  {
    nsBoundingBoxT b = nsBoundingBox::MakeFromMinMax(nsVec3T(-1, -2, -3), nsVec3T(1, 2, 3));

    NS_TEST_BOOL(b.m_vMin == nsVec3T(-1, -2, -3));
    NS_TEST_BOOL(b.m_vMax == nsVec3T(1, 2, 3));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeFromPoints")
  {
    nsVec3T p[6] = {
      nsVec3T(-4, 0, 0),
      nsVec3T(5, 0, 0),
      nsVec3T(0, -6, 0),
      nsVec3T(0, 7, 0),
      nsVec3T(0, 0, -8),
      nsVec3T(0, 0, 9),
    };

    nsBoundingBoxT b = nsBoundingBox::MakeFromPoints(p, 6);

    NS_TEST_BOOL(b.m_vMin == nsVec3T(-4, -6, -8));
    NS_TEST_BOOL(b.m_vMax == nsVec3T(5, 7, 9));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeInvalid")
  {
    nsBoundingBoxT b;
    b = nsBoundingBox::MakeInvalid();

    NS_TEST_BOOL(!b.IsValid());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeFromCenterAndHalfExtents")
  {
    nsBoundingBoxT b = nsBoundingBox::MakeFromCenterAndHalfExtents(nsVec3T(1, 2, 3), nsVec3T(4, 5, 6));

    NS_TEST_BOOL(b.m_vMin == nsVec3T(-3, -3, -3));
    NS_TEST_BOOL(b.m_vMax == nsVec3T(5, 7, 9));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetCorners")
  {
    nsBoundingBoxT b = nsBoundingBox::MakeFromMinMax(nsVec3T(-1, -2, -3), nsVec3T(1, 2, 3));

    nsVec3T c[8];
    b.GetCorners(c);

    NS_TEST_BOOL(c[0] == nsVec3T(-1, -2, -3));
    NS_TEST_BOOL(c[1] == nsVec3T(-1, -2, 3));
    NS_TEST_BOOL(c[2] == nsVec3T(-1, 2, -3));
    NS_TEST_BOOL(c[3] == nsVec3T(-1, 2, 3));
    NS_TEST_BOOL(c[4] == nsVec3T(1, -2, -3));
    NS_TEST_BOOL(c[5] == nsVec3T(1, -2, 3));
    NS_TEST_BOOL(c[6] == nsVec3T(1, 2, -3));
    NS_TEST_BOOL(c[7] == nsVec3T(1, 2, 3));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ExpandToInclue (Point)")
  {
    nsBoundingBoxT b;
    b = nsBoundingBox::MakeInvalid();
    b.ExpandToInclude(nsVec3T(1, 2, 3));

    NS_TEST_BOOL(b.m_vMin == nsVec3T(1, 2, 3));
    NS_TEST_BOOL(b.m_vMax == nsVec3T(1, 2, 3));


    b.ExpandToInclude(nsVec3T(2, 3, 4));

    NS_TEST_BOOL(b.m_vMin == nsVec3T(1, 2, 3));
    NS_TEST_BOOL(b.m_vMax == nsVec3T(2, 3, 4));

    b.ExpandToInclude(nsVec3T(0, 1, 2));

    NS_TEST_BOOL(b.m_vMin == nsVec3T(0, 1, 2));
    NS_TEST_BOOL(b.m_vMax == nsVec3T(2, 3, 4));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ExpandToInclude (Box)")
  {
    nsBoundingBoxT b1, b2;

    b1 = nsBoundingBox::MakeFromMinMax(nsVec3T(-1, -2, -3), nsVec3T(1, 2, 3));
    b2 = nsBoundingBox::MakeFromMinMax(nsVec3T(0), nsVec3T(4, 5, 6));

    b1.ExpandToInclude(b2);

    NS_TEST_BOOL(b1.m_vMin == nsVec3T(-1, -2, -3));
    NS_TEST_BOOL(b1.m_vMax == nsVec3T(4, 5, 6));

    b2 = nsBoundingBox::MakeFromMinMax(nsVec3T(-4, -5, -6), nsVec3T(0));

    b1.ExpandToInclude(b2);

    NS_TEST_BOOL(b1.m_vMin == nsVec3T(-4, -5, -6));
    NS_TEST_BOOL(b1.m_vMax == nsVec3T(4, 5, 6));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ExpandToInclude (array)")
  {
    nsVec3T v[4] = {nsVec3T(1, 1, 1), nsVec3T(-1, -1, -1), nsVec3T(2, 2, 2), nsVec3T(4, 4, 4)};

    nsBoundingBoxT b;
    b = nsBoundingBox::MakeInvalid();
    b.ExpandToInclude(v, 2, sizeof(nsVec3T) * 2);

    NS_TEST_BOOL(b.m_vMin == nsVec3T(1, 1, 1));
    NS_TEST_BOOL(b.m_vMax == nsVec3T(2, 2, 2));

    b.ExpandToInclude(v, 4, sizeof(nsVec3T));

    NS_TEST_BOOL(b.m_vMin == nsVec3T(-1, -1, -1));
    NS_TEST_BOOL(b.m_vMax == nsVec3T(4, 4, 4));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ExpandToCube")
  {
    nsBoundingBoxT b = nsBoundingBox::MakeFromCenterAndHalfExtents(nsVec3T(1, 2, 3), nsVec3T(4, 5, 6));

    b.ExpandToCube();

    NS_TEST_VEC3(b.GetCenter(), nsVec3T(1, 2, 3), nsMath::DefaultEpsilon<nsMathTestType>());
    NS_TEST_VEC3(b.GetHalfExtents(), nsVec3T(6, 6, 6), nsMath::DefaultEpsilon<nsMathTestType>());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Grow")
  {
    nsBoundingBoxT b = nsBoundingBoxT::MakeFromMinMax(nsVec3T(1, 2, 3), nsVec3T(4, 5, 6));
    b.Grow(nsVec3T(2, 4, 6));

    NS_TEST_BOOL(b.m_vMin == nsVec3T(-1, -2, -3));
    NS_TEST_BOOL(b.m_vMax == nsVec3T(6, 9, 12));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Contains (Point)")
  {
    nsBoundingBoxT b = nsBoundingBoxT::MakeFromMinMax(nsVec3T(0), nsVec3T(0));

    NS_TEST_BOOL(b.Contains(nsVec3T(0)));
    NS_TEST_BOOL(!b.Contains(nsVec3T(1, 0, 0)));
    NS_TEST_BOOL(!b.Contains(nsVec3T(-1, 0, 0)));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Contains (Box)")
  {
    nsBoundingBoxT b1 = nsBoundingBoxT::MakeFromMinMax(nsVec3T(-3), nsVec3T(3));
    nsBoundingBoxT b2 = nsBoundingBoxT::MakeFromMinMax(nsVec3T(-1), nsVec3T(1));
    nsBoundingBoxT b3 = nsBoundingBoxT::MakeFromMinMax(nsVec3T(-1), nsVec3T(4));

    NS_TEST_BOOL(b1.Contains(b1));
    NS_TEST_BOOL(b2.Contains(b2));
    NS_TEST_BOOL(b3.Contains(b3));

    NS_TEST_BOOL(b1.Contains(b2));
    NS_TEST_BOOL(!b1.Contains(b3));

    NS_TEST_BOOL(!b2.Contains(b1));
    NS_TEST_BOOL(!b2.Contains(b3));

    NS_TEST_BOOL(!b3.Contains(b1));
    NS_TEST_BOOL(b3.Contains(b2));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Contains (Array)")
  {
    nsBoundingBoxT b = nsBoundingBoxT::MakeFromMinMax(nsVec3T(1), nsVec3T(5));

    nsVec3T v[4] = {nsVec3T(0), nsVec3T(1), nsVec3T(5), nsVec3T(6)};

    NS_TEST_BOOL(!b.Contains(&v[0], 4, sizeof(nsVec3T)));
    NS_TEST_BOOL(b.Contains(&v[1], 2, sizeof(nsVec3T)));
    NS_TEST_BOOL(b.Contains(&v[2], 1, sizeof(nsVec3T)));

    NS_TEST_BOOL(!b.Contains(&v[1], 2, sizeof(nsVec3T) * 2));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Contains (Sphere)")
  {
    nsBoundingBoxT b = nsBoundingBoxT::MakeFromMinMax(nsVec3T(1), nsVec3T(5));

    NS_TEST_BOOL(b.Contains(nsBoundingSphereT::MakeFromCenterAndRadius(nsVec3T(3), 2)));
    NS_TEST_BOOL(!b.Contains(nsBoundingSphereT::MakeFromCenterAndRadius(nsVec3T(3), 2.1f)));
    NS_TEST_BOOL(!b.Contains(nsBoundingSphereT::MakeFromCenterAndRadius(nsVec3T(8), 2)));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Overlaps (box)")
  {
    nsBoundingBoxT b1 = nsBoundingBoxT::MakeFromMinMax(nsVec3T(-3), nsVec3T(3));
    nsBoundingBoxT b2 = nsBoundingBoxT::MakeFromMinMax(nsVec3T(-1), nsVec3T(1));
    nsBoundingBoxT b3 = nsBoundingBoxT::MakeFromMinMax(nsVec3T(1), nsVec3T(4));
    nsBoundingBoxT b4 = nsBoundingBoxT::MakeFromMinMax(nsVec3T(-4, 1, 1), nsVec3T(4, 2, 2));

    NS_TEST_BOOL(b1.Overlaps(b1));
    NS_TEST_BOOL(b2.Overlaps(b2));
    NS_TEST_BOOL(b3.Overlaps(b3));
    NS_TEST_BOOL(b4.Overlaps(b4));

    NS_TEST_BOOL(b1.Overlaps(b2));
    NS_TEST_BOOL(b1.Overlaps(b3));
    NS_TEST_BOOL(b1.Overlaps(b4));

    NS_TEST_BOOL(!b2.Overlaps(b3));
    NS_TEST_BOOL(!b2.Overlaps(b4));

    NS_TEST_BOOL(b3.Overlaps(b4));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Overlaps (Array)")
  {
    nsBoundingBoxT b = nsBoundingBoxT::MakeFromMinMax(nsVec3T(1), nsVec3T(5));

    nsVec3T v[4] = {nsVec3T(0), nsVec3T(1), nsVec3T(5), nsVec3T(6)};

    NS_TEST_BOOL(!b.Overlaps(&v[0], 1, sizeof(nsVec3T)));
    NS_TEST_BOOL(!b.Overlaps(&v[3], 1, sizeof(nsVec3T)));

    NS_TEST_BOOL(b.Overlaps(&v[0], 4, sizeof(nsVec3T)));
    NS_TEST_BOOL(b.Overlaps(&v[1], 2, sizeof(nsVec3T)));
    NS_TEST_BOOL(b.Overlaps(&v[2], 1, sizeof(nsVec3T)));

    NS_TEST_BOOL(b.Overlaps(&v[1], 2, sizeof(nsVec3T) * 2));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Overlaps (Sphere)")
  {
    nsBoundingBoxT b = nsBoundingBoxT::MakeFromMinMax(nsVec3T(1), nsVec3T(5));

    NS_TEST_BOOL(b.Overlaps(nsBoundingSphereT::MakeFromCenterAndRadius(nsVec3T(3), 2)));
    NS_TEST_BOOL(b.Overlaps(nsBoundingSphereT::MakeFromCenterAndRadius(nsVec3T(3), 2.1f)));
    NS_TEST_BOOL(!b.Overlaps(nsBoundingSphereT::MakeFromCenterAndRadius(nsVec3T(8), 2)));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsIdentical, ==, !=")
  {
    nsBoundingBoxT b1, b2, b3;

    b1 = nsBoundingBox::MakeFromMinMax(nsVec3T(1), nsVec3T(2));
    b2 = nsBoundingBox::MakeFromMinMax(nsVec3T(1), nsVec3T(2));
    b3 = nsBoundingBox::MakeFromMinMax(nsVec3T(1), nsVec3T(2.01f));

    NS_TEST_BOOL(b1.IsIdentical(b1));
    NS_TEST_BOOL(b2.IsIdentical(b2));
    NS_TEST_BOOL(b3.IsIdentical(b3));

    NS_TEST_BOOL(b1 == b1);
    NS_TEST_BOOL(b2 == b2);
    NS_TEST_BOOL(b3 == b3);

    NS_TEST_BOOL(b1.IsIdentical(b2));
    NS_TEST_BOOL(b2.IsIdentical(b1));

    NS_TEST_BOOL(!b1.IsIdentical(b3));
    NS_TEST_BOOL(!b2.IsIdentical(b3));
    NS_TEST_BOOL(!b3.IsIdentical(b1));
    NS_TEST_BOOL(!b3.IsIdentical(b1));

    NS_TEST_BOOL(b1 == b2);
    NS_TEST_BOOL(b2 == b1);

    NS_TEST_BOOL(b1 != b3);
    NS_TEST_BOOL(b2 != b3);
    NS_TEST_BOOL(b3 != b1);
    NS_TEST_BOOL(b3 != b1);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsEqual")
  {
    nsBoundingBoxT b1, b2;
    b1 = nsBoundingBox::MakeFromMinMax(nsVec3T(-1), nsVec3T(1));
    b2 = nsBoundingBox::MakeFromMinMax(nsVec3T(-1), nsVec3T(2));

    NS_TEST_BOOL(!b1.IsEqual(b2));
    NS_TEST_BOOL(!b1.IsEqual(b2, 0.5f));
    NS_TEST_BOOL(b1.IsEqual(b2, 1));
    NS_TEST_BOOL(b1.IsEqual(b2, 2));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetCenter")
  {
    nsBoundingBoxT b = nsBoundingBoxT::MakeFromMinMax(nsVec3T(3), nsVec3T(7));

    NS_TEST_BOOL(b.GetCenter() == nsVec3T(5));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetExtents")
  {
    nsBoundingBoxT b = nsBoundingBoxT::MakeFromMinMax(nsVec3T(3), nsVec3T(7));

    NS_TEST_BOOL(b.GetExtents() == nsVec3T(4));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetHalfExtents")
  {
    nsBoundingBoxT b = nsBoundingBoxT::MakeFromMinMax(nsVec3T(3), nsVec3T(7));

    NS_TEST_BOOL(b.GetHalfExtents() == nsVec3T(2));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Translate")
  {
    nsBoundingBoxT b = nsBoundingBoxT::MakeFromMinMax(nsVec3T(3), nsVec3T(5));

    b.Translate(nsVec3T(1, 2, 3));

    NS_TEST_BOOL(b.m_vMin == nsVec3T(4, 5, 6));
    NS_TEST_BOOL(b.m_vMax == nsVec3T(6, 7, 8));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ScaleFromCenter")
  {
    {
      nsBoundingBoxT b = nsBoundingBoxT::MakeFromMinMax(nsVec3T(3), nsVec3T(5));

      b.ScaleFromCenter(nsVec3T(1, 2, 3));

      NS_TEST_BOOL(b.m_vMin == nsVec3T(3, 2, 1));
      NS_TEST_BOOL(b.m_vMax == nsVec3T(5, 6, 7));
    }
    {
      nsBoundingBoxT b = nsBoundingBoxT::MakeFromMinMax(nsVec3T(3), nsVec3T(5));

      b.ScaleFromCenter(nsVec3T(-1, -2, -3));

      NS_TEST_BOOL(b.m_vMin == nsVec3T(3, 2, 1));
      NS_TEST_BOOL(b.m_vMax == nsVec3T(5, 6, 7));
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ScaleFromOrigin")
  {
    {
      nsBoundingBoxT b = nsBoundingBoxT::MakeFromMinMax(nsVec3T(3), nsVec3T(5));

      b.ScaleFromOrigin(nsVec3T(1, 2, 3));

      NS_TEST_BOOL(b.m_vMin == nsVec3T(3, 6, 9));
      NS_TEST_BOOL(b.m_vMax == nsVec3T(5, 10, 15));
    }
    {
      nsBoundingBoxT b = nsBoundingBoxT::MakeFromMinMax(nsVec3T(3), nsVec3T(5));

      b.ScaleFromOrigin(nsVec3T(-1, -2, -3));

      NS_TEST_BOOL(b.m_vMin == nsVec3T(-5, -10, -15));
      NS_TEST_BOOL(b.m_vMax == nsVec3T(-3, -6, -9));
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "TransformFromOrigin")
  {
    nsBoundingBoxT b = nsBoundingBoxT::MakeFromMinMax(nsVec3T(3), nsVec3T(5));

    nsMat4T m = nsMat4::MakeScaling(nsVec3T(2));

    b.TransformFromOrigin(m);

    NS_TEST_BOOL(b.m_vMin == nsVec3T(6, 6, 6));
    NS_TEST_BOOL(b.m_vMax == nsVec3T(10, 10, 10));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "TransformFromCenter")
  {
    nsBoundingBoxT b = nsBoundingBoxT::MakeFromMinMax(nsVec3T(3), nsVec3T(5));

    nsMat4T m = nsMat4::MakeScaling(nsVec3T(2));

    b.TransformFromCenter(m);

    NS_TEST_BOOL(b.m_vMin == nsVec3T(2, 2, 2));
    NS_TEST_BOOL(b.m_vMax == nsVec3T(6, 6, 6));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetClampedPoint")
  {
    nsBoundingBoxT b = nsBoundingBoxT::MakeFromMinMax(nsVec3T(-1, -2, -3), nsVec3T(1, 2, 3));

    NS_TEST_BOOL(b.GetClampedPoint(nsVec3T(-2, 0, 0)) == nsVec3T(-1, 0, 0));
    NS_TEST_BOOL(b.GetClampedPoint(nsVec3T(2, 0, 0)) == nsVec3T(1, 0, 0));

    NS_TEST_BOOL(b.GetClampedPoint(nsVec3T(0, -3, 0)) == nsVec3T(0, -2, 0));
    NS_TEST_BOOL(b.GetClampedPoint(nsVec3T(0, 3, 0)) == nsVec3T(0, 2, 0));

    NS_TEST_BOOL(b.GetClampedPoint(nsVec3T(0, 0, -4)) == nsVec3T(0, 0, -3));
    NS_TEST_BOOL(b.GetClampedPoint(nsVec3T(0, 0, 4)) == nsVec3T(0, 0, 3));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetDistanceTo (point)")
  {
    nsBoundingBoxT b = nsBoundingBoxT::MakeFromMinMax(nsVec3T(-1, -2, -3), nsVec3T(1, 2, 3));

    NS_TEST_BOOL(b.GetDistanceTo(nsVec3T(-2, 0, 0)) == 1);
    NS_TEST_BOOL(b.GetDistanceTo(nsVec3T(2, 0, 0)) == 1);

    NS_TEST_BOOL(b.GetDistanceTo(nsVec3T(0, -4, 0)) == 2);
    NS_TEST_BOOL(b.GetDistanceTo(nsVec3T(0, 4, 0)) == 2);

    NS_TEST_BOOL(b.GetDistanceTo(nsVec3T(0, 0, -6)) == 3);
    NS_TEST_BOOL(b.GetDistanceTo(nsVec3T(0, 0, 6)) == 3);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetDistanceTo (Sphere)")
  {
    nsBoundingBoxT b = nsBoundingBoxT::MakeFromMinMax(nsVec3T(1), nsVec3T(5));

    NS_TEST_BOOL(b.GetDistanceTo(nsBoundingSphereT::MakeFromCenterAndRadius(nsVec3T(3), 2)) < 0);
    NS_TEST_BOOL(b.GetDistanceTo(nsBoundingSphereT::MakeFromCenterAndRadius(nsVec3T(5), 1)) < 0);
    NS_TEST_FLOAT(b.GetDistanceTo(nsBoundingSphereT::MakeFromCenterAndRadius(nsVec3T(8, 2, 2), 2)), 1, 0.001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetDistanceTo (box)")
  {
    nsBoundingBoxT b = nsBoundingBoxT::MakeFromMinMax(nsVec3T(1), nsVec3T(5));

    nsBoundingBoxT b1, b2, b3;
    b1 = nsBoundingBox::MakeFromCenterAndHalfExtents(nsVec3T(3), nsVec3T(2));
    b2 = nsBoundingBox::MakeFromCenterAndHalfExtents(nsVec3T(5), nsVec3T(1));
    b3 = nsBoundingBox::MakeFromCenterAndHalfExtents(nsVec3T(9, 2, 2), nsVec3T(2));

    NS_TEST_BOOL(b.GetDistanceTo(b1) <= 0);
    NS_TEST_BOOL(b.GetDistanceTo(b2) <= 0);
    NS_TEST_FLOAT(b.GetDistanceTo(b3), 2, 0.001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetDistanceSquaredTo (point)")
  {
    nsBoundingBoxT b = nsBoundingBoxT::MakeFromMinMax(nsVec3T(-1, -2, -3), nsVec3T(1, 2, 3));

    NS_TEST_BOOL(b.GetDistanceSquaredTo(nsVec3T(-2, 0, 0)) == 1);
    NS_TEST_BOOL(b.GetDistanceSquaredTo(nsVec3T(2, 0, 0)) == 1);

    NS_TEST_BOOL(b.GetDistanceSquaredTo(nsVec3T(0, -4, 0)) == 4);
    NS_TEST_BOOL(b.GetDistanceSquaredTo(nsVec3T(0, 4, 0)) == 4);

    NS_TEST_BOOL(b.GetDistanceSquaredTo(nsVec3T(0, 0, -6)) == 9);
    NS_TEST_BOOL(b.GetDistanceSquaredTo(nsVec3T(0, 0, 6)) == 9);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetDistanceSquaredTo (box)")
  {
    nsBoundingBoxT b = nsBoundingBoxT::MakeFromMinMax(nsVec3T(1), nsVec3T(5));

    nsBoundingBoxT b1, b2, b3;
    b1 = nsBoundingBox::MakeFromCenterAndHalfExtents(nsVec3T(3), nsVec3T(2));
    b2 = nsBoundingBox::MakeFromCenterAndHalfExtents(nsVec3T(5), nsVec3T(1));
    b3 = nsBoundingBox::MakeFromCenterAndHalfExtents(nsVec3T(9, 2, 2), nsVec3T(2));

    NS_TEST_BOOL(b.GetDistanceSquaredTo(b1) <= 0);
    NS_TEST_BOOL(b.GetDistanceSquaredTo(b2) <= 0);
    NS_TEST_FLOAT(b.GetDistanceSquaredTo(b3), 4, 0.001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetBoundingSphere")
  {
    nsBoundingBoxT b;
    b = nsBoundingBox::MakeFromCenterAndHalfExtents(nsVec3T(5, 4, 2), nsVec3T(3));

    nsBoundingSphereT s = b.GetBoundingSphere();

    NS_TEST_BOOL(s.m_vCenter == nsVec3T(5, 4, 2));
    NS_TEST_FLOAT(s.m_fRadius, nsVec3T(3).GetLength(), 0.001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetRayIntersection")
  {
    if (nsMath::SupportsInfinity<nsMathTestType>())
    {
      const nsVec3T c = nsVec3T(10);

      nsBoundingBoxT b;
      b = nsBoundingBox::MakeFromCenterAndHalfExtents(c, nsVec3T(2, 4, 8));

      for (nsMathTestType x = b.m_vMin.x - (nsMathTestType)1; x < b.m_vMax.x + (nsMathTestType)1; x += (nsMathTestType)0.2f)
      {
        for (nsMathTestType y = b.m_vMin.y - (nsMathTestType)1; y < b.m_vMax.y + (nsMathTestType)1; y += (nsMathTestType)0.2f)
        {
          for (nsMathTestType z = b.m_vMin.z - (nsMathTestType)1; z < b.m_vMax.z + (nsMathTestType)1; z += (nsMathTestType)0.2f)
          {
            const nsVec3T v(x, y, z);

            if (b.Contains(v))
              continue;

            const nsVec3T vTarget = b.GetClampedPoint(v);

            const nsVec3T vDir = (vTarget - c).GetNormalized();

            const nsVec3T vSource = vTarget + vDir * (nsMathTestType)3;

            nsMathTestType f;
            nsVec3T vi;
            NS_TEST_BOOL(b.GetRayIntersection(vSource, -vDir, &f, &vi) == true);
            NS_TEST_FLOAT(f, 3, 0.001f);
            NS_TEST_BOOL(vi.IsEqual(vTarget, 0.0001f));

            NS_TEST_BOOL(b.GetRayIntersection(vSource, vDir, &f, &vi) == false);
            NS_TEST_BOOL(b.GetRayIntersection(vTarget, vDir, &f, &vi) == false);
          }
        }
      }
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetLineSegmentIntersection")
  {
    if (nsMath::SupportsInfinity<nsMathTestType>())
    {
      const nsVec3T c = nsVec3T(10);

      nsBoundingBoxT b;
      b = nsBoundingBox::MakeFromCenterAndHalfExtents(c, nsVec3T(2, 4, 8));

      for (nsMathTestType x = b.m_vMin.x - (nsMathTestType)1; x < b.m_vMax.x + (nsMathTestType)1; x += (nsMathTestType)0.2f)
      {
        for (nsMathTestType y = b.m_vMin.y - (nsMathTestType)1; y < b.m_vMax.y + (nsMathTestType)1; y += (nsMathTestType)0.2f)
        {
          for (nsMathTestType z = b.m_vMin.z - (nsMathTestType)1; z < b.m_vMax.z + (nsMathTestType)1; z += (nsMathTestType)0.2f)
          {
            const nsVec3T v(x, y, z);

            if (b.Contains(v))
              continue;

            const nsVec3T vTarget0 = b.GetClampedPoint(v);

            const nsVec3T vDir = (vTarget0 - c).GetNormalized();

            const nsVec3T vTarget = vTarget0 - vDir * (nsMathTestType)1;
            const nsVec3T vSource = vTarget0 + vDir * (nsMathTestType)3;

            nsMathTestType f;
            nsVec3T vi;
            NS_TEST_BOOL(b.GetLineSegmentIntersection(vSource, vTarget, &f, &vi) == true);
            NS_TEST_FLOAT(f, 0.75f, 0.001f);
            NS_TEST_BOOL(vi.IsEqual(vTarget0, 0.0001f));
          }
        }
      }
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsNaN")
  {
    if (nsMath::SupportsNaN<nsMathTestType>())
    {
      nsBoundingBoxT b;

      b = nsBoundingBox::MakeInvalid();
      NS_TEST_BOOL(!b.IsNaN());

      b = nsBoundingBox::MakeInvalid();
      b.m_vMin.x = nsMath::NaN<nsMathTestType>();
      NS_TEST_BOOL(b.IsNaN());

      b = nsBoundingBox::MakeInvalid();
      b.m_vMin.y = nsMath::NaN<nsMathTestType>();
      NS_TEST_BOOL(b.IsNaN());

      b = nsBoundingBox::MakeInvalid();
      b.m_vMin.z = nsMath::NaN<nsMathTestType>();
      NS_TEST_BOOL(b.IsNaN());

      b = nsBoundingBox::MakeInvalid();
      b.m_vMax.x = nsMath::NaN<nsMathTestType>();
      NS_TEST_BOOL(b.IsNaN());

      b = nsBoundingBox::MakeInvalid();
      b.m_vMax.y = nsMath::NaN<nsMathTestType>();
      NS_TEST_BOOL(b.IsNaN());

      b = nsBoundingBox::MakeInvalid();
      b.m_vMax.z = nsMath::NaN<nsMathTestType>();
      NS_TEST_BOOL(b.IsNaN());
    }
  }
}
