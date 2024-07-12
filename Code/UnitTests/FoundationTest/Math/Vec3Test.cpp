#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Math/Random.h>
#include <Foundation/Math/Vec2.h>
#include <Foundation/Math/Vec3.h>
#include <Foundation/Math/Vec4.h>


NS_CREATE_SIMPLE_TEST(Math, Vec3)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor")
  {
#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
    if (nsMath::SupportsNaN<nsVec3T::ComponentType>())
    {
      // In debug the default constructor initializes everything with NaN.
      nsVec3T vDefCtor;
      NS_TEST_BOOL(nsMath::IsNaN(vDefCtor.x) && nsMath::IsNaN(vDefCtor.y) && nsMath::IsNaN(vDefCtor.z));
    }
#else
    // Placement new of the default constructor should not have any effect on the previous data.
    nsVec3T::ComponentType testBlock[3] = {(nsVec3T::ComponentType)1, (nsVec3T::ComponentType)2, (nsVec3T::ComponentType)3};
    nsVec3T* pDefCtor = ::new ((void*)&testBlock[0]) nsVec3T;
    NS_TEST_BOOL(pDefCtor->x == (nsVec3T::ComponentType)1 && pDefCtor->y == (nsVec3T::ComponentType)2 && pDefCtor->z == (nsVec3T::ComponentType)3.);
#endif

    // Make sure the class didn't accidentally change in size.
    NS_TEST_BOOL(sizeof(nsVec3) == 12);
    NS_TEST_BOOL(sizeof(nsVec3d) == 24);

    nsVec3T vInit1F(2.0f);
    NS_TEST_BOOL(vInit1F.x == 2.0f && vInit1F.y == 2.0f && vInit1F.z == 2.0f);

    nsVec3T vInit4F(1.0f, 2.0f, 3.0f);
    NS_TEST_BOOL(vInit4F.x == 1.0f && vInit4F.y == 2.0f && vInit4F.z == 3.0f);

    nsVec3T vCopy(vInit4F);
    NS_TEST_BOOL(vCopy.x == 1.0f && vCopy.y == 2.0f && vCopy.z == 3.0f);

    nsVec3T vZero = nsVec3T::MakeZero();
    NS_TEST_BOOL(vZero.x == 0.0f && vZero.y == 0.0f && vZero.z == 0.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Conversion")
  {
    nsVec3T vData(1.0f, 2.0f, 3.0f);
    nsVec2T vToVec2 = vData.GetAsVec2();
    NS_TEST_BOOL(vToVec2.x == vData.x && vToVec2.y == vData.y);

    nsVec4T vToVec4 = vData.GetAsVec4(42.0f);
    NS_TEST_BOOL(vToVec4.x == vData.x && vToVec4.y == vData.y && vToVec4.z == vData.z && vToVec4.w == 42.0f);

    nsVec4T vToVec4Pos = vData.GetAsPositionVec4();
    NS_TEST_BOOL(vToVec4Pos.x == vData.x && vToVec4Pos.y == vData.y && vToVec4Pos.z == vData.z && vToVec4Pos.w == 1.0f);

    nsVec4T vToVec4Dir = vData.GetAsDirectionVec4();
    NS_TEST_BOOL(vToVec4Dir.x == vData.x && vToVec4Dir.y == vData.y && vToVec4Dir.z == vData.z && vToVec4Dir.w == 0.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Setter")
  {
    nsVec3T vSet1F;
    vSet1F.Set(2.0f);
    NS_TEST_BOOL(vSet1F.x == 2.0f && vSet1F.y == 2.0f && vSet1F.z == 2.0f);

    nsVec3T vSet4F;
    vSet4F.Set(1.0f, 2.0f, 3.0f);
    NS_TEST_BOOL(vSet4F.x == 1.0f && vSet4F.y == 2.0f && vSet4F.z == 3.0f);

    nsVec3T vSetZero;
    vSetZero.SetZero();
    NS_TEST_BOOL(vSetZero.x == 0.0f && vSetZero.y == 0.0f && vSetZero.z == 0.0f);
  }


  {
    const nsVec3T vOp1(-4.0, 4.0f, -2.0f);
    const nsVec3T compArray[3] = {nsVec3T(1.0f, 0.0f, 0.0f), nsVec3T(0.0f, 1.0f, 0.0f), nsVec3T(0.0f, 0.0f, 1.0f)};

    NS_TEST_BLOCK(nsTestBlock::Enabled, "GetLength")
    {
      NS_TEST_FLOAT(vOp1.GetLength(), 6.0f, nsMath::SmallEpsilon<nsMathTestType>());
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "SetLength")
    {
      nsVec3T vSetLength = vOp1.GetNormalized() * nsMath::DefaultEpsilon<nsMathTestType>();
      NS_TEST_BOOL(vSetLength.SetLength(4.0f, nsMath::LargeEpsilon<nsMathTestType>()) == NS_FAILURE);
      NS_TEST_BOOL(vSetLength == nsVec3T::MakeZero());
      vSetLength = vOp1.GetNormalized() * (nsMathTestType)0.001;
      NS_TEST_BOOL(vSetLength.SetLength(4.0f, (nsMathTestType)nsMath::DefaultEpsilon<nsMathTestType>()) == NS_SUCCESS);
      NS_TEST_FLOAT(vSetLength.GetLength(), 4.0f, nsMath::SmallEpsilon<nsMathTestType>());
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "GetLengthSquared")
    {
      NS_TEST_FLOAT(vOp1.GetLengthSquared(), 36.0f, nsMath::SmallEpsilon<nsMathTestType>());
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "GetLengthAndNormalize")
    {
      nsVec3T vLengthAndNorm = vOp1;
      nsMathTestType fLength = vLengthAndNorm.GetLengthAndNormalize();
      NS_TEST_FLOAT(vLengthAndNorm.GetLength(), 1.0f, nsMath::SmallEpsilon<nsMathTestType>());
      NS_TEST_FLOAT(fLength, 6.0f, nsMath::SmallEpsilon<nsMathTestType>());
      NS_TEST_FLOAT(vLengthAndNorm.x * vLengthAndNorm.x + vLengthAndNorm.y * vLengthAndNorm.y + vLengthAndNorm.z * vLengthAndNorm.z, 1.0f,
        nsMath::SmallEpsilon<nsMathTestType>());
      NS_TEST_BOOL(vLengthAndNorm.IsNormalized(nsMath::SmallEpsilon<nsMathTestType>()));
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "GetNormalized")
    {
      nsVec3T vGetNorm = vOp1.GetNormalized();
      NS_TEST_FLOAT(vGetNorm.x * vGetNorm.x + vGetNorm.y * vGetNorm.y + vGetNorm.z * vGetNorm.z, 1.0f, nsMath::SmallEpsilon<nsMathTestType>());
      NS_TEST_BOOL(vGetNorm.IsNormalized(nsMath::SmallEpsilon<nsMathTestType>()));
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "Normalize")
    {
      nsVec3T vNorm = vOp1;
      vNorm.Normalize();
      NS_TEST_FLOAT(vNorm.x * vNorm.x + vNorm.y * vNorm.y + vNorm.z * vNorm.z, 1.0f, nsMath::SmallEpsilon<nsMathTestType>());
      NS_TEST_BOOL(vNorm.IsNormalized(nsMath::SmallEpsilon<nsMathTestType>()));
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "NormalizeIfNotZero")
    {
      nsVec3T vNorm = vOp1;
      vNorm.Normalize();

      nsVec3T vNormCond = vNorm * nsMath::DefaultEpsilon<nsMathTestType>();
      NS_TEST_BOOL(vNormCond.NormalizeIfNotZero(vOp1, nsMath::LargeEpsilon<nsMathTestType>()) == NS_FAILURE);
      NS_TEST_BOOL(vNormCond == vOp1);
      vNormCond = vNorm * nsMath::DefaultEpsilon<nsMathTestType>();
      NS_TEST_BOOL(vNormCond.NormalizeIfNotZero(vOp1, nsMath::SmallEpsilon<nsMathTestType>()) == NS_SUCCESS);
      NS_TEST_VEC3(vNormCond, vNorm, nsMath::DefaultEpsilon<nsVec3T::ComponentType>());
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "IsZero")
    {
      NS_TEST_BOOL(nsVec3T::MakeZero().IsZero());
      for (int i = 0; i < 3; ++i)
      {
        NS_TEST_BOOL(!compArray[i].IsZero());
      }
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "IsZero(float)")
    {
      NS_TEST_BOOL(nsVec3T::MakeZero().IsZero(0.0f));
      for (int i = 0; i < 3; ++i)
      {
        NS_TEST_BOOL(!compArray[i].IsZero(0.0f));
        NS_TEST_BOOL(compArray[i].IsZero(1.0f));
        NS_TEST_BOOL((-compArray[i]).IsZero(1.0f));
      }
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "IsNormalized (2)")
    {
      for (int i = 0; i < 3; ++i)
      {
        NS_TEST_BOOL(compArray[i].IsNormalized());
        NS_TEST_BOOL((-compArray[i]).IsNormalized());
        NS_TEST_BOOL((compArray[i] * (nsMathTestType)2).IsNormalized((nsMathTestType)4));
        NS_TEST_BOOL((compArray[i] * (nsMathTestType)2).IsNormalized((nsMathTestType)4));
      }
    }

    if (nsMath::SupportsNaN<nsMathTestType>())
    {
      nsMathTestType fNaN = nsMath::NaN<nsMathTestType>();
      const nsVec3T nanArray[3] = {nsVec3T(fNaN, 0.0f, 0.0f), nsVec3T(0.0f, fNaN, 0.0f), nsVec3T(0.0f, 0.0f, fNaN)};

      NS_TEST_BLOCK(nsTestBlock::Enabled, "IsNaN")
      {
        for (int i = 0; i < 3; ++i)
        {
          NS_TEST_BOOL(nanArray[i].IsNaN());
          NS_TEST_BOOL(!compArray[i].IsNaN());
        }
      }

      NS_TEST_BLOCK(nsTestBlock::Enabled, "IsValid")
      {
        for (int i = 0; i < 3; ++i)
        {
          NS_TEST_BOOL(!nanArray[i].IsValid());
          NS_TEST_BOOL(compArray[i].IsValid());

          NS_TEST_BOOL(!(compArray[i] * nsMath::Infinity<nsMathTestType>()).IsValid());
          NS_TEST_BOOL(!(compArray[i] * -nsMath::Infinity<nsMathTestType>()).IsValid());
        }
      }
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Operators")
  {
    const nsVec3T vOp1(-4.0, 0.2f, -7.0f);
    const nsVec3T vOp2(2.0, 0.3f, 0.0f);
    const nsVec3T compArray[3] = {nsVec3T(1.0f, 0.0f, 0.0f), nsVec3T(0.0f, 1.0f, 0.0f), nsVec3T(0.0f, 0.0f, 1.0f)};
    // IsIdentical
    NS_TEST_BOOL(vOp1.IsIdentical(vOp1));
    for (int i = 0; i < 3; ++i)
    {
      NS_TEST_BOOL(!vOp1.IsIdentical(vOp1 + (nsMathTestType)nsMath::SmallEpsilon<nsMathTestType>() * compArray[i]));
      NS_TEST_BOOL(!vOp1.IsIdentical(vOp1 - (nsMathTestType)nsMath::SmallEpsilon<nsMathTestType>() * compArray[i]));
    }

    // IsEqual
    NS_TEST_BOOL(vOp1.IsEqual(vOp1, 0.0f));
    for (int i = 0; i < 3; ++i)
    {
      NS_TEST_BOOL(vOp1.IsEqual(vOp1 + nsMath::SmallEpsilon<nsMathTestType>() * compArray[i], 2 * nsMath::SmallEpsilon<nsMathTestType>()));
      NS_TEST_BOOL(vOp1.IsEqual(vOp1 - nsMath::SmallEpsilon<nsMathTestType>() * compArray[i], 2 * nsMath::SmallEpsilon<nsMathTestType>()));
      NS_TEST_BOOL(vOp1.IsEqual(vOp1 + nsMath::DefaultEpsilon<nsMathTestType>() * compArray[i], 2 * nsMath::DefaultEpsilon<nsMathTestType>()));
      NS_TEST_BOOL(vOp1.IsEqual(vOp1 - nsMath::DefaultEpsilon<nsMathTestType>() * compArray[i], 2 * nsMath::DefaultEpsilon<nsMathTestType>()));
    }

    // operator-
    nsVec3T vNegated = -vOp1;
    NS_TEST_BOOL(vOp1.x == -vNegated.x && vOp1.y == -vNegated.y && vOp1.z == -vNegated.z);

    // operator+= (nsVec3T)
    nsVec3T vPlusAssign = vOp1;
    vPlusAssign += vOp2;
    NS_TEST_BOOL(vPlusAssign.IsEqual(nsVec3T(-2.0f, 0.5f, -7.0f), nsMath::SmallEpsilon<nsMathTestType>()));

    // operator-= (nsVec3T)
    nsVec3T vMinusAssign = vOp1;
    vMinusAssign -= vOp2;
    NS_TEST_BOOL(vMinusAssign.IsEqual(nsVec3T(-6.0f, -0.1f, -7.0f), nsMath::SmallEpsilon<nsMathTestType>()));

    // operator*= (float)
    nsVec3T vMulFloat = vOp1;
    vMulFloat *= 2.0f;
    NS_TEST_BOOL(vMulFloat.IsEqual(nsVec3T(-8.0f, 0.4f, -14.0f), nsMath::SmallEpsilon<nsMathTestType>()));
    vMulFloat *= 0.0f;
    NS_TEST_BOOL(vMulFloat.IsEqual(nsVec3T::MakeZero(), nsMath::SmallEpsilon<nsMathTestType>()));

    // operator/= (float)
    nsVec3T vDivFloat = vOp1;
    vDivFloat /= 2.0f;
    NS_TEST_BOOL(vDivFloat.IsEqual(nsVec3T(-2.0f, 0.1f, -3.5f), nsMath::SmallEpsilon<nsMathTestType>()));

    // operator+ (nsVec3T, nsVec3T)
    nsVec3T vPlus = (vOp1 + vOp2);
    NS_TEST_BOOL(vPlus.IsEqual(nsVec3T(-2.0f, 0.5f, -7.0f), nsMath::SmallEpsilon<nsMathTestType>()));

    // operator- (nsVec3T, nsVec3T)
    nsVec3T vMinus = (vOp1 - vOp2);
    NS_TEST_BOOL(vMinus.IsEqual(nsVec3T(-6.0f, -0.1f, -7.0f), nsMath::SmallEpsilon<nsMathTestType>()));

    // operator* (float, nsVec3T)
    nsVec3T vMulFloatVec3 = ((nsMathTestType)2 * vOp1);
    NS_TEST_BOOL(
      vMulFloatVec3.IsEqual(nsVec3T((nsMathTestType)-8.0, (nsMathTestType)0.4, (nsMathTestType)-14.0), nsMath::SmallEpsilon<nsMathTestType>()));
    vMulFloatVec3 = ((nsMathTestType)0 * vOp1);
    NS_TEST_BOOL(vMulFloatVec3.IsEqual(nsVec3T::MakeZero(), nsMath::SmallEpsilon<nsMathTestType>()));

    // operator* (nsVec3T, float)
    nsVec3T vMulVec3Float = (vOp1 * (nsMathTestType)2);
    NS_TEST_BOOL(vMulVec3Float.IsEqual(nsVec3T(-8.0f, 0.4f, -14.0f), nsMath::SmallEpsilon<nsMathTestType>()));
    vMulVec3Float = (vOp1 * (nsMathTestType)0);
    NS_TEST_BOOL(vMulVec3Float.IsEqual(nsVec3T::MakeZero(), nsMath::SmallEpsilon<nsMathTestType>()));

    // operator/ (nsVec3T, float)
    nsVec3T vDivVec3Float = (vOp1 / (nsMathTestType)2);
    NS_TEST_BOOL(vDivVec3Float.IsEqual(nsVec3T(-2.0f, 0.1f, -3.5f), nsMath::SmallEpsilon<nsMathTestType>()));

    // operator== (nsVec3T, nsVec3T)
    NS_TEST_BOOL(vOp1 == vOp1);
    for (int i = 0; i < 3; ++i)
    {
      NS_TEST_BOOL(!(vOp1 == (vOp1 + (nsMathTestType)nsMath::SmallEpsilon<nsMathTestType>() * compArray[i])));
      NS_TEST_BOOL(!(vOp1 == (vOp1 - (nsMathTestType)nsMath::SmallEpsilon<nsMathTestType>() * compArray[i])));
    }

    // operator!= (nsVec3T, nsVec3T)
    NS_TEST_BOOL(!(vOp1 != vOp1));
    for (int i = 0; i < 3; ++i)
    {
      NS_TEST_BOOL(vOp1 != (vOp1 + (nsMathTestType)nsMath::SmallEpsilon<nsMathTestType>() * compArray[i]));
      NS_TEST_BOOL(vOp1 != (vOp1 - (nsMathTestType)nsMath::SmallEpsilon<nsMathTestType>() * compArray[i]));
    }

    // operator< (nsVec3T, nsVec3T)
    for (int i = 0; i < 3; ++i)
    {
      for (int j = 0; j < 3; ++j)
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
    const nsVec3T vOp1(-4.0, 0.2f, -7.0f);
    const nsVec3T vOp2(2.0, -0.3f, 0.5f);

    const nsVec3T compArray[3] = {nsVec3T(1.0f, 0.0f, 0.0f), nsVec3T(0.0f, 1.0f, 0.0f), nsVec3T(0.0f, 0.0f, 1.0f)};

    // GetAngleBetween
    for (int i = 0; i < 3; ++i)
    {
      for (int j = 0; j < 3; ++j)
      {
        NS_TEST_FLOAT(compArray[i].GetAngleBetween(compArray[j]).GetDegree(), i == j ? 0.0f : 90.0f, 0.00001f);
      }
    }

    // Dot
    for (int i = 0; i < 3; ++i)
    {
      for (int j = 0; j < 3; ++j)
      {
        NS_TEST_FLOAT(compArray[i].Dot(compArray[j]), i == j ? 1.0f : 0.0f, nsMath::SmallEpsilon<nsMathTestType>());
      }
    }
    NS_TEST_FLOAT(vOp1.Dot(vOp2), -11.56f, nsMath::SmallEpsilon<nsMathTestType>());
    NS_TEST_FLOAT(vOp2.Dot(vOp1), -11.56f, nsMath::SmallEpsilon<nsMathTestType>());

    // Cross
    // Right-handed coordinate system check
    NS_TEST_BOOL(compArray[0].CrossRH(compArray[1]).IsEqual(compArray[2], nsMath::SmallEpsilon<nsMathTestType>()));
    NS_TEST_BOOL(compArray[1].CrossRH(compArray[2]).IsEqual(compArray[0], nsMath::SmallEpsilon<nsMathTestType>()));
    NS_TEST_BOOL(compArray[2].CrossRH(compArray[0]).IsEqual(compArray[1], nsMath::SmallEpsilon<nsMathTestType>()));

    // CompMin
    NS_TEST_BOOL(vOp1.CompMin(vOp2).IsEqual(nsVec3T(-4.0f, -0.3f, -7.0f), nsMath::SmallEpsilon<nsMathTestType>()));
    NS_TEST_BOOL(vOp2.CompMin(vOp1).IsEqual(nsVec3T(-4.0f, -0.3f, -7.0f), nsMath::SmallEpsilon<nsMathTestType>()));

    // CompMax
    NS_TEST_BOOL(vOp1.CompMax(vOp2).IsEqual(nsVec3T(2.0f, 0.2f, 0.5f), nsMath::SmallEpsilon<nsMathTestType>()));
    NS_TEST_BOOL(vOp2.CompMax(vOp1).IsEqual(nsVec3T(2.0f, 0.2f, 0.5f), nsMath::SmallEpsilon<nsMathTestType>()));

    // CompClamp
    NS_TEST_BOOL(vOp1.CompClamp(vOp1, vOp2).IsEqual(nsVec3T(-4.0f, -0.3f, -7.0f), nsMath::SmallEpsilon<nsMathTestType>()));
    NS_TEST_BOOL(vOp2.CompClamp(vOp1, vOp2).IsEqual(nsVec3T(2.0f, 0.2f, 0.5f), nsMath::SmallEpsilon<nsMathTestType>()));

    // CompMul
    NS_TEST_BOOL(vOp1.CompMul(vOp2).IsEqual(nsVec3T(-8.0f, -0.06f, -3.5f), nsMath::SmallEpsilon<nsMathTestType>()));
    NS_TEST_BOOL(vOp2.CompMul(vOp1).IsEqual(nsVec3T(-8.0f, -0.06f, -3.5f), nsMath::SmallEpsilon<nsMathTestType>()));

    // CompDiv
    NS_TEST_BOOL(vOp1.CompDiv(vOp2).IsEqual(nsVec3T(-2.0f, -0.66666666f, -14.0f), nsMath::SmallEpsilon<nsMathTestType>()));

    // Abs
    NS_TEST_VEC3(vOp1.Abs(), nsVec3T(4.0, 0.2f, 7.0f), nsMath::SmallEpsilon<nsMathTestType>());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CalculateNormal")
  {
    nsVec3T n;
    NS_TEST_BOOL(n.CalculateNormal(nsVec3T(-1, 0, 1), nsVec3T(1, 0, 1), nsVec3T(0, 0, -1)) == NS_SUCCESS);
    NS_TEST_VEC3(n, nsVec3T(0, 1, 0), 0.001f);

    NS_TEST_BOOL(n.CalculateNormal(nsVec3T(-1, 0, -1), nsVec3T(1, 0, -1), nsVec3T(0, 0, 1)) == NS_SUCCESS);
    NS_TEST_VEC3(n, nsVec3T(0, -1, 0), 0.001f);

    NS_TEST_BOOL(n.CalculateNormal(nsVec3T(-1, 0, 1), nsVec3T(1, 0, 1), nsVec3T(1, 0, 1)) == NS_FAILURE);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeOrthogonalTo")
  {
    nsVec3T v;

    v.Set(1, 1, 0);
    v.MakeOrthogonalTo(nsVec3T(1, 0, 0));
    NS_TEST_VEC3(v, nsVec3T(0, 1, 0), 0.001f);

    v.Set(1, 1, 0);
    v.MakeOrthogonalTo(nsVec3T(0, 1, 0));
    NS_TEST_VEC3(v, nsVec3T(1, 0, 0), 0.001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetOrthogonalVector")
  {
    nsVec3T v;

    for (float i = 1; i < 360; i += 3.0f)
    {
      v.Set(i, i * 3, i * 7);
      NS_TEST_FLOAT(v.GetOrthogonalVector().Dot(v), 0.0f, 0.001f);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetReflectedVector")
  {
    nsVec3T v, v2;

    v.Set(1, 1, 0);
    v2 = v.GetReflectedVector(nsVec3T(0, -1, 0));
    NS_TEST_VEC3(v2, nsVec3T(1, -1, 0), 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeRandomPointInSphere")
  {
    nsVec3T v;

    nsRandom rng;
    rng.Initialize(0xEEFF0011AABBCCDDULL);

    nsVec3T avg;
    avg.SetZero();

    const nsUInt32 uiNumSamples = 100'000;
    for (nsUInt32 i = 0; i < uiNumSamples; ++i)
    {
      v = nsVec3T::MakeRandomPointInSphere(rng);

      NS_TEST_BOOL(v.GetLength() <= 1.0f + nsMath::SmallEpsilon<float>());
      NS_TEST_BOOL(!v.IsZero());

      avg += v;
    }

    avg /= (float)uiNumSamples;

    // the average point cloud center should be within at least 10% of the sphere's center
    // otherwise the points aren't equally distributed
    NS_TEST_BOOL(avg.IsZero(0.1f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeRandomDirection")
  {
    nsVec3T v;

    nsRandom rng;
    rng.InitializeFromCurrentTime();

    nsVec3T avg;
    avg.SetZero();

    const nsUInt32 uiNumSamples = 100'000;
    for (nsUInt32 i = 0; i < uiNumSamples; ++i)
    {
      v = nsVec3T::MakeRandomDirection(rng);

      NS_TEST_BOOL(v.IsNormalized());

      avg += v;
    }

    avg /= (float)uiNumSamples;

    // the average point cloud center should be within at least 10% of the sphere's center
    // otherwise the points aren't equally distributed
    NS_TEST_BOOL(avg.IsZero(0.1f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeRandomDeviationX")
  {
    nsVec3T v;
    nsVec3T avg;
    avg.SetZero();

    nsRandom rng;
    rng.InitializeFromCurrentTime();

    const nsAngle dev = nsAngle::MakeFromDegree(65);
    const nsUInt32 uiNumSamples = 100'000;
    const nsVec3 vAxis(1, 0, 0);

    for (nsUInt32 i = 0; i < uiNumSamples; ++i)
    {
      v = nsVec3T::MakeRandomDeviationX(rng, dev);

      NS_TEST_BOOL(v.IsNormalized());

      NS_TEST_BOOL(vAxis.GetAngleBetween(v).GetRadian() <= dev.GetRadian() + nsMath::DefaultEpsilon<float>());

      avg += v;
    }

    // average direction should be close to the main axis
    avg.Normalize();
    NS_TEST_BOOL(avg.IsEqual(vAxis, 0.1f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeRandomDeviationY")
  {
    nsVec3T v;
    nsVec3T avg;
    avg.SetZero();

    nsRandom rng;
    rng.InitializeFromCurrentTime();

    const nsAngle dev = nsAngle::MakeFromDegree(65);
    const nsUInt32 uiNumSamples = 100'000;
    const nsVec3 vAxis(0, 1, 0);

    for (nsUInt32 i = 0; i < uiNumSamples; ++i)
    {
      v = nsVec3T::MakeRandomDeviationY(rng, dev);

      NS_TEST_BOOL(v.IsNormalized());

      NS_TEST_BOOL(vAxis.GetAngleBetween(v).GetRadian() <= dev.GetRadian() + nsMath::DefaultEpsilon<float>());

      avg += v;
    }

    // average direction should be close to the main axis
    avg.Normalize();
    NS_TEST_BOOL(avg.IsEqual(vAxis, 0.1f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeRandomDeviationZ")
  {
    nsVec3T v;
    nsVec3T avg;
    avg.SetZero();

    nsRandom rng;
    rng.InitializeFromCurrentTime();

    const nsAngle dev = nsAngle::MakeFromDegree(65);
    const nsUInt32 uiNumSamples = 100'000;
    const nsVec3 vAxis(0, 0, 1);

    for (nsUInt32 i = 0; i < uiNumSamples; ++i)
    {
      v = nsVec3T::MakeRandomDeviationZ(rng, dev);

      NS_TEST_BOOL(v.IsNormalized());

      NS_TEST_BOOL(vAxis.GetAngleBetween(v).GetRadian() <= dev.GetRadian() + nsMath::DefaultEpsilon<float>());

      avg += v;
    }

    // average direction should be close to the main axis
    avg.Normalize();
    NS_TEST_BOOL(avg.IsEqual(vAxis, 0.1f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeRandomDeviation")
  {
    nsVec3T v;

    nsRandom rng;
    rng.InitializeFromCurrentTime();

    const nsAngle dev = nsAngle::MakeFromDegree(65);
    const nsUInt32 uiNumSamples = 100'000;
    nsVec3 vAxis;

    for (nsUInt32 i = 0; i < uiNumSamples; ++i)
    {
      vAxis = nsVec3T::MakeRandomDirection(rng);

      v = nsVec3T::MakeRandomDeviation(rng, dev, vAxis);

      NS_TEST_BOOL(v.IsNormalized());

      NS_TEST_BOOL(vAxis.GetAngleBetween(v).GetDegree() <= dev.GetDegree() + 1.0f);
    }
  }
}
