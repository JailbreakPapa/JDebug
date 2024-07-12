#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Math/Vec2.h>
#include <Foundation/Math/Vec3.h>
#include <Foundation/Math/Vec4.h>

#include <Foundation/Math/FixedPoint.h>

NS_CREATE_SIMPLE_TEST(Math, Vec2)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor")
  {
#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
    if (nsMath::SupportsNaN<nsMathTestType>())
    {
      // In debug the default constructor initializes everything with NaN.
      nsVec2T vDefCtor;
      NS_TEST_BOOL(nsMath::IsNaN(vDefCtor.x) && nsMath::IsNaN(vDefCtor.y));
    }
#else
    // Placement new of the default constructor should not have any effect on the previous data.
    nsVec2T::ComponentType testBlock[2] = {(nsVec2T::ComponentType)1, (nsVec2T::ComponentType)2};
    nsVec2T* pDefCtor = ::new ((void*)&testBlock[0]) nsVec2T;
    NS_TEST_BOOL(pDefCtor->x == (nsVec2T::ComponentType)1 && pDefCtor->y == (nsVec2T::ComponentType)2);
#endif
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor(x,y)")
  {
    nsVec2T v(1, 2);
    NS_TEST_FLOAT(v.x, 1, 0);
    NS_TEST_FLOAT(v.y, 2, 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor(xy)")
  {
    nsVec2T v(3);
    NS_TEST_VEC2(v, nsVec2T(3, 3), 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeZero")
  {
    NS_TEST_VEC2(nsVec2T::MakeZero(), nsVec2T(0, 0), 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetAsVec3")
  {
    NS_TEST_VEC3(nsVec2T(2, 3).GetAsVec3(4), nsVec3T(2, 3, 4), 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetAsVec4")
  {
    NS_TEST_VEC4(nsVec2T(2, 3).GetAsVec4(4, 5), nsVec4T(2, 3, 4, 5), 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Set(x, y)")
  {
    nsVec2T v;
    v.Set(2, 3);

    NS_TEST_FLOAT(v.x, 2, 0);
    NS_TEST_FLOAT(v.y, 3, 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Set(xy)")
  {
    nsVec2T v;
    v.Set(4);

    NS_TEST_FLOAT(v.x, 4, 0);
    NS_TEST_FLOAT(v.y, 4, 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetZero")
  {
    nsVec2T v;
    v.Set(4);
    v.SetZero();

    NS_TEST_FLOAT(v.x, 0, 0);
    NS_TEST_FLOAT(v.y, 0, 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetLength")
  {
    nsVec2T v(0);
    NS_TEST_FLOAT(v.GetLength(), 0, 0.0001f);

    v.Set(1, 0);
    NS_TEST_FLOAT(v.GetLength(), 1, 0.0001f);

    v.Set(0, 1);
    NS_TEST_FLOAT(v.GetLength(), 1, 0.0001f);

    v.Set(2, 3);
    NS_TEST_FLOAT(v.GetLength(), nsMath::Sqrt((nsMathTestType)(4 + 9)), 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetLengthSquared")
  {
    nsVec2T v(0);
    NS_TEST_FLOAT(v.GetLengthSquared(), 0, 0.0001f);

    v.Set(1, 0);
    NS_TEST_FLOAT(v.GetLengthSquared(), 1, 0.0001f);

    v.Set(0, 1);
    NS_TEST_FLOAT(v.GetLengthSquared(), 1, 0.0001f);

    v.Set(2, 3);
    NS_TEST_FLOAT(v.GetLengthSquared(), 4 + 9, 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetLengthAndNormalize")
  {
    nsVec2T v(0.5f, 0);
    nsVec2T::ComponentType l = v.GetLengthAndNormalize();
    NS_TEST_FLOAT(l, 0.5f, 0.0001f);
    NS_TEST_FLOAT(v.GetLength(), 1, 0.0001f);

    v.Set(1, 0);
    l = v.GetLengthAndNormalize();
    NS_TEST_FLOAT(l, 1, 0.0001f);
    NS_TEST_FLOAT(v.GetLength(), 1, 0.0001f);

    v.Set(0, 1);
    l = v.GetLengthAndNormalize();
    NS_TEST_FLOAT(l, 1, 0.0001f);
    NS_TEST_FLOAT(v.GetLength(), 1, 0.0001f);

    v.Set(2, 3);
    l = v.GetLengthAndNormalize();
    NS_TEST_FLOAT(l, nsMath::Sqrt((nsMathTestType)(4 + 9)), 0.0001f);
    NS_TEST_FLOAT(v.GetLength(), 1, 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetNormalized")
  {
    nsVec2T v;

    v.Set(10, 0);
    NS_TEST_VEC2(v.GetNormalized(), nsVec2T(1, 0), 0.001f);

    v.Set(0, 10);
    NS_TEST_VEC2(v.GetNormalized(), nsVec2T(0, 1), 0.001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Normalize")
  {
    nsVec2T v;

    v.Set(10, 0);
    v.Normalize();
    NS_TEST_VEC2(v, nsVec2T(1, 0), 0.001f);

    v.Set(0, 10);
    v.Normalize();
    NS_TEST_VEC2(v, nsVec2T(0, 1), 0.001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "NormalizeIfNotZero")
  {
    nsVec2T v;

    v.Set(10, 0);
    NS_TEST_BOOL(v.NormalizeIfNotZero() == NS_SUCCESS);
    NS_TEST_VEC2(v, nsVec2T(1, 0), 0.001f);

    v.Set(0, 10);
    NS_TEST_BOOL(v.NormalizeIfNotZero() == NS_SUCCESS);
    NS_TEST_VEC2(v, nsVec2T(0, 1), 0.001f);

    v.SetZero();
    NS_TEST_BOOL(v.NormalizeIfNotZero() == NS_FAILURE);
    NS_TEST_VEC2(v, nsVec2T(1, 0), 0.001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsZero")
  {
    nsVec2T v;

    v.Set(1);
    NS_TEST_BOOL(v.IsZero() == false);

    v.Set(0.001f);
    NS_TEST_BOOL(v.IsZero() == false);
    NS_TEST_BOOL(v.IsZero(0.01f) == true);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsNormalized")
  {
    nsVec2T v;

    v.SetZero();
    NS_TEST_BOOL(v.IsNormalized(nsMath::HugeEpsilon<nsMathTestType>()) == false);

    v.Set(1, 0);
    NS_TEST_BOOL(v.IsNormalized(nsMath::HugeEpsilon<nsMathTestType>()) == true);

    v.Set(0, 1);
    NS_TEST_BOOL(v.IsNormalized(nsMath::HugeEpsilon<nsMathTestType>()) == true);

    v.Set(0.1f, 1);
    NS_TEST_BOOL(v.IsNormalized(nsMath::DefaultEpsilon<nsMathTestType>()) == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsNaN")
  {
    if (nsMath::SupportsNaN<nsMathTestType>())
    {
      nsVec2T v(0);

      NS_TEST_BOOL(!v.IsNaN());

      v.x = nsMath::NaN<nsMathTestType>();
      NS_TEST_BOOL(v.IsNaN());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsValid")
  {
    if (nsMath::SupportsNaN<nsMathTestType>())
    {
      nsVec2T v(0);

      NS_TEST_BOOL(v.IsValid());

      v.x = nsMath::NaN<nsMathTestType>();
      NS_TEST_BOOL(!v.IsValid());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator-")
  {
    nsVec2T v(1);

    NS_TEST_VEC2(-v, nsVec2T(-1), 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator+=")
  {
    nsVec2T v(1, 2);

    v += nsVec2T(3, 4);
    NS_TEST_VEC2(v, nsVec2T(4, 6), 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator-=")
  {
    nsVec2T v(1, 2);

    v -= nsVec2T(3, 5);
    NS_TEST_VEC2(v, nsVec2T(-2, -3), 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator*=(float)")
  {
    nsVec2T v(1, 2);

    v *= 3;
    NS_TEST_VEC2(v, nsVec2T(3, 6), 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator/=(float)")
  {
    nsVec2T v(1, 2);

    v /= 2;
    NS_TEST_VEC2(v, nsVec2T(0.5f, 1), 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsIdentical")
  {
    nsVec2T v1(1, 2);
    nsVec2T v2 = v1;

    NS_TEST_BOOL(v1.IsIdentical(v2));

    v2.x += nsVec2T::ComponentType(0.001f);
    NS_TEST_BOOL(!v1.IsIdentical(v2));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsEqual")
  {
    nsVec2T v1(1, 2);
    nsVec2T v2 = v1;

    NS_TEST_BOOL(v1.IsEqual(v2, 0.00001f));

    v2.x += nsVec2T::ComponentType(0.001f);
    NS_TEST_BOOL(!v1.IsEqual(v2, 0.0001f));
    NS_TEST_BOOL(v1.IsEqual(v2, 0.01f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetAngleBetween")
  {
    nsVec2T v1(1, 0);
    nsVec2T v2(0, 1);

    NS_TEST_FLOAT(v1.GetAngleBetween(v1).GetDegree(), 0, 0.001f);
    NS_TEST_FLOAT(v2.GetAngleBetween(v2).GetDegree(), 0, 0.001f);
    NS_TEST_FLOAT(v1.GetAngleBetween(v2).GetDegree(), 90, 0.001f);
    NS_TEST_FLOAT(v1.GetAngleBetween(-v1).GetDegree(), 180, 0.001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Dot")
  {
    nsVec2T v1(1, 0);
    nsVec2T v2(0, 1);
    nsVec2T v0(0, 0);

    NS_TEST_FLOAT(v0.Dot(v0), 0, 0.001f);
    NS_TEST_FLOAT(v1.Dot(v1), 1, 0.001f);
    NS_TEST_FLOAT(v2.Dot(v2), 1, 0.001f);
    NS_TEST_FLOAT(v1.Dot(v2), 0, 0.001f);
    NS_TEST_FLOAT(v1.Dot(-v1), -1, 0.001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CompMin")
  {
    nsVec2T v1(2, 3);
    nsVec2T v2 = v1.CompMin(nsVec2T(1, 4));
    NS_TEST_VEC2(v2, nsVec2T(1, 3), 0);

    v2 = v1.CompMin(nsVec2T(3, 1));
    NS_TEST_VEC2(v2, nsVec2T(2, 1), 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CompMax")
  {
    nsVec2T v1(2, 3.5f);
    nsVec2T v2 = v1.CompMax(nsVec2T(1, 4));
    NS_TEST_VEC2(v2, nsVec2T(2, 4), 0);

    v2 = v1.CompMax(nsVec2T(3, 1));
    NS_TEST_VEC2(v2, nsVec2T(3, 3.5f), 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CompClamp")
  {
    const nsVec2T vOp1(-4.0, 0.2f);
    const nsVec2T vOp2(2.0, -0.3f);

    NS_TEST_BOOL(vOp1.CompClamp(vOp1, vOp2).IsEqual(nsVec2T(-4.0f, -0.3f), nsMath::SmallEpsilon<nsMathTestType>()));
    NS_TEST_BOOL(vOp2.CompClamp(vOp1, vOp2).IsEqual(nsVec2T(2.0f, 0.2f), nsMath::SmallEpsilon<nsMathTestType>()));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CompMul")
  {
    nsVec2T v1(2, 3);
    nsVec2T v2 = v1.CompMul(nsVec2T(2, 4));
    NS_TEST_VEC2(v2, nsVec2T(4, 12), 0);

    v2 = v1.CompMul(nsVec2T(3, 7));
    NS_TEST_VEC2(v2, nsVec2T(6, 21), 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CompDiv")
  {
    nsVec2T v1(12, 32);
    nsVec2T v2 = v1.CompDiv(nsVec2T(3, 4));
    NS_TEST_VEC2(v2, nsVec2T(4, 8), 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Abs")
  {
    nsVec2T v1(-5, 7);
    nsVec2T v2 = v1.Abs();
    NS_TEST_VEC2(v2, nsVec2T(5, 7), 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeOrthogonalTo")
  {
    nsVec2T v;

    v.Set(1, 1);
    v.MakeOrthogonalTo(nsVec2T(1, 0));
    NS_TEST_VEC2(v, nsVec2T(0, 1), 0.001f);

    v.Set(1, 1);
    v.MakeOrthogonalTo(nsVec2T(0, 1));
    NS_TEST_VEC2(v, nsVec2T(1, 0), 0.001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetOrthogonalVector")
  {
    nsVec2T v;

    for (float i = 1; i < 360; i += 3)
    {
      v.Set(i, i * 3);
      NS_TEST_FLOAT(v.GetOrthogonalVector().Dot(v), 0, 0.001f);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetReflectedVector")
  {
    nsVec2T v, v2;

    v.Set(1, 1);
    v2 = v.GetReflectedVector(nsVec2T(0, -1));
    NS_TEST_VEC2(v2, nsVec2T(1, -1), 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator+")
  {
    nsVec2T v = nsVec2T(1, 2) + nsVec2T(3, 4);
    NS_TEST_VEC2(v, nsVec2T(4, 6), 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator-")
  {
    nsVec2T v = nsVec2T(1, 2) - nsVec2T(3, 5);
    NS_TEST_VEC2(v, nsVec2T(-2, -3), 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator* (vec, float) | operator* (float, vec)")
  {
    nsVec2T v = nsVec2T(1, 2) * nsVec2T::ComponentType(3);
    NS_TEST_VEC2(v, nsVec2T(3, 6), 0.0001f);

    v = nsVec2T::ComponentType(7) * nsVec2T(1, 2);
    NS_TEST_VEC2(v, nsVec2T(7, 14), 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator/ (vec, float)")
  {
    nsVec2T v = nsVec2T(2, 4) / nsVec2T::ComponentType(2);
    NS_TEST_VEC2(v, nsVec2T(1, 2), 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator== | operator!=")
  {
    nsVec2T v1(1, 2);
    nsVec2T v2 = v1;

    NS_TEST_BOOL(v1 == v2);

    v2.x += nsVec2T::ComponentType(0.001f);
    NS_TEST_BOOL(v1 != v2);
  }
}
