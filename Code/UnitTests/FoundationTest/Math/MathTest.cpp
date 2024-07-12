#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Logging/Log.h>
#include <Foundation/Math/Math.h>
#include <Foundation/Math/Vec2.h>

/// ********************* Binary to Int conversion *********************
/// Most significant bit comes first.
/// Adapted from http://bytes.com/topic/c/answers/219656-literal-binary
///
/// Sample usage:
/// NS_8BIT(01010101) == 85
/// NS_16BIT(10101010, 01010101) == 43605
/// NS_32BIT(10000000, 11111111, 10101010, 01010101) == 2164238933
/// ********************************************************************
#define OCT__(n) 0##n##LU

#define NS_8BIT__(iBits)                                                                                                           \
  (((iBits & 000000001) ? 1 : 0) + ((iBits & 000000010) ? 2 : 0) + ((iBits & 000000100) ? 4 : 0) + ((iBits & 000001000) ? 8 : 0) + \
    ((iBits & 000010000) ? 16 : 0) + ((iBits & 000100000) ? 32 : 0) + ((iBits & 001000000) ? 64 : 0) + ((iBits & 010000000) ? 128 : 0))

#define NS_8BIT(B) ((nsUInt8)NS_8BIT__(OCT__(B)))

#define NS_16BIT(B2, B1) (((nsUInt8)NS_8BIT(B2) << 8) + NS_8BIT(B1))

#define NS_32BIT(B4, B3, B2, B1) \
  ((unsigned long)NS_8BIT(B4) << 24) + ((unsigned long)NS_8BIT(B3) << 16) + ((unsigned long)NS_8BIT(B2) << 8) + ((unsigned long)NS_8BIT(B1))

namespace
{
  struct UniqueInt
  {
    int i, id;
    UniqueInt(int i, int iId)
      : i(i)
      , id(iId)
    {
    }

    bool operator<(const UniqueInt& rh) { return this->i < rh.i; }

    bool operator>(const UniqueInt& rh) { return this->i > rh.i; }
  };
}; // namespace


NS_CREATE_SIMPLE_TEST_GROUP(Math);

