#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/SimdMath/SimdBSphere.h>

NS_CREATE_SIMPLE_TEST(SimdMath, SimdBSphere)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeFromCenterAndRadius")
  {
    nsSimdBSphere s = nsSimdBSphere::MakeFromCenterAndRadius(nsSimdVec4f(1, 2, 3), 4);

    NS_TEST_BOOL((s.m_CenterAndRadius == nsSimdVec4f(1, 2, 3, 4)).AllSet());

    NS_TEST_BOOL((s.GetCenter() == nsSimdVec4f(1, 2, 3)).AllSet<3>());
    NS_TEST_BOOL(s.GetRadius() == 4.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeInvalid / IsValid")
  {
    nsSimdBSphere s(nsSimdVec4f(1, 2, 3), 4);

    NS_TEST_BOOL(s.IsValid());

    s = nsSimdBSphere::MakeInvalid();

    NS_TEST_BOOL(!s.IsValid());
    NS_TEST_BOOL(!s.IsNaN());

    s = nsSimdBSphere(nsSimdVec4f(1, 2, 3), nsMath::NaN<float>());
    NS_TEST_BOOL(s.IsNaN());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ExpandToInclude(Point)")
  {
    nsSimdBSphere s(nsSimdVec4f::MakeZero(), 0.0f);

    s.ExpandToInclude(nsSimdVec4f(3, 0, 0));

    NS_TEST_BOOL((s.m_CenterAndRadius == nsSimdVec4f(0, 0, 0, 3)).AllSet());

    s = nsSimdBSphere::MakeInvalid();

    s.ExpandToInclude(nsSimdVec4f(0.25, 0, 0));

    NS_TEST_BOOL((s.m_CenterAndRadius == nsSimdVec4f(0, 0, 0, 0.25)).AllSet());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ExpandToInclude(array)")
  {
    nsSimdBSphere s(nsSimdVec4f(2, 2, 0), 0.0f);

    nsSimdVec4f p[4] = {nsSimdVec4f(0, 2, 0), nsSimdVec4f(4, 2, 0), nsSimdVec4f(2, 0, 0), nsSimdVec4f(2, 4, 0)};

    s.ExpandToInclude(p, 4);

    NS_TEST_BOOL((s.m_CenterAndRadius == nsSimdVec4f(2, 2, 0, 2)).AllSet());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ExpandToInclude (sphere)")
  {
    nsSimdBSphere s1(nsSimdVec4f(5, 0, 0), 1);
    nsSimdBSphere s2(nsSimdVec4f(6, 0, 0), 1);
    nsSimdBSphere s3(nsSimdVec4f(5, 0, 0), 2);

    s1.ExpandToInclude(s2);
    NS_TEST_BOOL((s1.m_CenterAndRadius == nsSimdVec4f(5, 0, 0, 2)).AllSet());

    s1.ExpandToInclude(s3);
    NS_TEST_BOOL((s1.m_CenterAndRadius == nsSimdVec4f(5, 0, 0, 2)).AllSet());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Transform")
  {
    nsSimdBSphere s(nsSimdVec4f(5, 0, 0), 2);

    nsSimdTransform t(nsSimdVec4f(4, 5, 6));
    t.m_Rotation = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 0, 1), nsAngle::MakeFromDegree(90));
    t.m_Scale = nsSimdVec4f(1, -2, -4);

    s.Transform(t);
    NS_TEST_BOOL(s.m_CenterAndRadius.IsEqual(nsSimdVec4f(4, 10, 6, 8), nsSimdFloat(nsMath::SmallEpsilon<float>())).AllSet());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetDistanceTo (point)")
  {
    nsSimdBSphere s(nsSimdVec4f(5, 0, 0), 2);

    NS_TEST_BOOL(s.GetDistanceTo(nsSimdVec4f(5, 0, 0)) == -2.0f);
    NS_TEST_BOOL(s.GetDistanceTo(nsSimdVec4f(7, 0, 0)) == 0.0f);
    NS_TEST_BOOL(s.GetDistanceTo(nsSimdVec4f(9, 0, 0)) == 2.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetDistanceTo (sphere)")
  {
    nsSimdBSphere s1(nsSimdVec4f(5, 0, 0), 2);
    nsSimdBSphere s2(nsSimdVec4f(10, 0, 0), 3);
    nsSimdBSphere s3(nsSimdVec4f(10, 0, 0), 1);

    NS_TEST_BOOL(s1.GetDistanceTo(s2) == 0.0f);
    NS_TEST_BOOL(s1.GetDistanceTo(s3) == 2.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Contains (point)")
  {
    nsSimdBSphere s(nsSimdVec4f(5, 0, 0), 2.0f);

    NS_TEST_BOOL(s.Contains(nsSimdVec4f(3, 0, 0)));
    NS_TEST_BOOL(s.Contains(nsSimdVec4f(5, 0, 0)));
    NS_TEST_BOOL(s.Contains(nsSimdVec4f(6, 0, 0)));
    NS_TEST_BOOL(s.Contains(nsSimdVec4f(7, 0, 0)));

    NS_TEST_BOOL(!s.Contains(nsSimdVec4f(2, 0, 0)));
    NS_TEST_BOOL(!s.Contains(nsSimdVec4f(8, 0, 0)));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Contains (sphere)")
  {
    nsSimdBSphere s1(nsSimdVec4f(5, 0, 0), 2);
    nsSimdBSphere s2(nsSimdVec4f(6, 0, 0), 1);
    nsSimdBSphere s3(nsSimdVec4f(6, 0, 0), 2);

    NS_TEST_BOOL(s1.Contains(s1));
    NS_TEST_BOOL(s2.Contains(s2));
    NS_TEST_BOOL(s3.Contains(s3));

    NS_TEST_BOOL(s1.Contains(s2));
    NS_TEST_BOOL(!s1.Contains(s3));

    NS_TEST_BOOL(!s2.Contains(s3));
    NS_TEST_BOOL(s3.Contains(s2));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Overlaps (sphere)")
  {
    nsSimdBSphere s1(nsSimdVec4f(5, 0, 0), 2);
    nsSimdBSphere s2(nsSimdVec4f(6, 0, 0), 2);
    nsSimdBSphere s3(nsSimdVec4f(8, 0, 0), 1);

    NS_TEST_BOOL(s1.Overlaps(s1));
    NS_TEST_BOOL(s2.Overlaps(s2));
    NS_TEST_BOOL(s3.Overlaps(s3));

    NS_TEST_BOOL(s1.Overlaps(s2));
    NS_TEST_BOOL(!s1.Overlaps(s3));

    NS_TEST_BOOL(s2.Overlaps(s3));
    NS_TEST_BOOL(s3.Overlaps(s2));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetClampedPoint")
  {
    nsSimdBSphere s(nsSimdVec4f(1, 2, 3), 2.0f);

    NS_TEST_BOOL(s.GetClampedPoint(nsSimdVec4f(2, 2, 3)).IsEqual(nsSimdVec4f(2, 2, 3), 0.001f).AllSet<3>());
    NS_TEST_BOOL(s.GetClampedPoint(nsSimdVec4f(5, 2, 3)).IsEqual(nsSimdVec4f(3, 2, 3), 0.001f).AllSet<3>());
    NS_TEST_BOOL(s.GetClampedPoint(nsSimdVec4f(1, 7, 3)).IsEqual(nsSimdVec4f(1, 4, 3), 0.001f).AllSet<3>());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Comparison")
  {
    nsSimdBSphere s1(nsSimdVec4f(5, 0, 0), 2);
    nsSimdBSphere s2(nsSimdVec4f(6, 0, 0), 1);

    NS_TEST_BOOL(s1 == nsSimdBSphere(nsSimdVec4f(5, 0, 0), 2));
    NS_TEST_BOOL(s1 != s2);
  }
}
