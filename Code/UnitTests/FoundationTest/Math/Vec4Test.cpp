#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Math/Vec2.h>
#include <Foundation/Math/Vec3.h>
#include <Foundation/Math/Vec4.h>


NS_CREATE_SIMPLE_TEST(Math, Vec4)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor")
  {
#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
    if (nsMath::SupportsNaN<nsMathTestType>())
    {
      // In debug the default constructor initializes everything with NaN.
      nsVec4T vDefCtor;
      NS_TEST_BOOL(nsMath::IsNaN(vDefCtor.x) && nsMath::IsNaN(vDefCtor.y) /* && nsMath::IsNaN(vDefCtor.z) && nsMath::IsNaN(vDefCtor.w)*/);
    }
#else
    // Placement new of the default constructor should not have any effect on the previous data.
    nsVec4T::ComponentType testBlock[4] = {
      (nsVec4T::ComponentType)1, (nsVec4T::ComponentType)2, (nsVec4T::ComponentType)3, (nsVec4T::ComponentType)4};
    nsVec4T* pDefCtor = ::new ((void*)&testBlock[0]) nsVec4T;
    NS_TEST_BOOL(pDefCtor->x == (nsVec4T::ComponentType)1 && pDefCtor->y == (nsVec4T::ComponentType)2 && pDefCtor->z == (nsVec4T::ComponentType)3 &&
                 pDefCtor->w == (nsVec4T::ComponentType)4);
#endif

    // Make sure the class didn't accidentally change in size.
    NS_TEST_BOOL(sizeof(nsVec4) == 16);
    NS_TEST_BOOL(sizeof(nsVec4d) == 32);

    nsVec4T vInit1F(2.0f);
    NS_TEST_BOOL(vInit1F.x == 2.0f && vInit1F.y == 2.0f && vInit1F.z == 2.0f && vInit1F.w == 2.0f);

    nsVec4T vInit4F(1.0f, 2.0f, 3.0f, 4.0f);
    NS_TEST_BOOL(vInit4F.x == 1.0f && vInit4F.y == 2.0f && vInit4F.z == 3.0f && vInit4F.w == 4.0f);

    nsVec4T vCopy(vInit4F);
    NS_TEST_BOOL(vCopy.x == 1.0f && vCopy.y == 2.0f && vCopy.z == 3.0f && vCopy.w == 4.0f);

    nsVec4T vZero = nsVec4T::MakeZero();
    NS_TEST_BOOL(vZero.x == 0.0f && vZero.y == 0.0f && vZero.z == 0.0f && vZero.w == 0.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Conversion")
  {
    nsVec4T vData(1.0f, 2.0f, 3.0f, 4.0f);
    nsVec2T vToVec2 = vData.GetAsVec2();
    NS_TEST_BOOL(vToVec2.x == vData.x && vToVec2.y == vData.y);

    nsVec3T vToVec3 = vData.GetAsVec3();
    NS_TEST_BOOL(vToVec3.x == vData.x && vToVec3.y == vData.y && vToVec3.z == vData.z);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Setter")
  {
    nsVec4T vSet1F;
    vSet1F.Set(2.0f);
    NS_TEST_BOOL(vSet1F.x == 2.0f && vSet1F.y == 2.0f && vSet1F.z == 2.0f && vSet1F.w == 2.0f);

    nsVec4T vSet4F;
    vSet4F.Set(1.0f, 2.0f, 3.0f, 4.0f);
    NS_TEST_BOOL(vSet4F.x == 1.0f && vSet4F.y == 2.0f && vSet4F.z == 3.0f && vSet4F.w == 4.0f);

    nsVec4T vSetZero;
    vSetZero.SetZero();
    NS_TEST_BOOL(vSetZero.x == 0.0f && vSetZero.y == 0.0f && vSetZero.z == 0.0f && vSetZero.w == 0.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Length")
  {
    const nsVec4T vOp1(-4.0, 4.0f, -2.0f, -0.0f);
    const nsVec4T compArray[4] = {
      nsVec4T(1.0f, 0.0f, 0.0f, 0.0f), nsVec4T(0.0f, 1.0f, 0.0f, 0.0f), nsVec4T(0.0f, 0.0f, 1.0f, 0.0f), nsVec4T(0.0f, 0.0f, 0.0f, 1.0f)};

    // GetLength
    NS_TEST_FLOAT(vOp1.GetLength(), 6.0f, nsMath::SmallEpsilon<nsMathTestType>());

    // GetLengthSquared
    NS_TEST_FLOAT(vOp1.GetLengthSquared(), 36.0f, nsMath::SmallEpsilon<nsMathTestType>());

    // GetLengthAndNormalize
    nsVec4T vLengthAndNorm = vOp1;
    nsMathTestType fLength = vLengthAndNorm.GetLengthAndNormalize();
    NS_TEST_FLOAT(vLengthAndNorm.GetLength(), 1.0f, nsMath::SmallEpsilon<nsMathTestType>());
    NS_TEST_FLOAT(fLength, 6.0f, nsMath::SmallEpsilon<nsMathTestType>());
    NS_TEST_FLOAT(vLengthAndNorm.x * vLengthAndNorm.x + vLengthAndNorm.y * vLengthAndNorm.y + vLengthAndNorm.z * vLengthAndNorm.z +
                    vLengthAndNorm.w * vLengthAndNorm.w,
      1.0f, nsMath::SmallEpsilon<nsMathTestType>());
    NS_TEST_BOOL(vLengthAndNorm.IsNormalized(nsMath::SmallEpsilon<nsMathTestType>()));

    // GetNormalized
    nsVec4T vGetNorm = vOp1.GetNormalized();
    NS_TEST_FLOAT(vGetNorm.x * vGetNorm.x + vGetNorm.y * vGetNorm.y + vGetNorm.z * vGetNorm.z + vGetNorm.w * vGetNorm.w, 1.0f,
      nsMath::SmallEpsilon<nsMathTestType>());
    NS_TEST_BOOL(vGetNorm.IsNormalized(nsMath::SmallEpsilon<nsMathTestType>()));

    // Normalize
    nsVec4T vNorm = vOp1;
    vNorm.Normalize();
    NS_TEST_FLOAT(vNorm.x * vNorm.x + vNorm.y * vNorm.y + vNorm.z * vNorm.z + vNorm.w * vNorm.w, 1.0f, nsMath::SmallEpsilon<nsMathTestType>());
    NS_TEST_BOOL(vNorm.IsNormalized(nsMath::SmallEpsilon<nsMathTestType>()));

    // NormalizeIfNotZero
    nsVec4T vNormCond = vNorm * nsMath::DefaultEpsilon<nsMathTestType>();
    NS_TEST_BOOL(vNormCond.NormalizeIfNotZero(vOp1, nsMath::LargeEpsilon<nsMathTestType>()) == NS_FAILURE);
    NS_TEST_BOOL(vNormCond == vOp1);
    vNormCond = vNorm * nsMath::DefaultEpsilon<nsMathTestType>();
    NS_TEST_BOOL(vNormCond.NormalizeIfNotZero(vOp1, nsMath::SmallEpsilon<nsMathTestType>()) == NS_SUCCESS);
    NS_TEST_VEC4(vNormCond, vNorm, nsMath::DefaultEpsilon<nsVec4T::ComponentType>());

    // IsZero
    NS_TEST_BOOL(nsVec4T::MakeZero().IsZero());
    for (int i = 0; i < 4; ++i)
    {
      NS_TEST_BOOL(!compArray[i].IsZero());
    }

    // IsZero(float)
    NS_TEST_BOOL(nsVec4T::MakeZero().IsZero(0.0f));
    for (int i = 0; i < 4; ++i)
    {
      NS_TEST_BOOL(!compArray[i].IsZero(0.0f));
      NS_TEST_BOOL(compArray[i].IsZero(1.0f));
      NS_TEST_BOOL((-compArray[i]).IsZero(1.0f));
    }

    // IsNormalized (already tested above)
    for (int i = 0; i < 4; ++i)
    {
      NS_TEST_BOOL(compArray[i].IsNormalized());
      NS_TEST_BOOL((-compArray[i]).IsNormalized());
      NS_TEST_BOOL((compArray[i] * (nsMathTestType)2).IsNormalized((nsMathTestType)4));
      NS_TEST_BOOL((compArray[i] * (nsMathTestType)2).IsNormalized((nsMathTestType)4));
    }

    if (nsMath::SupportsNaN<nsMathTestType>())
    {
      nsMathTestType TypeNaN = nsMath::NaN<nsMathTestType>();
      const nsVec4T nanArray[4] = {nsVec4T(TypeNaN, 0.0f, 0.0f, 0.0f), nsVec4T(0.0f, TypeNaN, 0.0f, 0.0f), nsVec4T(0.0f, 0.0f, TypeNaN, 0.0f),
        nsVec4T(0.0f, 0.0f, 0.0f, TypeNaN)};

      // IsNaN
      for (int i = 0; i < 4; ++i)
      {
        NS_TEST_BOOL(nanArray[i].IsNaN());
        NS_TEST_BOOL(!compArray[i].IsNaN());
      }

      // IsValid
      for (int i = 0; i < 4; ++i)
      {
        NS_TEST_BOOL(!nanArray[i].IsValid());
        NS_TEST_BOOL(compArray[i].IsValid());

        NS_TEST_BOOL(!(compArray[i] * nsMath::Infinity<nsMathTestType>()).IsValid());
        NS_TEST_BOOL(!(compArray[i] * -nsMath::Infinity<nsMathTestType>()).IsValid());
      }
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Operators")
  {
    const nsVec4T vOp1(-4.0, 0.2f, -7.0f, -0.0f);
    const nsVec4T vOp2(2.0, 0.3f, 0.0f, 1.0f);
    const nsVec4T compArray[4] = {
      nsVec4T(1.0f, 0.0f, 0.0f, 0.0f), nsVec4T(0.0f, 1.0f, 0.0f, 0.0f), nsVec4T(0.0f, 0.0f, 1.0f, 0.0f), nsVec4T(0.0f, 0.0f, 0.0f, 1.0f)};
    // IsIdentical
    NS_TEST_BOOL(vOp1.IsIdentical(vOp1));
    for (int i = 0; i < 4; ++i)
    {
      NS_TEST_BOOL(!vOp1.IsIdentical(vOp1 + (nsMathTestType)nsMath::SmallEpsilon<nsMathTestType>() * compArray[i]));
      NS_TEST_BOOL(!vOp1.IsIdentical(vOp1 - (nsMathTestType)nsMath::SmallEpsilon<nsMathTestType>() * compArray[i]));
    }

    // IsEqual
    NS_TEST_BOOL(vOp1.IsEqual(vOp1, 0.0f));
    for (int i = 0; i < 4; ++i)
    {
      NS_TEST_BOOL(vOp1.IsEqual(vOp1 + nsMath::SmallEpsilon<nsMathTestType>() * compArray[i], 2 * nsMath::SmallEpsilon<nsMathTestType>()));
      NS_TEST_BOOL(vOp1.IsEqual(vOp1 - nsMath::SmallEpsilon<nsMathTestType>() * compArray[i], 2 * nsMath::SmallEpsilon<nsMathTestType>()));
      NS_TEST_BOOL(vOp1.IsEqual(vOp1 + nsMath::DefaultEpsilon<nsMathTestType>() * compArray[i], 2 * nsMath::DefaultEpsilon<nsMathTestType>()));
      NS_TEST_BOOL(vOp1.IsEqual(vOp1 - nsMath::DefaultEpsilon<nsMathTestType>() * compArray[i], 2 * nsMath::DefaultEpsilon<nsMathTestType>()));
    }

    // operator-
    nsVec4T vNegated = -vOp1;
    NS_TEST_BOOL(vOp1.x == -vNegated.x && vOp1.y == -vNegated.y && vOp1.z == -vNegated.z && vOp1.w == -vNegated.w);

    // operator+= (nsVec4T)
    nsVec4T vPlusAssign = vOp1;
    vPlusAssign += vOp2;
    NS_TEST_BOOL(vPlusAssign.IsEqual(nsVec4T(-2.0f, 0.5f, -7.0f, 1.0f), nsMath::SmallEpsilon<nsMathTestType>()));

    // operator-= (nsVec4T)
    nsVec4T vMinusAssign = vOp1;
    vMinusAssign -= vOp2;
    NS_TEST_BOOL(vMinusAssign.IsEqual(nsVec4T(-6.0f, -0.1f, -7.0f, -1.0f), nsMath::SmallEpsilon<nsMathTestType>()));

    // operator*= (float)
    nsVec4T vMulFloat = vOp1;
    vMulFloat *= 2.0f;
    NS_TEST_BOOL(vMulFloat.IsEqual(nsVec4T(-8.0f, 0.4f, -14.0f, -0.0f), nsMath::SmallEpsilon<nsMathTestType>()));
    vMulFloat *= 0.0f;
    NS_TEST_BOOL(vMulFloat.IsEqual(nsVec4T::MakeZero(), nsMath::SmallEpsilon<nsMathTestType>()));

    // operator/= (float)
    nsVec4T vDivFloat = vOp1;
    vDivFloat /= 2.0f;
    NS_TEST_BOOL(vDivFloat.IsEqual(nsVec4T(-2.0f, 0.1f, -3.5f, -0.0f), nsMath::SmallEpsilon<nsMathTestType>()));

    // operator+ (nsVec4T, nsVec4T)
    nsVec4T vPlus = (vOp1 + vOp2);
    NS_TEST_BOOL(vPlus.IsEqual(nsVec4T(-2.0f, 0.5f, -7.0f, 1.0f), nsMath::SmallEpsilon<nsMathTestType>()));

    // operator- (nsVec4T, nsVec4T)
    nsVec4T vMinus = (vOp1 - vOp2);
    NS_TEST_BOOL(vMinus.IsEqual(nsVec4T(-6.0f, -0.1f, -7.0f, -1.0f), nsMath::SmallEpsilon<nsMathTestType>()));

    // operator* (float, nsVec4T)
    nsVec4T vMulFloatVec4 = ((nsMathTestType)2 * vOp1);
    NS_TEST_BOOL(vMulFloatVec4.IsEqual(nsVec4T(-8.0f, 0.4f, -14.0f, -0.0f), nsMath::SmallEpsilon<nsMathTestType>()));
    vMulFloatVec4 = ((nsMathTestType)0 * vOp1);
    NS_TEST_BOOL(vMulFloatVec4.IsEqual(nsVec4T::MakeZero(), nsMath::SmallEpsilon<nsMathTestType>()));

    // operator* (nsVec4T, float)
    nsVec4T vMulVec4Float = (vOp1 * (nsMathTestType)2);
    NS_TEST_BOOL(vMulVec4Float.IsEqual(nsVec4T(-8.0f, 0.4f, -14.0f, -0.0f), nsMath::SmallEpsilon<nsMathTestType>()));
    vMulVec4Float = (vOp1 * (nsMathTestType)0);
    NS_TEST_BOOL(vMulVec4Float.IsEqual(nsVec4T::MakeZero(), nsMath::SmallEpsilon<nsMathTestType>()));

    // operator/ (nsVec4T, float)
    nsVec4T vDivVec4Float = (vOp1 / (nsMathTestType)2);
    NS_TEST_BOOL(vDivVec4Float.IsEqual(nsVec4T(-2.0f, 0.1f, -3.5f, -0.0f), nsMath::SmallEpsilon<nsMathTestType>()));

    // operator== (nsVec4T, nsVec4T)
    NS_TEST_BOOL(vOp1 == vOp1);
    for (int i = 0; i < 4; ++i)
    {
      NS_TEST_BOOL(!(vOp1 == (vOp1 + (nsMathTestType)nsMath::SmallEpsilon<nsMathTestType>() * compArray[i])));
      NS_TEST_BOOL(!(vOp1 == (vOp1 - (nsMathTestType)nsMath::SmallEpsilon<nsMathTestType>() * compArray[i])));
    }

    // operator!= (nsVec4T, nsVec4T)
    NS_TEST_BOOL(!(vOp1 != vOp1));
    for (int i = 0; i < 4; ++i)
    {
      NS_TEST_BOOL(vOp1 != (vOp1 + (nsMathTestType)nsMath::SmallEpsilon<nsMathTestType>() * compArray[i]));
      NS_TEST_BOOL(vOp1 != (vOp1 - (nsMathTestType)nsMath::SmallEpsilon<nsMathTestType>() * compArray[i]));
    }

    // operator< (nsVec4T, nsVec4T)
    for (int i = 0; i < 4; ++i)
    {
      for (int j = 0; j < 4; ++j)
      {
        if (i == j)
        {
          NS_TEST_BOOL(!(compArray[i] < compArray[j]));
          NS_TEST_BOOL(!(compArray[j] < compArray[i]));
        }
        else if (i < j)
        {
          NS_TEST_BOOL(!(compArray[i] < compArray[j]));
          NS_TEST_BOOL(compArray[j] < compArray[i]);
        }
        else
        {
          NS_TEST_BOOL(!(compArray[j] < compArray[i]));
          NS_TEST_BOOL(compArray[i] < compArray[j]);
        }
      }
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Common")
  {
    const nsVec4T vOp1(-4.0, 0.2f, -7.0f, -0.0f);
    const nsVec4T vOp2(2.0, -0.3f, 0.5f, 1.0f);

    // Dot
    NS_TEST_FLOAT(vOp1.Dot(vOp2), -11.56f, nsMath::SmallEpsilon<nsMathTestType>());
    NS_TEST_FLOAT(vOp2.Dot(vOp1), -11.56f, nsMath::SmallEpsilon<nsMathTestType>());

    // CompMin
    NS_TEST_BOOL(vOp1.CompMin(vOp2).IsEqual(nsVec4T(-4.0f, -0.3f, -7.0f, -0.0f), nsMath::SmallEpsilon<nsMathTestType>()));
    NS_TEST_BOOL(vOp2.CompMin(vOp1).IsEqual(nsVec4T(-4.0f, -0.3f, -7.0f, -0.0f), nsMath::SmallEpsilon<nsMathTestType>()));

    // CompMax
    NS_TEST_BOOL(vOp1.CompMax(vOp2).IsEqual(nsVec4T(2.0f, 0.2f, 0.5f, 1.0f), nsMath::SmallEpsilon<nsMathTestType>()));
    NS_TEST_BOOL(vOp2.CompMax(vOp1).IsEqual(nsVec4T(2.0f, 0.2f, 0.5f, 1.0f), nsMath::SmallEpsilon<nsMathTestType>()));

    // CompClamp
    NS_TEST_BOOL(vOp1.CompClamp(vOp1, vOp2).IsEqual(nsVec4T(-4.0f, -0.3f, -7.0f, -0.0f), nsMath::SmallEpsilon<nsMathTestType>()));
    NS_TEST_BOOL(vOp2.CompClamp(vOp1, vOp2).IsEqual(nsVec4T(2.0f, 0.2f, 0.5f, 1.0f), nsMath::SmallEpsilon<nsMathTestType>()));

    // CompMul
    NS_TEST_BOOL(vOp1.CompMul(vOp2).IsEqual(nsVec4T(-8.0f, -0.06f, -3.5f, 0.0f), nsMath::SmallEpsilon<nsMathTestType>()));
    NS_TEST_BOOL(vOp2.CompMul(vOp1).IsEqual(nsVec4T(-8.0f, -0.06f, -3.5f, 0.0f), nsMath::SmallEpsilon<nsMathTestType>()));

    // CompDiv
    NS_TEST_BOOL(vOp1.CompDiv(vOp2).IsEqual(nsVec4T(-2.0f, -0.66666666f, -14.0f, 0.0f), nsMath::SmallEpsilon<nsMathTestType>()));

    // Abs
    NS_TEST_VEC4(vOp1.Abs(), nsVec4T(4.0, 0.2f, 7.0f, 0.0f), nsMath::SmallEpsilon<nsMathTestType>());
  }
}
