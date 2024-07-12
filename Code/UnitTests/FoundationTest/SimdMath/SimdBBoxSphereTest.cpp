#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Math/BoundingBoxSphere.h>
#include <Foundation/Math/Random.h>
#include <Foundation/Math/Transform.h>
#include <Foundation/SimdMath/SimdBBoxSphere.h>
#include <Foundation/SimdMath/SimdConversion.h>

NS_CREATE_SIMPLE_TEST(SimdMath, SimdBBoxSphere)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Make Functions")
  {
    nsSimdBBoxSphere b = nsSimdBBoxSphere::MakeFromCenterExtents(nsSimdVec4f(-1, -2, -3), nsSimdVec4f(1, 2, 3), 2);

    NS_TEST_BOOL((b.m_CenterAndRadius == nsSimdVec4f(-1, -2, -3, 2)).AllSet<4>());
    NS_TEST_BOOL((b.m_BoxHalfExtents == nsSimdVec4f(1, 2, 3)).AllSet<3>());

    nsSimdBBox box(nsSimdVec4f(1, 1, 1), nsSimdVec4f(3, 3, 3));
    nsSimdBSphere sphere(nsSimdVec4f(2, 2, 2), 1);

    b = nsSimdBBoxSphere::MakeFromBoxAndSphere(box, sphere);

    NS_TEST_BOOL((b.m_CenterAndRadius == nsSimdVec4f(2, 2, 2, 1)).AllSet<4>());
    NS_TEST_BOOL((b.m_BoxHalfExtents == nsSimdVec4f(1, 1, 1)).AllSet<3>());
    NS_TEST_BOOL(b.GetBox() == box);
    NS_TEST_BOOL(b.GetSphere() == sphere);

    b = nsSimdBBoxSphere::MakeFromBox(box);

    NS_TEST_BOOL(b.m_CenterAndRadius.IsEqual(nsSimdVec4f(2, 2, 2, nsMath::Sqrt(3.0f)), 0.00001f).AllSet<4>());
    NS_TEST_BOOL((b.m_BoxHalfExtents == nsSimdVec4f(1, 1, 1)).AllSet<3>());
    NS_TEST_BOOL(b.GetBox() == box);

    b = nsSimdBBoxSphere::MakeFromSphere(sphere);

    NS_TEST_BOOL((b.m_CenterAndRadius == nsSimdVec4f(2, 2, 2, 1)).AllSet<4>());
    NS_TEST_BOOL((b.m_BoxHalfExtents == nsSimdVec4f(1, 1, 1)).AllSet<3>());
    NS_TEST_BOOL(b.GetSphere() == sphere);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeInvalid")
  {
    nsSimdBBoxSphere b = nsSimdBBoxSphere::MakeInvalid();

    NS_TEST_BOOL(!b.IsValid());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsNaN")
  {
    if (nsMath::SupportsNaN<float>())
    {
      nsSimdBBoxSphere b = nsSimdBBoxSphere::MakeInvalid();

      b = nsSimdBBoxSphere::MakeInvalid();
      NS_TEST_BOOL(!b.IsNaN());

      b = nsSimdBBoxSphere::MakeInvalid();
      b.m_CenterAndRadius.SetX(nsMath::NaN<float>());
      NS_TEST_BOOL(b.IsNaN());

      b = nsSimdBBoxSphere::MakeInvalid();
      b.m_CenterAndRadius.SetY(nsMath::NaN<float>());
      NS_TEST_BOOL(b.IsNaN());

      b = nsSimdBBoxSphere::MakeInvalid();
      b.m_CenterAndRadius.SetZ(nsMath::NaN<float>());
      NS_TEST_BOOL(b.IsNaN());

      b = nsSimdBBoxSphere::MakeInvalid();
      b.m_CenterAndRadius.SetW(nsMath::NaN<float>());
      NS_TEST_BOOL(b.IsNaN());

      b = nsSimdBBoxSphere::MakeInvalid();
      b.m_BoxHalfExtents.SetX(nsMath::NaN<float>());
      NS_TEST_BOOL(b.IsNaN());

      b = nsSimdBBoxSphere::MakeInvalid();
      b.m_BoxHalfExtents.SetY(nsMath::NaN<float>());
      NS_TEST_BOOL(b.IsNaN());

      b = nsSimdBBoxSphere::MakeInvalid();
      b.m_BoxHalfExtents.SetZ(nsMath::NaN<float>());
      NS_TEST_BOOL(b.IsNaN());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetFromPoints")
  {
    nsSimdVec4f p[6] = {
      nsSimdVec4f(-4, 0, 0),
      nsSimdVec4f(5, 0, 0),
      nsSimdVec4f(0, -6, 0),
      nsSimdVec4f(0, 7, 0),
      nsSimdVec4f(0, 0, -8),
      nsSimdVec4f(0, 0, 9),
    };

    const nsSimdBBoxSphere b = nsSimdBBoxSphere::MakeFromPoints(p, 6);

    NS_TEST_BOOL((b.m_CenterAndRadius == nsSimdVec4f(0.5, 0.5, 0.5)).AllSet<3>());
    NS_TEST_BOOL((b.m_BoxHalfExtents == nsSimdVec4f(4.5, 6.5, 8.5)).AllSet<3>());
    NS_TEST_BOOL(b.m_CenterAndRadius.w().IsEqual(nsSimdVec4f(0.5, 0.5, 8.5).GetLength<3>(), 0.00001f));
    NS_TEST_BOOL(b.m_CenterAndRadius.w() <= b.m_BoxHalfExtents.GetLength<3>());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ExpandToInclude")
  {
    nsSimdBBoxSphere b1 = nsSimdBBoxSphere::MakeInvalid();
    nsSimdBBoxSphere b2(nsSimdBBox(nsSimdVec4f(2, 2, 2), nsSimdVec4f(4, 4, 4)));

    b1.ExpandToInclude(b2);
    NS_TEST_BOOL(b1 == b2);

    nsSimdBSphere sphere(nsSimdVec4f(2, 2, 2), 2);
    b2 = nsSimdBBoxSphere(sphere);

    b1.ExpandToInclude(b2);
    NS_TEST_BOOL(b1 != b2);

    NS_TEST_BOOL((b1.m_CenterAndRadius == nsSimdVec4f(2, 2, 2)).AllSet<3>());
    NS_TEST_BOOL((b1.m_BoxHalfExtents == nsSimdVec4f(2, 2, 2)).AllSet<3>());
    NS_TEST_FLOAT(b1.m_CenterAndRadius.w(), nsMath::Sqrt(3.0f) * 2.0f, 0.00001f);
    NS_TEST_BOOL(b1.m_CenterAndRadius.w() <= b1.m_BoxHalfExtents.GetLength<3>());

    b1 = nsSimdBBoxSphere::MakeInvalid();
    b2 = nsSimdBBox(nsSimdVec4f(0.25, 0.25, 0.25), nsSimdVec4f(0.5, 0.5, 0.5));

    b1.ExpandToInclude(b2);
    NS_TEST_BOOL(b1 == b2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Transform")
  {
    nsSimdBBoxSphere b = nsSimdBBoxSphere::MakeFromCenterExtents(nsSimdVec4f(1), nsSimdVec4f(5), 5);

    nsSimdTransform t(nsSimdVec4f(1, 1, 1), nsSimdQuat::MakeIdentity(), nsSimdVec4f(2, 3, -2));

    b.Transform(t);

    NS_TEST_BOOL((b.m_CenterAndRadius == nsSimdVec4f(3, 4, -1, 15)).AllSet<4>());
    NS_TEST_BOOL((b.m_BoxHalfExtents == nsSimdVec4f(10, 15, 10)).AllSet<3>());

    // verification
    nsRandom rnd;
    rnd.Initialize(0x736454);

    nsDynamicArray<nsSimdVec4f, nsAlignedAllocatorWrapper> points;
    points.SetCountUninitialized(10);
    float fSize = 10;

    for (nsUInt32 i = 0; i < points.GetCount(); ++i)
    {
      float x = (float)rnd.DoubleMinMax(-fSize, fSize);
      float y = (float)rnd.DoubleMinMax(-fSize, fSize);
      float z = (float)rnd.DoubleMinMax(-fSize, fSize);
      points[i] = nsSimdVec4f(x, y, z);
    }

    b = nsSimdBBoxSphere::MakeFromPoints(points.GetData(), points.GetCount());

    t.m_Rotation = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 0, 1), nsAngle::MakeFromDegree(-30));
    b.Transform(t);

    for (nsUInt32 i = 0; i < points.GetCount(); ++i)
    {
      nsSimdVec4f tp = t.TransformPosition(points[i]);

      nsSimdFloat boxDist = b.GetBox().GetDistanceTo(tp);
      NS_TEST_BOOL(boxDist < nsMath::DefaultEpsilon<float>());

      nsSimdFloat sphereDist = b.GetSphere().GetDistanceTo(tp);
      NS_TEST_BOOL(sphereDist < nsMath::DefaultEpsilon<float>());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Comparison")
  {
    nsSimdBBoxSphere b1(nsSimdBBox(nsSimdVec4f(5, 0, 0), nsSimdVec4f(1, 2, 3)));
    nsSimdBBoxSphere b2(nsSimdBBox(nsSimdVec4f(6, 0, 0), nsSimdVec4f(1, 2, 3)));

    NS_TEST_BOOL(b1 == nsSimdBBoxSphere(nsSimdBBox(nsSimdVec4f(5, 0, 0), nsSimdVec4f(1, 2, 3))));
    NS_TEST_BOOL(b1 != b2);
  }
}
