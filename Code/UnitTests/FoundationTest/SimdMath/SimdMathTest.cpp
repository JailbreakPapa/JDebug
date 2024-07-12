#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/SimdMath/SimdMath.h>

namespace
{
  nsSimdVec4f SimdDegree(float fDegree)
  {
    return nsSimdVec4f(nsAngle::MakeFromDegree(fDegree));
  }
} // namespace

NS_CREATE_SIMPLE_TEST(SimdMath, SimdMath)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Exp")
  {
    float testVals[] = {0.0f, 1.0f, 2.0f};
    for (nsUInt32 i = 0; i < NS_ARRAY_SIZE(testVals); ++i)
    {
      const float v = testVals[i];
      const float r = nsMath::Exp(v);
      NS_TEST_BOOL(nsSimdMath::Exp(nsSimdVec4f(v)).IsEqual(nsSimdVec4f(r), 0.000001f).AllSet());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Ln")
  {
    float testVals[] = {1.0f, 2.7182818284f, 7.3890560989f};
    for (nsUInt32 i = 0; i < NS_ARRAY_SIZE(testVals); ++i)
    {
      const float v = testVals[i];
      const float r = nsMath::Ln(v);
      NS_TEST_BOOL(nsSimdMath::Ln(nsSimdVec4f(v)).IsEqual(nsSimdVec4f(r), 0.000001f).AllSet());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Log2")
  {
    float testVals[] = {1.0f, 2.0f, 4.0f};
    for (nsUInt32 i = 0; i < NS_ARRAY_SIZE(testVals); ++i)
    {
      const float v = testVals[i];
      const float r = nsMath::Log2(v);
      NS_TEST_BOOL(nsSimdMath::Log2(nsSimdVec4f(v)).IsEqual(nsSimdVec4f(r), 0.000001f).AllSet());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Log2i")
  {
    int testVals[] = {0, 1, 2, 3, 4, 6, 7, 8};
    for (nsUInt32 i = 0; i < NS_ARRAY_SIZE(testVals); ++i)
    {
      const int v = testVals[i];
      const int r = nsMath::Log2i(v);
      NS_TEST_BOOL((nsSimdMath::Log2i(nsSimdVec4i(v)) == nsSimdVec4i(r)).AllSet());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Log10")
  {
    float testVals[] = {1.0f, 10.0f, 100.0f};
    for (nsUInt32 i = 0; i < NS_ARRAY_SIZE(testVals); ++i)
    {
      const float v = testVals[i];
      const float r = nsMath::Log10(v);
      NS_TEST_BOOL(nsSimdMath::Log10(nsSimdVec4f(v)).IsEqual(nsSimdVec4f(r), 0.000001f).AllSet());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Pow2")
  {
    float testVals[] = {0.0f, 1.0f, 2.0f};
    for (nsUInt32 i = 0; i < NS_ARRAY_SIZE(testVals); ++i)
    {
      const float v = testVals[i];
      const float r = nsMath::Pow2(v);
      NS_TEST_BOOL(nsSimdMath::Pow2(nsSimdVec4f(v)).IsEqual(nsSimdVec4f(r), 0.000001f).AllSet());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Sin")
  {
    NS_TEST_BOOL(nsSimdMath::Sin(SimdDegree(0.0f)).IsEqual(nsSimdVec4f(0.0f), 0.000001f).AllSet());
    NS_TEST_BOOL(nsSimdMath::Sin(SimdDegree(90.0f)).IsEqual(nsSimdVec4f(1.0f), 0.000001f).AllSet());
    NS_TEST_BOOL(nsSimdMath::Sin(SimdDegree(180.0f)).IsEqual(nsSimdVec4f(0.0f), 0.000001f).AllSet());
    NS_TEST_BOOL(nsSimdMath::Sin(SimdDegree(270.0f)).IsEqual(nsSimdVec4f(-1.0f), 0.000001f).AllSet());

    NS_TEST_BOOL(nsSimdMath::Sin(SimdDegree(45.0f)).IsEqual(nsSimdVec4f(0.7071067f), 0.000001f).AllSet());
    NS_TEST_BOOL(nsSimdMath::Sin(SimdDegree(135.0f)).IsEqual(nsSimdVec4f(0.7071067f), 0.000001f).AllSet());
    NS_TEST_BOOL(nsSimdMath::Sin(SimdDegree(225.0f)).IsEqual(nsSimdVec4f(-0.7071067f), 0.000001f).AllSet());
    NS_TEST_BOOL(nsSimdMath::Sin(SimdDegree(315.0f)).IsEqual(nsSimdVec4f(-0.7071067f), 0.000001f).AllSet());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Cos")
  {
    NS_TEST_BOOL(nsSimdMath::Cos(SimdDegree(0.0f)).IsEqual(nsSimdVec4f(1.0f), 0.000001f).AllSet());
    NS_TEST_BOOL(nsSimdMath::Cos(SimdDegree(90.0f)).IsEqual(nsSimdVec4f(0.0f), 0.000001f).AllSet());
    NS_TEST_BOOL(nsSimdMath::Cos(SimdDegree(180.0f)).IsEqual(nsSimdVec4f(-1.0f), 0.000001f).AllSet());
    NS_TEST_BOOL(nsSimdMath::Cos(SimdDegree(270.0f)).IsEqual(nsSimdVec4f(0.0f), 0.000001f).AllSet());

    NS_TEST_BOOL(nsSimdMath::Cos(SimdDegree(45.0f)).IsEqual(nsSimdVec4f(0.7071067f), 0.000001f).AllSet());
    NS_TEST_BOOL(nsSimdMath::Cos(SimdDegree(135.0f)).IsEqual(nsSimdVec4f(-0.7071067f), 0.000001f).AllSet());
    NS_TEST_BOOL(nsSimdMath::Cos(SimdDegree(225.0f)).IsEqual(nsSimdVec4f(-0.7071067f), 0.000001f).AllSet());
    NS_TEST_BOOL(nsSimdMath::Cos(SimdDegree(315.0f)).IsEqual(nsSimdVec4f(0.7071067f), 0.000001f).AllSet());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Tan")
  {
    NS_TEST_BOOL(nsSimdMath::Tan(SimdDegree(0.0f)).IsEqual(nsSimdVec4f(0.0f), 0.000001f).AllSet());
    NS_TEST_BOOL(nsSimdMath::Tan(SimdDegree(45.0f)).IsEqual(nsSimdVec4f(1.0f), 0.000001f).AllSet());
    NS_TEST_BOOL(nsSimdMath::Tan(SimdDegree(-45.0f)).IsEqual(nsSimdVec4f(-1.0f), 0.000001f).AllSet());
    NS_TEST_BOOL((nsSimdMath::Tan(SimdDegree(90.00001f)) < nsSimdVec4f(1000000.0f)).AllSet());
    NS_TEST_BOOL((nsSimdMath::Tan(SimdDegree(89.9999f)) > nsSimdVec4f(100000.0f)).AllSet());

    // Testing the period of tan(x) centered at 0 and the adjacent ones
    nsAngle angle = nsAngle::MakeFromDegree(-89.0f);
    while (angle.GetDegree() < 89.0f)
    {
      nsSimdVec4f simdAngle(angle.GetRadian());

      nsSimdVec4f fTan = nsSimdMath::Tan(simdAngle);
      nsSimdVec4f fTanPrev = nsSimdMath::Tan(SimdDegree(angle.GetDegree() - 180.0f));
      nsSimdVec4f fTanNext = nsSimdMath::Tan(SimdDegree(angle.GetDegree() + 180.0f));
      nsSimdVec4f fSin = nsSimdMath::Sin(simdAngle);
      nsSimdVec4f fCos = nsSimdMath::Cos(simdAngle);

      NS_TEST_BOOL((fTan - fTanPrev).IsEqual(nsSimdVec4f::MakeZero(), 0.002f).AllSet());
      NS_TEST_BOOL((fTan - fTanNext).IsEqual(nsSimdVec4f::MakeZero(), 0.002f).AllSet());
      NS_TEST_BOOL((fTan - fSin.CompDiv(fCos)).IsEqual(nsSimdVec4f::MakeZero(), 0.0005f).AllSet());
      angle += nsAngle::MakeFromDegree(1.234f);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ASin")
  {
    NS_TEST_BOOL(nsSimdMath::ASin(nsSimdVec4f(0.0f)).IsEqual(SimdDegree(0.0f), 0.0001f).AllSet());
    NS_TEST_BOOL(nsSimdMath::ASin(nsSimdVec4f(1.0f)).IsEqual(SimdDegree(90.0f), 0.00001f).AllSet());
    NS_TEST_BOOL(nsSimdMath::ASin(nsSimdVec4f(-1.0f)).IsEqual(SimdDegree(-90.0f), 0.00001f).AllSet());

    NS_TEST_BOOL(nsSimdMath::ASin(nsSimdVec4f(0.7071067f)).IsEqual(SimdDegree(45.0f), 0.0001f).AllSet());
    NS_TEST_BOOL(nsSimdMath::ASin(nsSimdVec4f(-0.7071067f)).IsEqual(SimdDegree(-45.0f), 0.0001f).AllSet());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ACos")
  {
    NS_TEST_BOOL(nsSimdMath::ACos(nsSimdVec4f(0.0f)).IsEqual(SimdDegree(90.0f), 0.0001f).AllSet());
    NS_TEST_BOOL(nsSimdMath::ACos(nsSimdVec4f(1.0f)).IsEqual(SimdDegree(0.0f), 0.00001f).AllSet());
    NS_TEST_BOOL(nsSimdMath::ACos(nsSimdVec4f(-1.0f)).IsEqual(SimdDegree(180.0f), 0.0001f).AllSet());

    NS_TEST_BOOL(nsSimdMath::ACos(nsSimdVec4f(0.7071067f)).IsEqual(SimdDegree(45.0f), 0.0001f).AllSet());
    NS_TEST_BOOL(nsSimdMath::ACos(nsSimdVec4f(-0.7071067f)).IsEqual(SimdDegree(135.0f), 0.0001f).AllSet());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ATan")
  {
    NS_TEST_BOOL(nsSimdMath::ATan(nsSimdVec4f(0.0f)).IsEqual(SimdDegree(0.0f), 0.0000001f).AllSet());
    NS_TEST_BOOL(nsSimdMath::ATan(nsSimdVec4f(1.0f)).IsEqual(SimdDegree(45.0f), 0.00001f).AllSet());
    NS_TEST_BOOL(nsSimdMath::ATan(nsSimdVec4f(-1.0f)).IsEqual(SimdDegree(-45.0f), 0.00001f).AllSet());
    NS_TEST_BOOL(nsSimdMath::ATan(nsSimdVec4f(10000000.0f)).IsEqual(SimdDegree(90.0f), 0.00002f).AllSet());
    NS_TEST_BOOL(nsSimdMath::ATan(nsSimdVec4f(-10000000.0f)).IsEqual(SimdDegree(-90.0f), 0.00002f).AllSet());
  }
}