NS_CREATE_SIMPLE_TEST(Math, General)
{
  // NS_TEST_BLOCK(nsTestBlock::Enabled, "Constants")
  //{
  //  // Macro test
  //  NS_TEST_BOOL(NS_8BIT(01010101) == 85);
  //  NS_TEST_BOOL(NS_16BIT(10101010, 01010101) == 43605);
  //  NS_TEST_BOOL(NS_32BIT(10000000, 11111111, 10101010, 01010101) == 2164238933);

  //  // Infinity test
  //  //                           Sign:_
  //  //                       Exponent: _______  _
  //  //                       Fraction:           _______  ________  ________
  //  nsIntFloatUnion uInf = { NS_32BIT(01111111, 10000000, 00000000, 00000000) };
  //  NS_TEST_BOOL(uInf.f == nsMath::FloatInfinity());

  //  // FloatMax_Pos test
  //  nsIntFloatUnion uMax = { NS_32BIT(01111111, 01111111, 11111111, 11111111) };
  //  NS_TEST_BOOL(uMax.f == nsMath::FloatMax_Pos());

  //  // FloatMax_Neg test
  //  nsIntFloatUnion uMin = { NS_32BIT(11111111, 01111111, 11111111, 11111111) };
  //  NS_TEST_BOOL(uMin.f == nsMath::FloatMax_Neg());
  //}

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Sin")
  {
    NS_TEST_FLOAT(nsMath::Sin(nsAngle::MakeFromDegree(0.0f)), 0.0f, 0.000001f);
    NS_TEST_FLOAT(nsMath::Sin(nsAngle::MakeFromDegree(90.0f)), 1.0f, 0.000001f);
    NS_TEST_FLOAT(nsMath::Sin(nsAngle::MakeFromDegree(180.0f)), 0.0f, 0.000001f);
    NS_TEST_FLOAT(nsMath::Sin(nsAngle::MakeFromDegree(270.0f)), -1.0f, 0.000001f);

    NS_TEST_FLOAT(nsMath::Sin(nsAngle::MakeFromDegree(45.0f)), 0.7071067f, 0.000001f);
    NS_TEST_FLOAT(nsMath::Sin(nsAngle::MakeFromDegree(135.0f)), 0.7071067f, 0.000001f);
    NS_TEST_FLOAT(nsMath::Sin(nsAngle::MakeFromDegree(225.0f)), -0.7071067f, 0.000001f);
    NS_TEST_FLOAT(nsMath::Sin(nsAngle::MakeFromDegree(315.0f)), -0.7071067f, 0.000001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Cos")
  {
    NS_TEST_FLOAT(nsMath::Cos(nsAngle::MakeFromDegree(0.0f)), 1.0f, 0.000001f);
    NS_TEST_FLOAT(nsMath::Cos(nsAngle::MakeFromDegree(90.0f)), 0.0f, 0.000001f);
    NS_TEST_FLOAT(nsMath::Cos(nsAngle::MakeFromDegree(180.0f)), -1.0f, 0.000001f);
    NS_TEST_FLOAT(nsMath::Cos(nsAngle::MakeFromDegree(270.0f)), 0.0f, 0.000001f);

    NS_TEST_FLOAT(nsMath::Cos(nsAngle::MakeFromDegree(45.0f)), 0.7071067f, 0.000001f);
    NS_TEST_FLOAT(nsMath::Cos(nsAngle::MakeFromDegree(135.0f)), -0.7071067f, 0.000001f);
    NS_TEST_FLOAT(nsMath::Cos(nsAngle::MakeFromDegree(225.0f)), -0.7071067f, 0.000001f);
    NS_TEST_FLOAT(nsMath::Cos(nsAngle::MakeFromDegree(315.0f)), 0.7071067f, 0.000001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Tan")
  {
    NS_TEST_FLOAT(nsMath::Tan(nsAngle::MakeFromDegree(0.0f)), 0.0f, 0.000001f);
    NS_TEST_FLOAT(nsMath::Tan(nsAngle::MakeFromDegree(45.0f)), 1.0f, 0.000001f);
    NS_TEST_FLOAT(nsMath::Tan(nsAngle::MakeFromDegree(-45.0f)), -1.0f, 0.000001f);
    NS_TEST_BOOL(nsMath::Tan(nsAngle::MakeFromDegree(90.00001f)) < 1000000.0f);
    NS_TEST_BOOL(nsMath::Tan(nsAngle::MakeFromDegree(89.9999f)) > 100000.0f);

    // Testing the period of tan(x) centered at 0 and the adjacent ones
    nsAngle angle = nsAngle::MakeFromDegree(-89.0f);
    while (angle.GetDegree() < 89.0f)
    {
      float fTan = nsMath::Tan(angle);
      float fTanPrev = nsMath::Tan(nsAngle::MakeFromDegree(angle.GetDegree() - 180.0f));
      float fTanNext = nsMath::Tan(nsAngle::MakeFromDegree(angle.GetDegree() + 180.0f));
      float fSin = nsMath::Sin(angle);
      float fCos = nsMath::Cos(angle);

      NS_TEST_FLOAT(fTan - fTanPrev, 0.0f, 0.002f);
      NS_TEST_FLOAT(fTan - fTanNext, 0.0f, 0.002f);
      NS_TEST_FLOAT(fTan - (fSin / fCos), 0.0f, 0.0005f);
      angle += nsAngle::MakeFromDegree(1.234f);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ASin")
  {
    NS_TEST_FLOAT(nsMath::ASin(0.0f).GetDegree(), 0.0f, 0.00001f);
    NS_TEST_FLOAT(nsMath::ASin(1.0f).GetDegree(), 90.0f, 0.00001f);
    NS_TEST_FLOAT(nsMath::ASin(-1.0f).GetDegree(), -90.0f, 0.00001f);

    NS_TEST_FLOAT(nsMath::ASin(0.7071067f).GetDegree(), 45.0f, 0.0001f);
    NS_TEST_FLOAT(nsMath::ASin(-0.7071067f).GetDegree(), -45.0f, 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ACos")
  {
    NS_TEST_FLOAT(nsMath::ACos(0.0f).GetDegree(), 90.0f, 0.00001f);
    NS_TEST_FLOAT(nsMath::ACos(1.0f).GetDegree(), 0.0f, 0.00001f);
    NS_TEST_FLOAT(nsMath::ACos(-1.0f).GetDegree(), 180.0f, 0.0001f);

    NS_TEST_FLOAT(nsMath::ACos(0.7071067f).GetDegree(), 45.0f, 0.0001f);
    NS_TEST_FLOAT(nsMath::ACos(-0.7071067f).GetDegree(), 135.0f, 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ATan")
  {
    NS_TEST_FLOAT(nsMath::ATan(0.0f).GetDegree(), 0.0f, 0.0000001f);
    NS_TEST_FLOAT(nsMath::ATan(1.0f).GetDegree(), 45.0f, 0.00001f);
    NS_TEST_FLOAT(nsMath::ATan(-1.0f).GetDegree(), -45.0f, 0.00001f);
    NS_TEST_FLOAT(nsMath::ATan(10000000.0f).GetDegree(), 90.0f, 0.00002f);
    NS_TEST_FLOAT(nsMath::ATan(-10000000.0f).GetDegree(), -90.0f, 0.00002f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ATan2")
  {
    for (float fScale = 0.125f; fScale < 1000000.0f; fScale *= 2.0f)
    {
      NS_TEST_FLOAT(nsMath::ATan2(0.0f, fScale).GetDegree(), 0.0f, 0.0000001f);
      NS_TEST_FLOAT(nsMath::ATan2(fScale, fScale).GetDegree(), 45.0f, 0.00001f);
      NS_TEST_FLOAT(nsMath::ATan2(fScale, 0.0f).GetDegree(), 90.0f, 0.00001f);
      NS_TEST_FLOAT(nsMath::ATan2(-fScale, fScale).GetDegree(), -45.0f, 0.00001f);
      NS_TEST_FLOAT(nsMath::ATan2(-fScale, 0.0f).GetDegree(), -90.0f, 0.00001f);
      NS_TEST_FLOAT(nsMath::ATan2(0.0f, -fScale).GetDegree(), 180.0f, 0.0001f);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Exp")
  {
    NS_TEST_FLOAT(1.0f, nsMath::Exp(0.0f), 0.000001f);
    NS_TEST_FLOAT(2.7182818284f, nsMath::Exp(1.0f), 0.000001f);
    NS_TEST_FLOAT(7.3890560989f, nsMath::Exp(2.0f), 0.000001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Ln")
  {
    NS_TEST_FLOAT(0.0f, nsMath::Ln(1.0f), 0.000001f);
    NS_TEST_FLOAT(1.0f, nsMath::Ln(2.7182818284f), 0.000001f);
    NS_TEST_FLOAT(2.0f, nsMath::Ln(7.3890560989f), 0.000001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Log2")
  {
    NS_TEST_FLOAT(0.0f, nsMath::Log2(1.0f), 0.000001f);
    NS_TEST_FLOAT(1.0f, nsMath::Log2(2.0f), 0.000001f);
    NS_TEST_FLOAT(2.0f, nsMath::Log2(4.0f), 0.000001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Log2i")
  {
    NS_TEST_BOOL(nsMath::Log2i(0) == nsUInt32(-1));
    NS_TEST_BOOL(nsMath::Log2i(1) == 0);
    NS_TEST_BOOL(nsMath::Log2i(2) == 1);
    NS_TEST_BOOL(nsMath::Log2i(3) == 1);
    NS_TEST_BOOL(nsMath::Log2i(4) == 2);
    NS_TEST_BOOL(nsMath::Log2i(6) == 2);
    NS_TEST_BOOL(nsMath::Log2i(7) == 2);
    NS_TEST_BOOL(nsMath::Log2i(8) == 3);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Log10")
  {
    NS_TEST_FLOAT(0.0f, nsMath::Log10(1.0f), 0.000001f);
    NS_TEST_FLOAT(1.0f, nsMath::Log10(10.0f), 0.000001f);
    NS_TEST_FLOAT(2.0f, nsMath::Log10(100.0f), 0.000001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Log")
  {
    NS_TEST_FLOAT(0.0f, nsMath::Log(2.7182818284f, 1.0f), 0.000001f);
    NS_TEST_FLOAT(1.0f, nsMath::Log(2.7182818284f, 2.7182818284f), 0.000001f);
    NS_TEST_FLOAT(2.0f, nsMath::Log(2.7182818284f, 7.3890560989f), 0.000001f);

    NS_TEST_FLOAT(0.0f, nsMath::Log(2.0f, 1.0f), 0.000001f);
    NS_TEST_FLOAT(1.0f, nsMath::Log(2.0f, 2.0f), 0.000001f);
    NS_TEST_FLOAT(2.0f, nsMath::Log(2.0f, 4.0f), 0.000001f);

    NS_TEST_FLOAT(0.0f, nsMath::Log(10.0f, 1.0f), 0.000001f);
    NS_TEST_FLOAT(1.0f, nsMath::Log(10.0f, 10.0f), 0.000001f);
    NS_TEST_FLOAT(2.0f, nsMath::Log(10.0f, 100.0f), 0.000001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Pow2")
  {
    NS_TEST_FLOAT(1.0f, nsMath::Pow2(0.0f), 0.000001f);
    NS_TEST_FLOAT(2.0f, nsMath::Pow2(1.0f), 0.000001f);
    NS_TEST_FLOAT(4.0f, nsMath::Pow2(2.0f), 0.000001f);

    NS_TEST_BOOL(nsMath::Pow2(0) == 1);
    NS_TEST_BOOL(nsMath::Pow2(1) == 2);
    NS_TEST_BOOL(nsMath::Pow2(2) == 4);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Pow")
  {
    NS_TEST_FLOAT(1.0f, nsMath::Pow(3.0f, 0.0f), 0.000001f);
    NS_TEST_FLOAT(3.0f, nsMath::Pow(3.0f, 1.0f), 0.000001f);
    NS_TEST_FLOAT(9.0f, nsMath::Pow(3.0f, 2.0f), 0.000001f);

    NS_TEST_BOOL(nsMath::Pow(3, 0) == 1);
    NS_TEST_BOOL(nsMath::Pow(3, 1) == 3);
    NS_TEST_BOOL(nsMath::Pow(3, 2) == 9);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Square")
  {
    NS_TEST_FLOAT(0.0f, nsMath::Square(0.0f), 0.000001f);
    NS_TEST_FLOAT(1.0f, nsMath::Square(1.0f), 0.000001f);
    NS_TEST_FLOAT(4.0f, nsMath::Square(2.0f), 0.000001f);
    NS_TEST_FLOAT(4.0f, nsMath::Square(-2.0f), 0.000001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Sqrt (float)")
  {
    NS_TEST_FLOAT(0.0f, nsMath::Sqrt(0.0f), 0.000001f);
    NS_TEST_FLOAT(1.0f, nsMath::Sqrt(1.0f), 0.000001f);
    NS_TEST_FLOAT(2.0f, nsMath::Sqrt(4.0f), 0.000001f);
    NS_TEST_FLOAT(4.0f, nsMath::Sqrt(16.0f), 0.000001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Sqrt (double)")
  {
    NS_TEST_DOUBLE(0.0, nsMath::Sqrt(0.0), 0.000001);
    NS_TEST_DOUBLE(1.0, nsMath::Sqrt(1.0), 0.000001);
    NS_TEST_DOUBLE(2.0, nsMath::Sqrt(4.0), 0.000001);
    NS_TEST_DOUBLE(4.0, nsMath::Sqrt(16.0), 0.000001);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Root")
  {
    NS_TEST_FLOAT(3.0f, nsMath::Root(27.0f, 3.0f), 0.000001f);
    NS_TEST_FLOAT(3.0f, nsMath::Root(81.0f, 4.0f), 0.000001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Sign")
  {
    NS_TEST_FLOAT(0.0f, nsMath::Sign(0.0f), 0.00000001f);
    NS_TEST_FLOAT(1.0f, nsMath::Sign(0.01f), 0.00000001f);
    NS_TEST_FLOAT(-1.0f, nsMath::Sign(-0.01f), 0.00000001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Abs")
  {
    NS_TEST_FLOAT(0.0f, nsMath::Abs(0.0f), 0.00000001f);
    NS_TEST_FLOAT(20.0f, nsMath::Abs(20.0f), 0.00000001f);
    NS_TEST_FLOAT(20.0f, nsMath::Abs(-20.0f), 0.00000001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Min")
  {
    NS_TEST_FLOAT(0.0f, nsMath::Min(0.0f, 23.0f), 0.00000001f);
    NS_TEST_FLOAT(-23.0f, nsMath::Min(0.0f, -23.0f), 0.00000001f);

    NS_TEST_BOOL(nsMath::Min(1, 2, 3) == 1);
    NS_TEST_BOOL(nsMath::Min(4, 2, 3) == 2);
    NS_TEST_BOOL(nsMath::Min(4, 5, 3) == 3);

    NS_TEST_BOOL(nsMath::Min(1, 2, 3, 4) == 1);
    NS_TEST_BOOL(nsMath::Min(5, 2, 3, 4) == 2);
    NS_TEST_BOOL(nsMath::Min(5, 6, 3, 4) == 3);
    NS_TEST_BOOL(nsMath::Min(5, 6, 7, 4) == 4);

    NS_TEST_BOOL(nsMath::Min(UniqueInt(1, 0), UniqueInt(1, 1)).id == 0);
    NS_TEST_BOOL(nsMath::Min(UniqueInt(1, 1), UniqueInt(1, 0)).id == 1);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Max")
  {
    NS_TEST_FLOAT(23.0f, nsMath::Max(0.0f, 23.0f), 0.00000001f);
    NS_TEST_FLOAT(0.0f, nsMath::Max(0.0f, -23.0f), 0.00000001f);

    NS_TEST_BOOL(nsMath::Max(1, 2, 3) == 3);
    NS_TEST_BOOL(nsMath::Max(1, 2, 0) == 2);
    NS_TEST_BOOL(nsMath::Max(1, 0, 0) == 1);

    NS_TEST_BOOL(nsMath::Max(1, 2, 3, 4) == 4);
    NS_TEST_BOOL(nsMath::Max(1, 2, 3, 0) == 3);
    NS_TEST_BOOL(nsMath::Max(1, 2, 0, 0) == 2);
    NS_TEST_BOOL(nsMath::Max(1, 0, 0, 0) == 1);

    NS_TEST_BOOL(nsMath::Max(UniqueInt(1, 0), UniqueInt(1, 1)).id == 0);
    NS_TEST_BOOL(nsMath::Max(UniqueInt(1, 1), UniqueInt(1, 0)).id == 1);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Clamp")
  {
    NS_TEST_FLOAT(15.0f, nsMath::Clamp(23.0f, 12.0f, 15.0f), 0.00000001f);
    NS_TEST_FLOAT(12.0f, nsMath::Clamp(3.0f, 12.0f, 15.0f), 0.00000001f);
    NS_TEST_FLOAT(14.0f, nsMath::Clamp(14.0f, 12.0f, 15.0f), 0.00000001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Saturate")
  {
    NS_TEST_FLOAT(0.0f, nsMath::Saturate(-1.5f), 0.00000001f);
    NS_TEST_FLOAT(0.5f, nsMath::Saturate(0.5f), 0.00000001f);
    NS_TEST_FLOAT(1.0f, nsMath::Saturate(12345.0f), 0.00000001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Floor")
  {
    NS_TEST_BOOL(12 == nsMath::Floor(12.34f));
    NS_TEST_BOOL(-13 == nsMath::Floor(-12.34f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Ceil")
  {
    NS_TEST_BOOL(13 == nsMath::Ceil(12.34f));
    NS_TEST_BOOL(-12 == nsMath::Ceil(-12.34f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "RoundDown (float)")
  {
    NS_TEST_FLOAT(10.0f, nsMath::RoundDown(12.34f, 5.0f), 0.0000001f);
    NS_TEST_FLOAT(-15.0f, nsMath::RoundDown(-12.34f, 5.0f), 0.0000001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "RoundUp (float)")
  {
    NS_TEST_FLOAT(15.0f, nsMath::RoundUp(12.34f, 5.0f), 0.0000001f);
    NS_TEST_FLOAT(-10.0f, nsMath::RoundUp(-12.34f, 5.0f), 0.0000001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "RoundDown (double)")
  {
    NS_TEST_DOUBLE(10.0, nsMath::RoundDown(12.34, 5.0), 0.0000001);
    NS_TEST_DOUBLE(-15.0, nsMath::RoundDown(-12.34, 5.0), 0.0000001);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "RoundUp (double)")
  {
    NS_TEST_DOUBLE(15.0, nsMath::RoundUp(12.34, 5.0), 0.0000001);
    NS_TEST_DOUBLE(-10.0, nsMath::RoundUp(-12.34, 5.0), 0.0000001);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Trunc")
  {
    NS_TEST_BOOL(nsMath::Trunc(12.34f) == 12);
    NS_TEST_BOOL(nsMath::Trunc(-12.34f) == -12);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "FloatToInt")
  {
    NS_TEST_BOOL(nsMath::FloatToInt(12.34f) == 12);
    NS_TEST_BOOL(nsMath::FloatToInt(-12.34f) == -12);

#if NS_DISABLED(NS_PLATFORM_ARCH_X86) || (_MSC_VER <= 1916)
    NS_TEST_BOOL(nsMath::FloatToInt(12000000000000.34) == 12000000000000);
    NS_TEST_BOOL(nsMath::FloatToInt(-12000000000000.34) == -12000000000000);
#endif
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Round")
  {
    NS_TEST_BOOL(nsMath::Round(12.34f) == 12);
    NS_TEST_BOOL(nsMath::Round(-12.34f) == -12);

    NS_TEST_BOOL(nsMath::Round(12.54f) == 13);
    NS_TEST_BOOL(nsMath::Round(-12.54f) == -13);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "RoundClosest (float)")
  {
    NS_TEST_FLOAT(nsMath::RoundToMultiple(12.0f, 3.0f), 12.0f, 0.00001f);
    NS_TEST_FLOAT(nsMath::RoundToMultiple(-12.0f, 3.0f), -12.0f, 0.00001f);
    NS_TEST_FLOAT(nsMath::RoundToMultiple(12.34f, 7.0f), 14.0f, 0.00001f);
    NS_TEST_FLOAT(nsMath::RoundToMultiple(-12.34f, 7.0f), -14.0f, 0.00001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "RoundClosest (double)")
  {
    NS_TEST_DOUBLE(nsMath::RoundToMultiple(12.0, 3.0), 12.0, 0.00001);
    NS_TEST_DOUBLE(nsMath::RoundToMultiple(-12.0, 3.0), -12.0, 0.00001);
    NS_TEST_DOUBLE(nsMath::RoundToMultiple(12.34, 7.0), 14.0, 0.00001);
    NS_TEST_DOUBLE(nsMath::RoundToMultiple(-12.34, 7.0), -14.0, 0.00001);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "RoundUp (int)")
  {
    NS_TEST_INT(nsMath::RoundUp(12, 7), 14);
    NS_TEST_INT(nsMath::RoundUp(-12, 7), -7);
    NS_TEST_INT(nsMath::RoundUp(16, 4), 16);
    NS_TEST_INT(nsMath::RoundUp(-16, 4), -16);
    NS_TEST_INT(nsMath::RoundUp(17, 4), 20);
    NS_TEST_INT(nsMath::RoundUp(-17, 4), -16);
    NS_TEST_INT(nsMath::RoundUp(15, 4), 16);
    NS_TEST_INT(nsMath::RoundUp(-15, 4), -12);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "RoundDown (int)")
  {
    NS_TEST_INT(nsMath::RoundDown(12, 7), 7);
    NS_TEST_INT(nsMath::RoundDown(-12, 7), -14);
    NS_TEST_INT(nsMath::RoundDown(16, 4), 16);
    NS_TEST_INT(nsMath::RoundDown(-16, 4), -16);
    NS_TEST_INT(nsMath::RoundDown(17, 4), 16);
    NS_TEST_INT(nsMath::RoundDown(-17, 4), -20);
    NS_TEST_INT(nsMath::RoundDown(15, 4), 12);
    NS_TEST_INT(nsMath::RoundDown(-15, 4), -16);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "RoundUp (unsigned int)")
  {
    NS_TEST_INT(nsMath::RoundUp(12u, 7), 14);
    NS_TEST_INT(nsMath::RoundUp(16u, 4), 16);
    NS_TEST_INT(nsMath::RoundUp(17u, 4), 20);
    NS_TEST_INT(nsMath::RoundUp(15u, 4), 16);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "RoundDown (unsigned int)")
  {
    NS_TEST_INT(nsMath::RoundDown(12u, 7), 7);
    NS_TEST_INT(nsMath::RoundDown(16u, 4), 16);
    NS_TEST_INT(nsMath::RoundDown(17u, 4), 16);
    NS_TEST_INT(nsMath::RoundDown(15u, 4), 12);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Fraction")
  {
    NS_TEST_FLOAT(nsMath::Fraction(12.34f), 0.34f, 0.00001f);
    NS_TEST_FLOAT(nsMath::Fraction(-12.34f), -0.34f, 0.00001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Mod (float)")
  {
    NS_TEST_FLOAT(2.34f, nsMath::Mod(12.34f, 2.5f), 0.000001f);
    NS_TEST_FLOAT(-2.34f, nsMath::Mod(-12.34f, 2.5f), 0.000001f);

    NS_TEST_FLOAT(2.34f, nsMath::Mod(12.34f, -2.5f), 0.000001f);
    NS_TEST_FLOAT(-2.34f, nsMath::Mod(-12.34f, -2.5f), 0.000001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Mod (double)")
  {
    NS_TEST_DOUBLE(2.34, nsMath::Mod(12.34, 2.5), 0.000001);
    NS_TEST_DOUBLE(-2.34, nsMath::Mod(-12.34, 2.5), 0.000001);

    NS_TEST_DOUBLE(2.34, nsMath::Mod(12.34, -2.5), 0.000001);
    NS_TEST_DOUBLE(-2.34, nsMath::Mod(-12.34, -2.5), 0.000001);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Invert")
  {
    NS_TEST_FLOAT(nsMath::Invert(1.0f), 1.0f, 0.000001f);
    NS_TEST_FLOAT(nsMath::Invert(2.0f), 0.5f, 0.000001f);
    NS_TEST_FLOAT(nsMath::Invert(4.0f), 0.25f, 0.000001f);

    NS_TEST_FLOAT(nsMath::Invert(-1.0f), -1.0f, 0.000001f);
    NS_TEST_FLOAT(nsMath::Invert(-2.0f), -0.5f, 0.000001f);
    NS_TEST_FLOAT(nsMath::Invert(-4.0f), -0.25f, 0.000001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Odd")
  {
    NS_TEST_BOOL(nsMath::IsOdd(0) == false);
    NS_TEST_BOOL(nsMath::IsOdd(1) == true);
    NS_TEST_BOOL(nsMath::IsOdd(2) == false);
    NS_TEST_BOOL(nsMath::IsOdd(-1) == true);
    NS_TEST_BOOL(nsMath::IsOdd(-2) == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Even")
  {
    NS_TEST_BOOL(nsMath::IsEven(0) == true);
    NS_TEST_BOOL(nsMath::IsEven(1) == false);
    NS_TEST_BOOL(nsMath::IsEven(2) == true);
    NS_TEST_BOOL(nsMath::IsEven(-1) == false);
    NS_TEST_BOOL(nsMath::IsEven(-2) == true);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Swap")
  {
    nsInt32 a = 1;
    nsInt32 b = 2;
    nsMath::Swap(a, b);
    NS_TEST_BOOL((a == 2) && (b == 1));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Lerp")
  {
    NS_TEST_FLOAT(nsMath::Lerp(-5.0f, 5.0f, 0.5f), 0.0f, 0.000001);
    NS_TEST_FLOAT(nsMath::Lerp(0.0f, 5.0f, 0.5f), 2.5f, 0.000001);
    NS_TEST_FLOAT(nsMath::Lerp(-5.0f, 5.0f, 0.0f), -5.0f, 0.000001);
    NS_TEST_FLOAT(nsMath::Lerp(-5.0f, 5.0f, 1.0f), 5.0f, 0.000001);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Unlerp")
  {
    NS_TEST_FLOAT(nsMath::Unlerp(-5.0f, 5.0f, 0.0f), 0.5f, 0.000001);
    NS_TEST_FLOAT(nsMath::Unlerp(0.0f, 5.0f, 2.5f), 0.5f, 0.000001);
    NS_TEST_FLOAT(nsMath::Unlerp(-5.0f, 5.0f, -5.0f), 0.0f, 0.000001);
    NS_TEST_FLOAT(nsMath::Unlerp(-5.0f, 5.0f, 5.0f), 1.0f, 0.000001);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Step")
  {
    NS_TEST_FLOAT(nsMath::Step(0.5f, 0.4f), 1.0f, 0.00001f);
    NS_TEST_FLOAT(nsMath::Step(0.3f, 0.4f), 0.0f, 0.00001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SmoothStep")
  {
    // Only test values that must be true for any symmetric step function.
    // How should one test smoothness?
    for (int iScale = -19; iScale <= 19; iScale += 2)
    {
      NS_TEST_FLOAT(nsMath::SmoothStep(0.0f * iScale, 0.1f * iScale, 0.4f * iScale), 0.0f, 0.000001);
      NS_TEST_FLOAT(nsMath::SmoothStep(0.1f * iScale, 0.1f * iScale, 0.4f * iScale), 0.0f, 0.000001);
      NS_TEST_FLOAT(nsMath::SmoothStep(0.4f * iScale, 0.1f * iScale, 0.4f * iScale), 1.0f, 0.000001);
      NS_TEST_FLOAT(nsMath::SmoothStep(0.25f * iScale, 0.1f * iScale, 0.4f * iScale), 0.5f, 0.000001);
      NS_TEST_FLOAT(nsMath::SmoothStep(0.5f * iScale, 0.1f * iScale, 0.4f * iScale), 1.0f, 0.000001);

      NS_TEST_FLOAT(nsMath::SmoothStep(0.5f * iScale, 0.4f * iScale, 0.1f * iScale), 0.0f, 0.000001);
      NS_TEST_FLOAT(nsMath::SmoothStep(0.4f * iScale, 0.4f * iScale, 0.1f * iScale), 0.0f, 0.000001);
      NS_TEST_FLOAT(nsMath::SmoothStep(0.1f * iScale, 0.4f * iScale, 0.1f * iScale), 1.0f, 0.000001);
      NS_TEST_FLOAT(nsMath::SmoothStep(0.25f * iScale, 0.1f * iScale, 0.4f * iScale), 0.5f, 0.000001);
      NS_TEST_FLOAT(nsMath::SmoothStep(0.0f * iScale, 0.4f * iScale, 0.1f * iScale), 1.0f, 0.000001);

      // For edge1 == edge2 SmoothStep should behave like Step
      NS_TEST_FLOAT(nsMath::SmoothStep(0.0f * iScale, 0.1f * iScale, 0.1f * iScale), iScale > 0 ? 0.0f : 1.0f, 0.000001);
      NS_TEST_FLOAT(nsMath::SmoothStep(0.2f * iScale, 0.1f * iScale, 0.1f * iScale), iScale < 0 ? 0.0f : 1.0f, 0.000001);
    }

    NS_TEST_FLOAT(nsMath::SmoothStep(0.2f, 0.0f, 1.0f), 0.104f, 0.00001f);
    NS_TEST_FLOAT(nsMath::SmoothStep(0.4f, 0.2f, 0.8f), 0.259259f, 0.00001f);

    NS_TEST_FLOAT(nsMath::SmootherStep(0.2f, 0.0f, 1.0f), 0.05792f, 0.00001f);
    NS_TEST_FLOAT(nsMath::SmootherStep(0.4f, 0.2f, 0.8f), 0.209876f, 0.00001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsPowerOf")
  {
    NS_TEST_BOOL(nsMath::IsPowerOf(4, 2) == true);
    NS_TEST_BOOL(nsMath::IsPowerOf(5, 2) == false);
    NS_TEST_BOOL(nsMath::IsPowerOf(0, 2) == false);
    NS_TEST_BOOL(nsMath::IsPowerOf(1, 2) == true);

    NS_TEST_BOOL(nsMath::IsPowerOf(4, 3) == false);
    NS_TEST_BOOL(nsMath::IsPowerOf(3, 3) == true);
    NS_TEST_BOOL(nsMath::IsPowerOf(1, 3) == true);
    NS_TEST_BOOL(nsMath::IsPowerOf(27, 3) == true);
    NS_TEST_BOOL(nsMath::IsPowerOf(28, 3) == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsPowerOf2")
  {
    NS_TEST_BOOL(nsMath::IsPowerOf2(4) == true);
    NS_TEST_BOOL(nsMath::IsPowerOf2(5) == false);
    NS_TEST_BOOL(nsMath::IsPowerOf2(0) == false);
    NS_TEST_BOOL(nsMath::IsPowerOf2(1) == true);
    NS_TEST_BOOL(nsMath::IsPowerOf2(0x7FFFFFFFu) == false);
    NS_TEST_BOOL(nsMath::IsPowerOf2(0x80000000u) == true);
    NS_TEST_BOOL(nsMath::IsPowerOf2(0x80000001u) == false);
    NS_TEST_BOOL(nsMath::IsPowerOf2(0xFFFFFFFFu) == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "PowerOf2_Floor")
  {
    NS_TEST_INT(nsMath::PowerOfTwo_Floor(64u), 64);
    NS_TEST_INT(nsMath::PowerOfTwo_Floor(33u), 32);
    NS_TEST_INT(nsMath::PowerOfTwo_Floor(4u), 4);
    NS_TEST_INT(nsMath::PowerOfTwo_Floor(5u), 4);
    NS_TEST_INT(nsMath::PowerOfTwo_Floor(1u), 1);
    NS_TEST_INT(nsMath::PowerOfTwo_Floor(0x80000000), 0x80000000);
    NS_TEST_INT(nsMath::PowerOfTwo_Floor(0x80000001), 0x80000000);
    // strange case...
    NS_TEST_INT(nsMath::PowerOfTwo_Floor(0u), 1);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "PowerOf2_Ceil")
  {
    NS_TEST_INT(nsMath::PowerOfTwo_Ceil(64u), 64);
    NS_TEST_INT(nsMath::PowerOfTwo_Ceil(33u), 64);
    NS_TEST_INT(nsMath::PowerOfTwo_Ceil(4u), 4);
    NS_TEST_INT(nsMath::PowerOfTwo_Ceil(5u), 8);
    NS_TEST_INT(nsMath::PowerOfTwo_Ceil(1u), 1);
    NS_TEST_INT(nsMath::PowerOfTwo_Ceil(0u), 1);
    NS_TEST_INT(nsMath::PowerOfTwo_Ceil(0x7FFFFFFFu), 0x80000000);
    NS_TEST_INT(nsMath::PowerOfTwo_Ceil(0x80000000), 0x80000000);
    // anything above 0x80000000 is undefined behavior due to how left-shift works
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GreatestCommonDivisor")
  {
    NS_TEST_INT(nsMath::GreatestCommonDivisor(13, 13), 13);
    NS_TEST_INT(nsMath::GreatestCommonDivisor(37, 600), 1);
    NS_TEST_INT(nsMath::GreatestCommonDivisor(20, 100), 20);
    NS_TEST_INT(nsMath::GreatestCommonDivisor(624129, 2061517), 18913);
  }


  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsEqual")
  {
    NS_TEST_BOOL(nsMath::IsEqual(1.0f, 0.999f, 0.01f) == true);
    NS_TEST_BOOL(nsMath::IsEqual(1.0f, 1.001f, 0.01f) == true);
    NS_TEST_BOOL(nsMath::IsEqual(1.0f, 0.999f, 0.0001f) == false);
    NS_TEST_BOOL(nsMath::IsEqual(1.0f, 1.001f, 0.0001f) == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "NaN_Infinity")
  {
    if (nsMath::SupportsNaN<nsMathTestType>())
    {
      NS_TEST_BOOL(nsMath::IsNaN(nsMath::NaN<nsMathTestType>()) == true);

      NS_TEST_BOOL(nsMath::Infinity<nsMathTestType>() == nsMath::Infinity<nsMathTestType>() - (nsMathTestType)1);
      NS_TEST_BOOL(nsMath::Infinity<nsMathTestType>() == nsMath::Infinity<nsMathTestType>() + (nsMathTestType)1);

      NS_TEST_BOOL(nsMath::IsNaN(nsMath::Infinity<nsMathTestType>() - nsMath::Infinity<nsMathTestType>()));

      NS_TEST_BOOL(!nsMath::IsFinite(nsMath::Infinity<nsMathTestType>()));
      NS_TEST_BOOL(!nsMath::IsFinite(-nsMath::Infinity<nsMathTestType>()));
      NS_TEST_BOOL(!nsMath::IsFinite(nsMath::NaN<nsMathTestType>()));
      NS_TEST_BOOL(!nsMath::IsNaN(nsMath::Infinity<nsMathTestType>()));
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsInRange")
  {
    NS_TEST_BOOL(nsMath::IsInRange(1.0f, 0.0f, 2.0f) == true);
    NS_TEST_BOOL(nsMath::IsInRange(1.0f, 0.0f, 1.0f) == true);
    NS_TEST_BOOL(nsMath::IsInRange(1.0f, 1.0f, 2.0f) == true);
    NS_TEST_BOOL(nsMath::IsInRange(0.0f, 1.0f, 2.0f) == false);
    NS_TEST_BOOL(nsMath::IsInRange(3.0f, 0.0f, 2.0f) == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsZero")
  {
    NS_TEST_BOOL(nsMath::IsZero(0.009f, 0.01f) == true);
    NS_TEST_BOOL(nsMath::IsZero(0.001f, 0.01f) == true);
    NS_TEST_BOOL(nsMath::IsZero(0.009f, 0.0001f) == false);
    NS_TEST_BOOL(nsMath::IsZero(0.001f, 0.0001f) == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ColorFloatToByte")
  {
    NS_TEST_INT(nsMath::ColorFloatToByte(nsMath::NaN<float>()), 0);
    NS_TEST_INT(nsMath::ColorFloatToByte(-1.0f), 0);
    NS_TEST_INT(nsMath::ColorFloatToByte(0.0f), 0);
    NS_TEST_INT(nsMath::ColorFloatToByte(0.4f), 102);
    NS_TEST_INT(nsMath::ColorFloatToByte(1.0f), 255);
    NS_TEST_INT(nsMath::ColorFloatToByte(1.5f), 255);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ColorFloatToShort")
  {
    NS_TEST_INT(nsMath::ColorFloatToShort(nsMath::NaN<float>()), 0);
    NS_TEST_INT(nsMath::ColorFloatToShort(-1.0f), 0);
    NS_TEST_INT(nsMath::ColorFloatToShort(0.0f), 0);
    NS_TEST_INT(nsMath::ColorFloatToShort(0.4f), 26214);
    NS_TEST_INT(nsMath::ColorFloatToShort(1.0f), 65535);
    NS_TEST_INT(nsMath::ColorFloatToShort(1.5f), 65535);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ColorFloatToSignedByte")
  {
    NS_TEST_INT(nsMath::ColorFloatToSignedByte(nsMath::NaN<float>()), 0);
    NS_TEST_INT(nsMath::ColorFloatToSignedByte(-1.0f), -127);
    NS_TEST_INT(nsMath::ColorFloatToSignedByte(0.0f), 0);
    NS_TEST_INT(nsMath::ColorFloatToSignedByte(0.4f), 51);
    NS_TEST_INT(nsMath::ColorFloatToSignedByte(1.0f), 127);
    NS_TEST_INT(nsMath::ColorFloatToSignedByte(1.5f), 127);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ColorFloatToSignedShort")
  {
    NS_TEST_INT(nsMath::ColorFloatToSignedShort(nsMath::NaN<float>()), 0);
    NS_TEST_INT(nsMath::ColorFloatToSignedShort(-1.0f), -32767);
    NS_TEST_INT(nsMath::ColorFloatToSignedShort(0.0f), 0);
    NS_TEST_INT(nsMath::ColorFloatToSignedShort(0.4f), 13107);
    NS_TEST_INT(nsMath::ColorFloatToSignedShort(0.5f), 16384);
    NS_TEST_INT(nsMath::ColorFloatToSignedShort(1.0f), 32767);
    NS_TEST_INT(nsMath::ColorFloatToSignedShort(1.5f), 32767);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ColorByteToFloat")
  {
    NS_TEST_FLOAT(nsMath::ColorByteToFloat(0), 0.0f, 0.000001f);
    NS_TEST_FLOAT(nsMath::ColorByteToFloat(128), 0.501960784f, 0.000001f);
    NS_TEST_FLOAT(nsMath::ColorByteToFloat(255), 1.0f, 0.000001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ColorShortToFloat")
  {
    NS_TEST_FLOAT(nsMath::ColorShortToFloat(0), 0.0f, 0.000001f);
    NS_TEST_FLOAT(nsMath::ColorShortToFloat(32768), 0.5000076f, 0.000001f);
    NS_TEST_FLOAT(nsMath::ColorShortToFloat(65535), 1.0f, 0.000001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ColorSignedByteToFloat")
  {
    NS_TEST_FLOAT(nsMath::ColorSignedByteToFloat(-128), -1.0f, 0.000001f);
    NS_TEST_FLOAT(nsMath::ColorSignedByteToFloat(-127), -1.0f, 0.000001f);
    NS_TEST_FLOAT(nsMath::ColorSignedByteToFloat(0), 0.0f, 0.000001f);
    NS_TEST_FLOAT(nsMath::ColorSignedByteToFloat(64), 0.50393700787f, 0.000001f);
    NS_TEST_FLOAT(nsMath::ColorSignedByteToFloat(127), 1.0f, 0.000001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ColorSignedShortToFloat")
  {
    NS_TEST_FLOAT(nsMath::ColorSignedShortToFloat(-32768), -1.0f, 0.000001f);
    NS_TEST_FLOAT(nsMath::ColorSignedShortToFloat(-32767), -1.0f, 0.000001f);
    NS_TEST_FLOAT(nsMath::ColorSignedShortToFloat(0), 0.0f, 0.000001f);
    NS_TEST_FLOAT(nsMath::ColorSignedShortToFloat(16384), 0.50001526f, 0.000001f);
    NS_TEST_FLOAT(nsMath::ColorSignedShortToFloat(32767), 1.0f, 0.000001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "EvaluateBnsierCurve")
  {
    // Determined through the scientific method of manually comparing the result of the function with an online Bnsier curve generator:
    // https://www.desmos.com/calculator/cahqdxeshd
    const nsVec2 res[] = {nsVec2(1, 5), nsVec2(0.893f, 4.455f), nsVec2(1.112f, 4.008f), nsVec2(1.557f, 3.631f), nsVec2(2.136f, 3.304f), nsVec2(2.750f, 3.000f),
      nsVec2(3.303f, 2.695f), nsVec2(3.701f, 2.368f), nsVec2(3.847f, 1.991f), nsVec2(3.645f, 1.543f), nsVec2(3, 1)};

    const float step = 1.0f / (NS_ARRAY_SIZE(res) - 1);
    for (int i = 0; i < NS_ARRAY_SIZE(res); ++i)
    {
      const nsVec2 r = nsMath::EvaluateBnsierCurve<nsVec2>(step * i, nsVec2(1, 5), nsVec2(0, 3), nsVec2(6, 3), nsVec2(3, 1));
      NS_TEST_VEC2(r, res[i], 0.002f);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "FirstBitLow")
  {
    NS_TEST_INT(nsMath::FirstBitLow(nsUInt32(0b1111)), 0);
    NS_TEST_INT(nsMath::FirstBitLow(nsUInt32(0b1110)), 1);
    NS_TEST_INT(nsMath::FirstBitLow(nsUInt32(0b1100)), 2);
    NS_TEST_INT(nsMath::FirstBitLow(nsUInt32(0b1000)), 3);
    NS_TEST_INT(nsMath::FirstBitLow(nsUInt32(0xFFFFFFFF)), 0);

    NS_TEST_INT(nsMath::FirstBitLow(nsUInt64(0xFF000000FF00000F)), 0);
    NS_TEST_INT(nsMath::FirstBitLow(nsUInt64(0xFF000000FF00000E)), 1);
    NS_TEST_INT(nsMath::FirstBitLow(nsUInt64(0xFF000000FF00000C)), 2);
    NS_TEST_INT(nsMath::FirstBitLow(nsUInt64(0xFF000000FF000008)), 3);
    NS_TEST_INT(nsMath::FirstBitLow(nsUInt64(0xFFFFFFFFFFFFFFFF)), 0);

    // Edge cases specifically for 32-bit systems where upper and lower 32-bit are handled individually.
    NS_TEST_INT(nsMath::FirstBitLow(nsUInt64(0x00000000FFFFFFFF)), 0);
    NS_TEST_INT(nsMath::FirstBitLow(nsUInt64(0xFFFFFFFF00000000)), 32);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "FirstBitHigh")
  {
    NS_TEST_INT(nsMath::FirstBitHigh(nsUInt32(0b1111)), 3);
    NS_TEST_INT(nsMath::FirstBitHigh(nsUInt32(0b0111)), 2);
    NS_TEST_INT(nsMath::FirstBitHigh(nsUInt32(0b0011)), 1);
    NS_TEST_INT(nsMath::FirstBitHigh(nsUInt32(0b0001)), 0);
    NS_TEST_INT(nsMath::FirstBitHigh(nsUInt32(0xFFFFFFFF)), 31);

    NS_TEST_INT(nsMath::FirstBitHigh(nsUInt64(0x00FF000000FF000F)), 55);
    NS_TEST_INT(nsMath::FirstBitHigh(nsUInt64(0x007F000000FF000F)), 54);
    NS_TEST_INT(nsMath::FirstBitHigh(nsUInt64(0x003F000000FF000F)), 53);
    NS_TEST_INT(nsMath::FirstBitHigh(nsUInt64(0x001F000000FF000F)), 52);
    NS_TEST_INT(nsMath::FirstBitHigh(nsUInt64(0xFFFFFFFFFFFFFFFF)), 63);

    // Edge cases specifically for 32-bit systems where upper and lower 32-bit are handled individually.
    NS_TEST_INT(nsMath::FirstBitHigh(nsUInt64(0x00000000FFFFFFFF)), 31);
    NS_TEST_INT(nsMath::FirstBitHigh(nsUInt64(0xFFFFFFFF00000000)), 63);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CountTrailingZeros (32)")
  {
    NS_TEST_INT(nsMath::CountTrailingZeros(0b1111u), 0);
    NS_TEST_INT(nsMath::CountTrailingZeros(0b1110u), 1);
    NS_TEST_INT(nsMath::CountTrailingZeros(0b1100u), 2);
    NS_TEST_INT(nsMath::CountTrailingZeros(0b1000u), 3);
    NS_TEST_INT(nsMath::CountTrailingZeros(0xFFFFFFFF), 0);
    NS_TEST_INT(nsMath::CountTrailingZeros(0u), 32);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CountTrailingZeros (64)")
  {
    NS_TEST_INT(nsMath::CountTrailingZeros(0b1111llu), 0);
    NS_TEST_INT(nsMath::CountTrailingZeros(0b1110llu), 1);
    NS_TEST_INT(nsMath::CountTrailingZeros(0b1100llu), 2);
    NS_TEST_INT(nsMath::CountTrailingZeros(0b1000llu), 3);
    NS_TEST_INT(nsMath::CountTrailingZeros(0xFFFFFFFF0llu), 4);
    NS_TEST_INT(nsMath::CountTrailingZeros(0llu), 64);
    NS_TEST_INT(nsMath::CountTrailingZeros(0xFFFFFFFF00llu), 8);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CountLeadingZeros")
  {
    NS_TEST_INT(nsMath::CountLeadingZeros(0b1111), 28);
    NS_TEST_INT(nsMath::CountLeadingZeros(0b0111), 29);
    NS_TEST_INT(nsMath::CountLeadingZeros(0b0011), 30);
    NS_TEST_INT(nsMath::CountLeadingZeros(0b0001), 31);
    NS_TEST_INT(nsMath::CountLeadingZeros(0xFFFFFFFF), 0);
    NS_TEST_INT(nsMath::CountLeadingZeros(0), 32);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Bitmask_LowN")
  {
    NS_TEST_INT(nsMath::Bitmask_LowN<nsUInt32>(0), 0);
    NS_TEST_INT(nsMath::Bitmask_LowN<nsUInt32>(1), 1);
    NS_TEST_INT(nsMath::Bitmask_LowN<nsUInt32>(2), 3);
    NS_TEST_INT(nsMath::Bitmask_LowN<nsUInt32>(3), 7);
    NS_TEST_INT(nsMath::Bitmask_LowN<nsUInt32>(31), 0x7fffffff);
    NS_TEST_INT(nsMath::Bitmask_LowN<nsUInt32>(32), 0xffffffffu);
    NS_TEST_INT(nsMath::Bitmask_LowN<nsUInt32>(33), 0xffffffffu);
    NS_TEST_INT(nsMath::Bitmask_LowN<nsUInt32>(50), 0xffffffffu);

    NS_TEST_INT(nsMath::Bitmask_LowN<nsUInt64>(0), 0);
    NS_TEST_INT(nsMath::Bitmask_LowN<nsUInt64>(1), 1);
    NS_TEST_INT(nsMath::Bitmask_LowN<nsUInt64>(2), 3);
    NS_TEST_INT(nsMath::Bitmask_LowN<nsUInt64>(3), 7);
    NS_TEST_INT(nsMath::Bitmask_LowN<nsUInt64>(31), 0x7fffffff);
    NS_TEST_INT(nsMath::Bitmask_LowN<nsUInt64>(32), 0xffffffffu);
    NS_TEST_INT(nsMath::Bitmask_LowN<nsUInt64>(63), 0x7fffffffffffffffull);
    NS_TEST_BOOL(nsMath::Bitmask_LowN<nsUInt64>(64) == 0xffffffffffffffffull);
    NS_TEST_BOOL(nsMath::Bitmask_LowN<nsUInt64>(65) == 0xffffffffffffffffull);
    NS_TEST_BOOL(nsMath::Bitmask_LowN<nsUInt64>(100) == 0xffffffffffffffffull);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Bitmask_HighN")
  {
    NS_TEST_INT(nsMath::Bitmask_HighN<nsUInt32>(0), 0u);
    NS_TEST_INT(nsMath::Bitmask_HighN<nsUInt32>(1), 0x80000000u);
    NS_TEST_INT(nsMath::Bitmask_HighN<nsUInt32>(2), 0xC0000000u);
    NS_TEST_INT(nsMath::Bitmask_HighN<nsUInt32>(3), 0xE0000000u);
    NS_TEST_INT(nsMath::Bitmask_HighN<nsUInt32>(31), 0xfffffffeu);
    NS_TEST_INT(nsMath::Bitmask_HighN<nsUInt32>(32), 0xffffffffu);
    NS_TEST_INT(nsMath::Bitmask_HighN<nsUInt32>(33), 0xffffffffu);
    NS_TEST_INT(nsMath::Bitmask_HighN<nsUInt32>(60), 0xffffffffu);

    NS_TEST_BOOL(nsMath::Bitmask_HighN<nsUInt64>(0) == 0);
    NS_TEST_BOOL(nsMath::Bitmask_HighN<nsUInt64>(1) == 0x8000000000000000llu);
    NS_TEST_BOOL(nsMath::Bitmask_HighN<nsUInt64>(2) == 0xC000000000000000llu);
    NS_TEST_BOOL(nsMath::Bitmask_HighN<nsUInt64>(3) == 0xE000000000000000llu);
    NS_TEST_BOOL(nsMath::Bitmask_HighN<nsUInt64>(31) == 0xfffffffe00000000llu);
    NS_TEST_BOOL(nsMath::Bitmask_HighN<nsUInt64>(32) == 0xffffffff00000000llu);
    NS_TEST_BOOL(nsMath::Bitmask_HighN<nsUInt64>(63) == 0xfffffffffffffffellu);
    NS_TEST_BOOL(nsMath::Bitmask_HighN<nsUInt64>(64) == 0xffffffffffffffffull);
    NS_TEST_BOOL(nsMath::Bitmask_HighN<nsUInt64>(65) == 0xffffffffffffffffull);
    NS_TEST_BOOL(nsMath::Bitmask_HighN<nsUInt64>(1000) == 0xffffffffffffffffull);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "TryMultiply32")
  {
    nsUInt32 res;

    res = 0;
    NS_TEST_BOOL(nsMath::TryMultiply32(res, 1, 1, 2, 3).Succeeded());
    NS_TEST_INT(res, 6);

    res = 0;
    NS_TEST_BOOL(nsMath::TryMultiply32(res, 1, 1, 1, 0xFFFFFFFF).Succeeded());
    NS_TEST_BOOL(res == 0xFFFFFFFF);

    res = 0;
    NS_TEST_BOOL(nsMath::TryMultiply32(res, 0xFFFF, 0x10001).Succeeded());
    NS_TEST_BOOL(res == 0xFFFFFFFF);

    res = 0;
    NS_TEST_BOOL(nsMath::TryMultiply32(res, 0x3FFFFFF, 2, 4, 8).Succeeded());
    NS_TEST_BOOL(res == 0xFFFFFFC0);

    res = 1;
    NS_TEST_BOOL(nsMath::TryMultiply32(res, 0xFFFFFFFF, 2).Failed());
    NS_TEST_BOOL(res == 1);

    res = 0;
    NS_TEST_BOOL(nsMath::TryMultiply32(res, 0x80000000, 2).Failed()); // slightly above 0xFFFFFFFF
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "TryMultiply64")
  {
    nsUInt64 res;

    res = 0;
    NS_TEST_BOOL(nsMath::TryMultiply64(res, 1, 1, 2, 3).Succeeded());
    NS_TEST_INT(res, 6);

    res = 0;
    NS_TEST_BOOL(nsMath::TryMultiply64(res, 1, 1, 1, 0xFFFFFFFF).Succeeded());
    NS_TEST_BOOL(res == 0xFFFFFFFF);

    res = 0;
    NS_TEST_BOOL(nsMath::TryMultiply64(res, 0xFFFF, 0x10001).Succeeded());
    NS_TEST_BOOL(res == 0xFFFFFFFF);

    res = 0;
    NS_TEST_BOOL(nsMath::TryMultiply64(res, 0x3FFFFFF, 2, 4, 8).Succeeded());
    NS_TEST_BOOL(res == 0xFFFFFFC0);

    res = 0;
    NS_TEST_BOOL(nsMath::TryMultiply64(res, 0xFFFFFFFF, 2).Succeeded());
    NS_TEST_BOOL(res == 0x1FFFFFFFE);

    res = 0;
    NS_TEST_BOOL(nsMath::TryMultiply64(res, 0x80000000, 2).Succeeded());
    NS_TEST_BOOL(res == 0x100000000);

    res = 0;
    NS_TEST_BOOL(nsMath::TryMultiply64(res, 0xFFFFFFFF, 0xFFFFFFFF).Succeeded());
    NS_TEST_BOOL(res == 0xFFFFFFFE00000001);

    res = 0;
    NS_TEST_BOOL(nsMath::TryMultiply64(res, 0xFFFFFFFFFFFFFFFF, 2).Failed());

    res = 0;
    NS_TEST_BOOL(nsMath::TryMultiply64(res, 0xFFFFFFFF, 0xFFFFFFFF, 2).Failed());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "TryConvertToSizeT")
  {
    nsUInt64 x = nsMath::MaxValue<nsUInt32>();
    nsUInt64 y = x + 1;

    size_t res = 0;

    NS_TEST_BOOL(nsMath::TryConvertToSizeT(res, x).Succeeded());
    NS_TEST_BOOL(res == x);

    res = 0;
#if NS_ENABLED(NS_PLATFORM_32BIT)
    NS_TEST_BOOL(nsMath::TryConvertToSizeT(res, y).Failed());
#else
    NS_TEST_BOOL(nsMath::TryConvertToSizeT(res, y).Succeeded());
    NS_TEST_BOOL(res == y);
#endif
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ReplaceNaN")
  {
    NS_TEST_FLOAT(nsMath::ReplaceNaN(0.0f, 42.0f), 0.0f, 0);
    NS_TEST_FLOAT(nsMath::ReplaceNaN(nsMath::HighValue<float>(), 2.0f), nsMath::HighValue<float>(), 0);
    NS_TEST_FLOAT(nsMath::ReplaceNaN(-nsMath::HighValue<float>(), 2.0f), -nsMath::HighValue<float>(), 0);

    NS_TEST_FLOAT(nsMath::ReplaceNaN(nsMath::NaN<float>(), 2.0f), 2.0f, 0);
    NS_TEST_FLOAT(nsMath::ReplaceNaN(nsMath::NaN<double>(), 3.0), 3.0, 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ComparisonOperator")
  {
    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::Equal, 1.0, 1.0));
    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::Equal, 1.0, 2.0) == false);

    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::NotEqual, 1.0, 2.0));
    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::NotEqual, 1.0, 1.0) == false);

    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::Less, 1.0, 1.0) == false);
    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::Less, 1.0, 2.0));
    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::Less, -2.0, -1.0));
    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::Less, 3.0, 2.0) == false);

    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::LessEqual, 1.0, 1.0));
    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::LessEqual, 1.0, 2.0));
    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::LessEqual, -2.0, -1.0));
    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::LessEqual, 3.0, 2.0) == false);

    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::Greater, 1.0, 1.0) == false);
    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::Greater, 3.0, 2.0));
    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::Greater, -1.0, -2.0));
    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::Greater, 2.0, 3.0) == false);

    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::GreaterEqual, 1.0, 1.0));
    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::GreaterEqual, 3.0, 2.0));
    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::GreaterEqual, -1.0, -2.0));
    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::GreaterEqual, 2.0, 3.0) == false);

    nsStringView a = "a";
    nsStringView b = "b";
    nsStringView c = "c";
    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::Equal, a, a));
    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::Equal, a, b) == false);

    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::NotEqual, a, c));
    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::NotEqual, a, a) == false);

    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::Less, a, a) == false);
    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::Less, a, b));
    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::Less, c, b) == false);

    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::LessEqual, a, a));
    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::LessEqual, a, b));
    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::LessEqual, c, b) == false);

    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::Greater, a, a) == false);
    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::Greater, c, b));
    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::Greater, a, b) == false);

    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::GreaterEqual, a, a));
    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::GreaterEqual, c, b));
    NS_TEST_BOOL(nsComparisonOperator::Compare(nsComparisonOperator::GreaterEqual, a, b) == false);
  }
}
