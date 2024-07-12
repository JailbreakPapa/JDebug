#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Math/Angle.h>

NS_CREATE_SIMPLE_TEST(Math, Angle)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "DegToRad")
  {
    NS_TEST_FLOAT(nsAngle::DegToRad(0.0f), 0.0f, 0.00001f);
    NS_TEST_FLOAT(nsAngle::DegToRad(45.0f), 0.785398163f, 0.00001f);
    NS_TEST_FLOAT(nsAngle::DegToRad(90.0f), 1.570796327f, 0.00001f);
    NS_TEST_FLOAT(nsAngle::DegToRad(120.0f), 2.094395102f, 0.00001f);
    NS_TEST_FLOAT(nsAngle::DegToRad(170.0f), 2.967059728f, 0.00001f);
    NS_TEST_FLOAT(nsAngle::DegToRad(180.0f), 3.141592654f, 0.00001f);
    NS_TEST_FLOAT(nsAngle::DegToRad(250.0f), 4.36332313f, 0.00001f);
    NS_TEST_FLOAT(nsAngle::DegToRad(320.0f), 5.585053606f, 0.00001f);
    NS_TEST_FLOAT(nsAngle::DegToRad(360.0f), 6.283185307f, 0.00001f);
    NS_TEST_FLOAT(nsAngle::DegToRad(700.0f), 12.217304764f, 0.00001f);
    NS_TEST_FLOAT(nsAngle::DegToRad(-123.0f), -2.14675498f, 0.00001f);
    NS_TEST_FLOAT(nsAngle::DegToRad(-1234.0f), -21.53736297f, 0.00001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "RadToDeg")
  {
    NS_TEST_FLOAT(nsAngle::RadToDeg(0.0f), 0.0f, 0.00001f);
    NS_TEST_FLOAT(nsAngle::RadToDeg(0.785398163f), 45.0f, 0.00001f);
    NS_TEST_FLOAT(nsAngle::RadToDeg(1.570796327f), 90.0f, 0.00001f);
    NS_TEST_FLOAT(nsAngle::RadToDeg(2.094395102f), 120.0f, 0.00001f);
    NS_TEST_FLOAT(nsAngle::RadToDeg(2.967059728f), 170.0f, 0.0001f);
    NS_TEST_FLOAT(nsAngle::RadToDeg(3.141592654f), 180.0f, 0.00001f);
    NS_TEST_FLOAT(nsAngle::RadToDeg(4.36332313f), 250.0f, 0.0001f);
    NS_TEST_FLOAT(nsAngle::RadToDeg(5.585053606f), 320.0f, 0.0001f);
    NS_TEST_FLOAT(nsAngle::RadToDeg(6.283185307f), 360.0f, 0.00001f);
    NS_TEST_FLOAT(nsAngle::RadToDeg(12.217304764f), 700.0f, 0.00001f);
    NS_TEST_FLOAT(nsAngle::RadToDeg(-2.14675498f), -123.0f, 0.00001f);
    NS_TEST_FLOAT(nsAngle::RadToDeg(-21.53736297f), -1234.0f, 0.001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Init")
  {
    nsAngle a0;
    NS_TEST_FLOAT(a0.GetRadian(), 0.0f, 0.0f);
    NS_TEST_FLOAT(a0.GetDegree(), 0.0f, 0.0f);

    nsAngle a1 = nsAngle::MakeFromRadian(1.570796327f);
    NS_TEST_FLOAT(a1.GetRadian(), 1.570796327f, 0.00001f);
    NS_TEST_FLOAT(a1.GetDegree(), 90.0f, 0.00001f);

    nsAngle a2 = nsAngle::MakeFromDegree(90);
    NS_TEST_FLOAT(a2.GetRadian(), 1.570796327f, 0.00001f);
    NS_TEST_FLOAT(a2.GetDegree(), 90.0f, 0.00001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "NormalizeRange / IsEqual ")
  {
    nsAngle a;

    for (nsInt32 i = 1; i < 359; i++)
    {
      a = nsAngle::MakeFromDegree((float)i);
      a.NormalizeRange();
      NS_TEST_FLOAT(a.GetDegree(), (float)i, 0.01f);
      a = nsAngle::MakeFromDegree((float)i);
      a.NormalizeRange();
      NS_TEST_FLOAT(a.GetDegree(), (float)i, 0.01f);
      a = nsAngle::MakeFromDegree((float)i + 360.0f);
      a.NormalizeRange();
      NS_TEST_FLOAT(a.GetDegree(), (float)i, 0.01f);
      a = nsAngle::MakeFromDegree((float)i - 360.0f);
      a.NormalizeRange();
      NS_TEST_FLOAT(a.GetDegree(), (float)i, 0.01f);
      a = nsAngle::MakeFromDegree((float)i + 3600.0f);
      a.NormalizeRange();
      NS_TEST_FLOAT(a.GetDegree(), (float)i, 0.01f);
      a = nsAngle::MakeFromDegree((float)i - 3600.0f);
      a.NormalizeRange();
      NS_TEST_FLOAT(a.GetDegree(), (float)i, 0.01f);
      a = nsAngle::MakeFromDegree((float)i + 36000.0f);
      a.NormalizeRange();
      NS_TEST_FLOAT(a.GetDegree(), (float)i, 0.01f);
      a = nsAngle::MakeFromDegree((float)i - 36000.0f);
      a.NormalizeRange();
      NS_TEST_FLOAT(a.GetDegree(), (float)i, 0.01f);
    }

    for (nsInt32 i = 0; i < 360; i++)
    {
      a = nsAngle::MakeFromDegree((float)i);
      NS_TEST_BOOL(a.GetNormalizedRange().IsEqualSimple(nsAngle::MakeFromDegree((float)i), nsAngle::MakeFromDegree(0.01f)));
      a = nsAngle::MakeFromDegree((float)i);
      NS_TEST_BOOL(a.GetNormalizedRange().IsEqualSimple(nsAngle::MakeFromDegree((float)i), nsAngle::MakeFromDegree(0.01f)));
      a = nsAngle::MakeFromDegree((float)i + 360.0f);
      NS_TEST_BOOL(a.GetNormalizedRange().IsEqualSimple(nsAngle::MakeFromDegree((float)i), nsAngle::MakeFromDegree(0.01f)));
      a = nsAngle::MakeFromDegree((float)i - 360.0f);
      NS_TEST_BOOL(a.GetNormalizedRange().IsEqualSimple(nsAngle::MakeFromDegree((float)i), nsAngle::MakeFromDegree(0.01f)));
      a = nsAngle::MakeFromDegree((float)i + 3600.0f);
      NS_TEST_BOOL(a.GetNormalizedRange().IsEqualSimple(nsAngle::MakeFromDegree((float)i), nsAngle::MakeFromDegree(0.01f)));
      a = nsAngle::MakeFromDegree((float)i - 3600.0f);
      NS_TEST_BOOL(a.GetNormalizedRange().IsEqualSimple(nsAngle::MakeFromDegree((float)i), nsAngle::MakeFromDegree(0.01f)));
      a = nsAngle::MakeFromDegree((float)i + 36000.0f);
      NS_TEST_BOOL(a.GetNormalizedRange().IsEqualSimple(nsAngle::MakeFromDegree((float)i), nsAngle::MakeFromDegree(0.01f)));
      a = nsAngle::MakeFromDegree((float)i - 36000.0f);
      NS_TEST_BOOL(a.GetNormalizedRange().IsEqualSimple(nsAngle::MakeFromDegree((float)i), nsAngle::MakeFromDegree(0.01f)));
    }

    for (nsInt32 i = 0; i < 360; i++)
    {
      a = nsAngle::MakeFromDegree((float)i);
      NS_TEST_BOOL(a.IsEqualNormalized(nsAngle::MakeFromDegree((float)i), nsAngle::MakeFromDegree(0.01f)));
      a = nsAngle::MakeFromDegree((float)i);
      NS_TEST_BOOL(a.IsEqualNormalized(nsAngle::MakeFromDegree((float)i), nsAngle::MakeFromDegree(0.01f)));
      a = nsAngle::MakeFromDegree((float)i + 360.0f);
      NS_TEST_BOOL(a.IsEqualNormalized(nsAngle::MakeFromDegree((float)i), nsAngle::MakeFromDegree(0.01f)));
      a = nsAngle::MakeFromDegree((float)i - 360.0f);
      NS_TEST_BOOL(a.IsEqualNormalized(nsAngle::MakeFromDegree((float)i), nsAngle::MakeFromDegree(0.01f)));
      a = nsAngle::MakeFromDegree((float)i + 3600.0f);
      NS_TEST_BOOL(a.IsEqualNormalized(nsAngle::MakeFromDegree((float)i), nsAngle::MakeFromDegree(0.01f)));
      a = nsAngle::MakeFromDegree((float)i - 3600.0f);
      NS_TEST_BOOL(a.IsEqualNormalized(nsAngle::MakeFromDegree((float)i), nsAngle::MakeFromDegree(0.01f)));
      a = nsAngle::MakeFromDegree((float)i + 36000.0f);
      NS_TEST_BOOL(a.IsEqualNormalized(nsAngle::MakeFromDegree((float)i), nsAngle::MakeFromDegree(0.01f)));
      a = nsAngle::MakeFromDegree((float)i - 36000.0f);
      NS_TEST_BOOL(a.IsEqualNormalized(nsAngle::MakeFromDegree((float)i), nsAngle::MakeFromDegree(0.01f)));
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "AngleBetween")
  {
    NS_TEST_FLOAT(nsAngle::AngleBetween(nsAngle::MakeFromDegree(0), nsAngle::MakeFromDegree(0)).GetDegree(), 0.0f, 0.0001f);
    NS_TEST_FLOAT(nsAngle::AngleBetween(nsAngle::MakeFromDegree(0), nsAngle::MakeFromDegree(360)).GetDegree(), 0.0f, 0.0001f);
    NS_TEST_FLOAT(nsAngle::AngleBetween(nsAngle::MakeFromDegree(360), nsAngle::MakeFromDegree(360)).GetDegree(), 0.0f, 0.0001f);
    NS_TEST_FLOAT(nsAngle::AngleBetween(nsAngle::MakeFromDegree(360), nsAngle::MakeFromDegree(0)).GetDegree(), 0.0f, 0.0001f);

    NS_TEST_FLOAT(nsAngle::AngleBetween(nsAngle::MakeFromDegree(5), nsAngle::MakeFromDegree(186)).GetDegree(), 179.0f, 0.0001f);
    NS_TEST_FLOAT(nsAngle::AngleBetween(nsAngle::MakeFromDegree(-5), nsAngle::MakeFromDegree(-186)).GetDegree(), 179.0f, 0.0001f);

    NS_TEST_FLOAT(nsAngle::AngleBetween(nsAngle::MakeFromDegree(360.0f + 5), nsAngle::MakeFromDegree(360.0f + 186)).GetDegree(), 179.0f, 0.0001f);
    NS_TEST_FLOAT(nsAngle::AngleBetween(nsAngle::MakeFromDegree(360.0f + -5), nsAngle::MakeFromDegree(360.0f - 186)).GetDegree(), 179.0f, 0.0001f);

    for (nsInt32 i = 0; i <= 179; ++i)
      NS_TEST_FLOAT(nsAngle::AngleBetween(nsAngle::MakeFromDegree((float)i), nsAngle::MakeFromDegree((float)(i + i))).GetDegree(), (float)i, 0.0001f);

    for (nsInt32 i = -179; i <= 0; ++i)
      NS_TEST_FLOAT(nsAngle::AngleBetween(nsAngle::MakeFromDegree((float)i), nsAngle::MakeFromDegree((float)(i + i))).GetDegree(), (float)-i, 0.0001f);
  }
}
