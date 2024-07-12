#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Math/BoundingBox.h>
#include <Foundation/Math/Transform.h>
#include <Foundation/SimdMath/SimdBBox.h>
#include <Foundation/SimdMath/SimdConversion.h>

#define NS_TEST_SIMD_VECTOR_EQUAL(NUM_COMPONENTS, A, B, EPSILON)                                                                                               \
  do                                                                                                                                                           \
  {                                                                                                                                                            \
    auto _nsDiff = B - A;                                                                                                                                      \
    nsTestBool((A).IsEqual((B), EPSILON).AllSet<NUM_COMPONENTS>(), "Test failed: " NS_STRINGIZE(A) ".IsEqual(" NS_STRINGIZE(B) ", " NS_STRINGIZE(EPSILON) ")", \
      NS_SOURCE_FILE, NS_SOURCE_LINE, NS_SOURCE_FUNCTION,                                                                                                      \
      "Difference %lf %lf %lf %lf", _nsDiff.x(), _nsDiff.y(), _nsDiff.z(), _nsDiff.w());                                                                       \
  } while (false)


NS_CREATE_SIMPLE_TEST(SimdMath, SimdBBox)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor")
  {
    nsSimdBBox b(nsSimdVec4f(-1, -2, -3), nsSimdVec4f(1, 2, 3));

    NS_TEST_BOOL((b.m_Min == nsSimdVec4f(-1, -2, -3)).AllSet<3>());
    NS_TEST_BOOL((b.m_Max == nsSimdVec4f(1, 2, 3)).AllSet<3>());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeInvalid")
  {
    nsSimdBBox b = nsSimdBBox::MakeInvalid();

    NS_TEST_BOOL(!b.IsValid());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsNaN")
  {
    nsSimdBBox b = nsSimdBBox::MakeInvalid();

    b = nsSimdBBox::MakeInvalid();
    NS_TEST_BOOL(!b.IsNaN());

    b = nsSimdBBox::MakeInvalid();
    b.m_Min.SetX(nsMath::NaN<nsMathTestType>());
    NS_TEST_BOOL(b.IsNaN());

    b = nsSimdBBox::MakeInvalid();
    b.m_Min.SetY(nsMath::NaN<nsMathTestType>());
    NS_TEST_BOOL(b.IsNaN());

    b = nsSimdBBox::MakeInvalid();
    b.m_Min.SetZ(nsMath::NaN<nsMathTestType>());
    NS_TEST_BOOL(b.IsNaN());

    b = nsSimdBBox::MakeInvalid();
    b.m_Max.SetX(nsMath::NaN<nsMathTestType>());
    NS_TEST_BOOL(b.IsNaN());

    b = nsSimdBBox::MakeInvalid();
    b.m_Max.SetY(nsMath::NaN<nsMathTestType>());
    NS_TEST_BOOL(b.IsNaN());

    b = nsSimdBBox::MakeInvalid();
    b.m_Max.SetZ(nsMath::NaN<nsMathTestType>());
    NS_TEST_BOOL(b.IsNaN());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeFromCenterAndHalfExtents")
  {
    const nsSimdBBox b = nsSimdBBox::MakeFromCenterAndHalfExtents(nsSimdVec4f(1, 2, 3), nsSimdVec4f(4, 5, 6));

    NS_TEST_BOOL((b.m_Min == nsSimdVec4f(-3, -3, -3)).AllSet<3>());
    NS_TEST_BOOL((b.m_Max == nsSimdVec4f(5, 7, 9)).AllSet<3>());

    NS_TEST_BOOL((b.GetCenter() == nsSimdVec4f(1, 2, 3)).AllSet<3>());
    NS_TEST_BOOL((b.GetExtents() == nsSimdVec4f(8, 10, 12)).AllSet<3>());
    NS_TEST_BOOL((b.GetHalfExtents() == nsSimdVec4f(4, 5, 6)).AllSet<3>());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeFromPoints")
  {
    nsSimdVec4f p[6] = {
      nsSimdVec4f(-4, 0, 0),
      nsSimdVec4f(5, 0, 0),
      nsSimdVec4f(0, -6, 0),
      nsSimdVec4f(0, 7, 0),
      nsSimdVec4f(0, 0, -8),
      nsSimdVec4f(0, 0, 9),
    };

    const nsSimdBBox b = nsSimdBBox::MakeFromPoints(p, 6);

    NS_TEST_BOOL((b.m_Min == nsSimdVec4f(-4, -6, -8)).AllSet<3>());
    NS_TEST_BOOL((b.m_Max == nsSimdVec4f(5, 7, 9)).AllSet<3>());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ExpandToInclude (Point)")
  {
    nsSimdBBox b = nsSimdBBox::MakeInvalid();
    b.ExpandToInclude(nsSimdVec4f(1, 2, 3));

    NS_TEST_BOOL((b.m_Min == nsSimdVec4f(1, 2, 3)).AllSet<3>());
    NS_TEST_BOOL((b.m_Max == nsSimdVec4f(1, 2, 3)).AllSet<3>());


    b.ExpandToInclude(nsSimdVec4f(2, 3, 4));

    NS_TEST_BOOL((b.m_Min == nsSimdVec4f(1, 2, 3)).AllSet<3>());
    NS_TEST_BOOL((b.m_Max == nsSimdVec4f(2, 3, 4)).AllSet<3>());

    b.ExpandToInclude(nsSimdVec4f(0, 1, 2));

    NS_TEST_BOOL((b.m_Min == nsSimdVec4f(0, 1, 2)).AllSet<3>());
    NS_TEST_BOOL((b.m_Max == nsSimdVec4f(2, 3, 4)).AllSet<3>());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ExpandToInclude (array)")
  {
    nsSimdVec4f v[4] = {nsSimdVec4f(1, 1, 1), nsSimdVec4f(-1, -1, -1), nsSimdVec4f(2, 2, 2), nsSimdVec4f(4, 4, 4)};

    nsSimdBBox b = nsSimdBBox::MakeInvalid();
    b.ExpandToInclude(v, 2, sizeof(nsSimdVec4f) * 2);

    NS_TEST_BOOL((b.m_Min == nsSimdVec4f(1, 1, 1)).AllSet<3>());
    NS_TEST_BOOL((b.m_Max == nsSimdVec4f(2, 2, 2)).AllSet<3>());

    b.ExpandToInclude(v, 4);

    NS_TEST_BOOL((b.m_Min == nsSimdVec4f(-1, -1, -1)).AllSet<3>());
    NS_TEST_BOOL((b.m_Max == nsSimdVec4f(4, 4, 4)).AllSet<3>());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ExpandToInclude (Box)")
  {
    nsSimdBBox b1(nsSimdVec4f(-1, -2, -3), nsSimdVec4f(1, 2, 3));
    nsSimdBBox b2(nsSimdVec4f(0), nsSimdVec4f(4, 5, 6));

    b1.ExpandToInclude(b2);

    NS_TEST_BOOL((b1.m_Min == nsSimdVec4f(-1, -2, -3)).AllSet<3>());
    NS_TEST_BOOL((b1.m_Max == nsSimdVec4f(4, 5, 6)).AllSet<3>());

    nsSimdBBox b3 = nsSimdBBox::MakeInvalid();
    b3.ExpandToInclude(b1);
    NS_TEST_BOOL(b3 == b1);

    b2.m_Min = nsSimdVec4f(-4, -5, -6);
    b2.m_Max.SetZero();

    b1.ExpandToInclude(b2);

    NS_TEST_BOOL((b1.m_Min == nsSimdVec4f(-4, -5, -6)).AllSet<3>());
    NS_TEST_BOOL((b1.m_Max == nsSimdVec4f(4, 5, 6)).AllSet<3>());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ExpandToCube")
  {
    nsSimdBBox b = nsSimdBBox::MakeFromCenterAndHalfExtents(nsSimdVec4f(1, 2, 3), nsSimdVec4f(4, 5, 6));

    b.ExpandToCube();

    NS_TEST_BOOL((b.GetCenter() == nsSimdVec4f(1, 2, 3)).AllSet<3>());
    NS_TEST_BOOL((b.GetHalfExtents() == nsSimdVec4f(6, 6, 6)).AllSet<3>());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Contains (Point)")
  {
    nsSimdBBox b(nsSimdVec4f(0), nsSimdVec4f(0));

    NS_TEST_BOOL(b.Contains(nsSimdVec4f(0)));
    NS_TEST_BOOL(!b.Contains(nsSimdVec4f(1, 0, 0)));
    NS_TEST_BOOL(!b.Contains(nsSimdVec4f(-1, 0, 0)));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Contains (Box)")
  {
    nsSimdBBox b1(nsSimdVec4f(-3), nsSimdVec4f(3));
    nsSimdBBox b2(nsSimdVec4f(-1), nsSimdVec4f(1));
    nsSimdBBox b3(nsSimdVec4f(-1), nsSimdVec4f(4));

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

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Contains (Sphere)")
  {
    nsSimdBBox b(nsSimdVec4f(1), nsSimdVec4f(5));

    NS_TEST_BOOL(b.Contains(nsSimdBSphere(nsSimdVec4f(3), 2)));
    NS_TEST_BOOL(!b.Contains(nsSimdBSphere(nsSimdVec4f(3), 2.1f)));
    NS_TEST_BOOL(!b.Contains(nsSimdBSphere(nsSimdVec4f(8), 2)));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Overlaps (box)")
  {
    nsSimdBBox b1(nsSimdVec4f(-3), nsSimdVec4f(3));
    nsSimdBBox b2(nsSimdVec4f(-1), nsSimdVec4f(1));
    nsSimdBBox b3(nsSimdVec4f(1), nsSimdVec4f(4));
    nsSimdBBox b4(nsSimdVec4f(-4, 1, 1), nsSimdVec4f(4, 2, 2));

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

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Overlaps (Sphere)")
  {
    nsSimdBBox b(nsSimdVec4f(1), nsSimdVec4f(5));

    NS_TEST_BOOL(b.Overlaps(nsSimdBSphere(nsSimdVec4f(3), 2)));
    NS_TEST_BOOL(b.Overlaps(nsSimdBSphere(nsSimdVec4f(3), 2.1f)));
    NS_TEST_BOOL(!b.Overlaps(nsSimdBSphere(nsSimdVec4f(8), 2)));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Grow")
  {
    nsSimdBBox b(nsSimdVec4f(1, 2, 3), nsSimdVec4f(4, 5, 6));
    b.Grow(nsSimdVec4f(2, 4, 6));

    NS_TEST_BOOL((b.m_Min == nsSimdVec4f(-1, -2, -3)).AllSet<3>());
    NS_TEST_BOOL((b.m_Max == nsSimdVec4f(6, 9, 12)).AllSet<3>());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Transform")
  {
    nsSimdBBox b(nsSimdVec4f(3), nsSimdVec4f(5));

    nsSimdTransform t(nsSimdVec4f(4, 5, 6));
    t.m_Rotation = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 0, 1), nsAngle::MakeFromDegree(90));
    t.m_Scale = nsSimdVec4f(1, -2, -4);

    b.Transform(t);

    NS_TEST_SIMD_VECTOR_EQUAL(3, b.m_Min, nsSimdVec4f(10, 8, -14), 0.00001f);
    NS_TEST_SIMD_VECTOR_EQUAL(3, b.m_Max, nsSimdVec4f(14, 10, -6), 0.00001f);

    t.m_Rotation = nsSimdQuat::MakeFromAxisAndAngle(nsSimdVec4f(0, 0, 1), nsAngle::MakeFromDegree(-30));

    b.m_Min = nsSimdVec4f(3);
    b.m_Max = nsSimdVec4f(5);
    b.Transform(t);

    // reference
    nsBoundingBox referenceBox = nsBoundingBoxT::MakeFromMinMax(nsVec3(3), nsVec3(5));
    {
      nsQuat q = nsQuat::MakeFromAxisAndAngle(nsVec3(0, 0, 1), nsAngle::MakeFromDegree(-30));

      nsTransform referenceTransform(nsVec3(4, 5, 6), q, nsVec3(1, -2, -4));

      referenceBox.TransformFromOrigin(referenceTransform.GetAsMat4());
    }

    NS_TEST_SIMD_VECTOR_EQUAL(3, b.m_Min, nsSimdConversion::ToVec3(referenceBox.m_vMin), 0.00001f);
    NS_TEST_SIMD_VECTOR_EQUAL(3, b.m_Max, nsSimdConversion::ToVec3(referenceBox.m_vMax), 0.00001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetClampedPoint")
  {
    nsSimdBBox b(nsSimdVec4f(-1, -2, -3), nsSimdVec4f(1, 2, 3));

    NS_TEST_BOOL((b.GetClampedPoint(nsSimdVec4f(-2, 0, 0)) == nsSimdVec4f(-1, 0, 0)).AllSet<3>());
    NS_TEST_BOOL((b.GetClampedPoint(nsSimdVec4f(2, 0, 0)) == nsSimdVec4f(1, 0, 0)).AllSet<3>());

    NS_TEST_BOOL((b.GetClampedPoint(nsSimdVec4f(0, -3, 0)) == nsSimdVec4f(0, -2, 0)).AllSet<3>());
    NS_TEST_BOOL((b.GetClampedPoint(nsSimdVec4f(0, 3, 0)) == nsSimdVec4f(0, 2, 0)).AllSet<3>());

    NS_TEST_BOOL((b.GetClampedPoint(nsSimdVec4f(0, 0, -4)) == nsSimdVec4f(0, 0, -3)).AllSet<3>());
    NS_TEST_BOOL((b.GetClampedPoint(nsSimdVec4f(0, 0, 4)) == nsSimdVec4f(0, 0, 3)).AllSet<3>());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetDistanceSquaredTo (point)")
  {
    nsSimdBBox b(nsSimdVec4f(-1, -2, -3), nsSimdVec4f(1, 2, 3));

    NS_TEST_BOOL(b.GetDistanceSquaredTo(nsSimdVec4f(-2, 0, 0)) == 1.0f);
    NS_TEST_BOOL(b.GetDistanceSquaredTo(nsSimdVec4f(2, 0, 0)) == 1.0f);

    NS_TEST_BOOL(b.GetDistanceSquaredTo(nsSimdVec4f(0, -4, 0)) == 4.0f);
    NS_TEST_BOOL(b.GetDistanceSquaredTo(nsSimdVec4f(0, 4, 0)) == 4.0f);

    NS_TEST_BOOL(b.GetDistanceSquaredTo(nsSimdVec4f(0, 0, -6)) == 9.0f);
    NS_TEST_BOOL(b.GetDistanceSquaredTo(nsSimdVec4f(0, 0, 6)) == 9.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetDistanceTo (point)")
  {
    nsSimdBBox b(nsSimdVec4f(-1, -2, -3), nsSimdVec4f(1, 2, 3));

    NS_TEST_BOOL(b.GetDistanceTo(nsSimdVec4f(-2, 0, 0)) == 1.0f);
    NS_TEST_BOOL(b.GetDistanceTo(nsSimdVec4f(2, 0, 0)) == 1.0f);

    NS_TEST_BOOL(b.GetDistanceTo(nsSimdVec4f(0, -4, 0)) == 2.0f);
    NS_TEST_BOOL(b.GetDistanceTo(nsSimdVec4f(0, 4, 0)) == 2.0f);

    NS_TEST_BOOL(b.GetDistanceTo(nsSimdVec4f(0, 0, -6)) == 3.0f);
    NS_TEST_BOOL(b.GetDistanceTo(nsSimdVec4f(0, 0, 6)) == 3.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Comparison")
  {
    nsSimdBBox b1(nsSimdVec4f(5, 0, 0), nsSimdVec4f(1, 2, 3));
    nsSimdBBox b2(nsSimdVec4f(6, 0, 0), nsSimdVec4f(1, 2, 3));

    NS_TEST_BOOL(b1 == nsSimdBBox(nsSimdVec4f(5, 0, 0), nsSimdVec4f(1, 2, 3)));
    NS_TEST_BOOL(b1 != b2);
  }
}
