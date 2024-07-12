#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Math/Color.h>
#include <Foundation/Math/Color8UNorm.h>
#include <Foundation/Math/Mat4.h>

NS_CREATE_SIMPLE_TEST(Math, Color)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor empty")
  {
#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
    if (nsMath::SupportsNaN<nsMathTestType>())
    {
      // In debug the default constructor initializes everything with NaN.
      nsColor defCtor;
      NS_TEST_BOOL(nsMath::IsNaN(defCtor.r) && nsMath::IsNaN(defCtor.g) && nsMath::IsNaN(defCtor.b) && nsMath::IsNaN(defCtor.a));
    }
#else
    // Placement new of the default constructor should not have any effect on the previous data.
    float testBlock[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    nsColor* pDefCtor = ::new ((void*)&testBlock[0]) nsColor;
    NS_TEST_BOOL(pDefCtor->r == 1.0f && pDefCtor->g == 2.0f && pDefCtor->b == 3.0f && pDefCtor->a == 4.0f);
#endif

    // Make sure the class didn't accidentally change in size
    NS_TEST_BOOL(sizeof(nsColor) == sizeof(float) * 4);
  }
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor components")
  {
    nsColor init3F(0.5f, 0.6f, 0.7f);
    NS_TEST_BOOL(init3F.r == 0.5f && init3F.g == 0.6f && init3F.b == 0.7f && init3F.a == 1.0f);

    nsColor init4F(0.5f, 0.6f, 0.7f, 0.8f);
    NS_TEST_BOOL(init4F.r == 0.5f && init4F.g == 0.6f && init4F.b == 0.7f && init4F.a == 0.8f);
  }
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor copy")
  {
    nsColor init4F(0.5f, 0.6f, 0.7f, 0.8f);
    nsColor copy(init4F);
    NS_TEST_BOOL(copy.r == 0.5f && copy.g == 0.6f && copy.b == 0.7f && copy.a == 0.8f);
  }

  {
    nsColor cornflowerBlue(nsColor(0.39f, 0.58f, 0.93f));

    NS_TEST_BLOCK(nsTestBlock::Enabled, "Conversion float")
    {
      float* pFloats = cornflowerBlue.GetData();
      NS_TEST_BOOL(
        pFloats[0] == cornflowerBlue.r && pFloats[1] == cornflowerBlue.g && pFloats[2] == cornflowerBlue.b && pFloats[3] == cornflowerBlue.a);

      const float* pConstFloats = cornflowerBlue.GetData();
      NS_TEST_BOOL(pConstFloats[0] == cornflowerBlue.r && pConstFloats[1] == cornflowerBlue.g && pConstFloats[2] == cornflowerBlue.b &&
                   pConstFloats[3] == cornflowerBlue.a);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "HSV conversion")
  {
    nsColor normalizedColor(0.0f, 1.0f, 0.999f, 0.0001f);
    NS_TEST_BOOL(normalizedColor.IsNormalized());
    nsColor notNormalizedColor0(-0.01f, 1.0f, 0.999f, 0.0001f);
    NS_TEST_BOOL(!notNormalizedColor0.IsNormalized());
    nsColor notNormalizedColor1(0.5f, 1.1f, 0.9f, 0.1f);
    NS_TEST_BOOL(!notNormalizedColor1.IsNormalized());
    nsColor notNormalizedColor2(0.1f, 1.0f, 1.999f, 0.1f);
    NS_TEST_BOOL(!notNormalizedColor2.IsNormalized());
    nsColor notNormalizedColor3(0.1f, 1.0f, 1.0f, -0.1f);
    NS_TEST_BOOL(!notNormalizedColor3.IsNormalized());


    // hsv test - took some samples from http://www.javascripter.net/faq/rgb2hsv.htm
    const nsColorGammaUB rgb[] = {nsColorGammaUB(255, 255, 255), nsColorGammaUB(0, 0, 0), nsColorGammaUB(123, 12, 1), nsColorGammaUB(31, 112, 153)};
    const nsVec3 hsv[] = {nsVec3(0, 0, 1), nsVec3(0, 0, 0), nsVec3(5.4f, 0.991f, 0.48f), nsVec3(200.2f, 0.797f, 0.600f)};

    for (int i = 0; i < 4; ++i)
    {
      const nsColor color = rgb[i];
      float hue, sat, val;
      color.GetHSV(hue, sat, val);

      NS_TEST_FLOAT(hue, hsv[i].x, 0.1f);
      NS_TEST_FLOAT(sat, hsv[i].y, 0.1f);
      NS_TEST_FLOAT(val, hsv[i].z, 0.1f);

      nsColor fromHSV = nsColor::MakeHSV(hsv[i].x, hsv[i].y, hsv[i].z);
      NS_TEST_FLOAT(fromHSV.r, color.r, 0.01f);
      NS_TEST_FLOAT(fromHSV.g, color.g, 0.01f);
      NS_TEST_FLOAT(fromHSV.b, color.b, 0.01f);
    }
  }

  {
    if (nsMath::SupportsNaN<nsMathTestType>())
    {
      float fNaN = nsMath::NaN<float>();
      const nsColor nanArray[4] = {
        nsColor(fNaN, 0.0f, 0.0f, 0.0f), nsColor(0.0f, fNaN, 0.0f, 0.0f), nsColor(0.0f, 0.0f, fNaN, 0.0f), nsColor(0.0f, 0.0f, 0.0f, fNaN)};
      const nsColor compArray[4] = {
        nsColor(1.0f, 0.0f, 0.0f, 0.0f), nsColor(0.0f, 1.0f, 0.0f, 0.0f), nsColor(0.0f, 0.0f, 1.0f, 0.0f), nsColor(0.0f, 0.0f, 0.0f, 1.0f)};


      NS_TEST_BLOCK(nsTestBlock::Enabled, "IsNaN")
      {
        for (int i = 0; i < 4; ++i)
        {
          NS_TEST_BOOL(nanArray[i].IsNaN());
          NS_TEST_BOOL(!compArray[i].IsNaN());
        }
      }

      NS_TEST_BLOCK(nsTestBlock::Enabled, "IsValid")
      {
        for (int i = 0; i < 4; ++i)
        {
          NS_TEST_BOOL(!nanArray[i].IsValid());
          NS_TEST_BOOL(compArray[i].IsValid());

          NS_TEST_BOOL(!(compArray[i] * nsMath::Infinity<float>()).IsValid());
          NS_TEST_BOOL(!(compArray[i] * -nsMath::Infinity<float>()).IsValid());
        }
      }
    }
  }

  {
    const nsColor op1(-4.0, 0.2f, -7.0f, -0.0f);
    const nsColor op2(2.0, 0.3f, 0.0f, 1.0f);
    const nsColor compArray[4] = {
      nsColor(1.0f, 0.0f, 0.0f, 0.0f), nsColor(0.0f, 1.0f, 0.0f, 0.0f), nsColor(0.0f, 0.0f, 1.0f, 0.0f), nsColor(0.0f, 0.0f, 0.0f, 1.0f)};

    NS_TEST_BLOCK(nsTestBlock::Enabled, "SetRGB / SetRGBA")
    {
      nsColor c1(0, 0, 0, 0);

      c1.SetRGBA(1, 2, 3, 4);

      NS_TEST_BOOL(c1 == nsColor(1, 2, 3, 4));

      c1.SetRGB(5, 6, 7);

      NS_TEST_BOOL(c1 == nsColor(5, 6, 7, 4));
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "IsIdenticalRGB")
    {
      nsColor c1(0, 0, 0, 0);
      nsColor c2(0, 0, 0, 1);

      NS_TEST_BOOL(c1.IsIdenticalRGB(c2));
      NS_TEST_BOOL(!c1.IsIdenticalRGBA(c2));
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "IsIdenticalRGBA")
    {
      NS_TEST_BOOL(op1.IsIdenticalRGBA(op1));
      for (int i = 0; i < 4; ++i)
      {
        NS_TEST_BOOL(!op1.IsIdenticalRGBA(op1 + nsMath::SmallEpsilon<float>() * compArray[i]));
        NS_TEST_BOOL(!op1.IsIdenticalRGBA(op1 - nsMath::SmallEpsilon<float>() * compArray[i]));
      }
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "IsEqualRGB")
    {
      nsColor c1(0, 0, 0, 0);
      nsColor c2(0, 0, 0.2f, 1);

      NS_TEST_BOOL(!c1.IsEqualRGB(c2, 0.1f));
      NS_TEST_BOOL(c1.IsEqualRGB(c2, 0.3f));
      NS_TEST_BOOL(!c1.IsEqualRGBA(c2, 0.3f));
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "IsEqualRGBA")
    {
      NS_TEST_BOOL(op1.IsEqualRGBA(op1, 0.0f));
      for (int i = 0; i < 4; ++i)
      {
        NS_TEST_BOOL(op1.IsEqualRGBA(op1 + nsMath::SmallEpsilon<float>() * compArray[i], 2 * nsMath::SmallEpsilon<float>()));
        NS_TEST_BOOL(op1.IsEqualRGBA(op1 - nsMath::SmallEpsilon<float>() * compArray[i], 2 * nsMath::SmallEpsilon<float>()));
        NS_TEST_BOOL(op1.IsEqualRGBA(op1 + nsMath::DefaultEpsilon<float>() * compArray[i], 2 * nsMath::DefaultEpsilon<float>()));
        NS_TEST_BOOL(op1.IsEqualRGBA(op1 - nsMath::DefaultEpsilon<float>() * compArray[i], 2 * nsMath::DefaultEpsilon<float>()));
      }
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "operator+= (nsColor)")
    {
      nsColor plusAssign = op1;
      plusAssign += op2;
      NS_TEST_BOOL(plusAssign.IsEqualRGBA(nsColor(-2.0f, 0.5f, -7.0f, 1.0f), nsMath::SmallEpsilon<float>()));
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "operator-= (nsColor)")
    {
      nsColor minusAssign = op1;
      minusAssign -= op2;
      NS_TEST_BOOL(minusAssign.IsEqualRGBA(nsColor(-6.0f, -0.1f, -7.0f, -1.0f), nsMath::SmallEpsilon<float>()));
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "ooperator*= (float)")
    {
      nsColor mulFloat = op1;
      mulFloat *= 2.0f;
      NS_TEST_BOOL(mulFloat.IsEqualRGBA(nsColor(-8.0f, 0.4f, -14.0f, -0.0f), nsMath::SmallEpsilon<float>()));
      mulFloat *= 0.0f;
      NS_TEST_BOOL(mulFloat.IsEqualRGBA(nsColor(0.0f, 0.0f, 0.0f, 0.0f), nsMath::SmallEpsilon<float>()));
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "operator/= (float)")
    {
      nsColor vDivFloat = op1;
      vDivFloat /= 2.0f;
      NS_TEST_BOOL(vDivFloat.IsEqualRGBA(nsColor(-2.0f, 0.1f, -3.5f, -0.0f), nsMath::SmallEpsilon<float>()));
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "operator+ (nsColor, nsColor)")
    {
      nsColor plus = (op1 + op2);
      NS_TEST_BOOL(plus.IsEqualRGBA(nsColor(-2.0f, 0.5f, -7.0f, 1.0f), nsMath::SmallEpsilon<float>()));
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "operator- (nsColor, nsColor)")
    {
      nsColor minus = (op1 - op2);
      NS_TEST_BOOL(minus.IsEqualRGBA(nsColor(-6.0f, -0.1f, -7.0f, -1.0f), nsMath::SmallEpsilon<float>()));
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "operator* (float, nsColor)")
    {
      nsColor mulFloatVec4 = 2 * op1;
      NS_TEST_BOOL(mulFloatVec4.IsEqualRGBA(nsColor(-8.0f, 0.4f, -14.0f, -0.0f), nsMath::SmallEpsilon<float>()));
      mulFloatVec4 = ((float)0 * op1);
      NS_TEST_BOOL(mulFloatVec4.IsEqualRGBA(nsColor(0.0f, 0.0f, 0.0f, 0.0f), nsMath::SmallEpsilon<float>()));
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "operator* (nsColor, float)")
    {
      nsColor mulVec4Float = op1 * 2;
      NS_TEST_BOOL(mulVec4Float.IsEqualRGBA(nsColor(-8.0f, 0.4f, -14.0f, -0.0f), nsMath::SmallEpsilon<float>()));
      mulVec4Float = (op1 * (float)0);
      NS_TEST_BOOL(mulVec4Float.IsEqualRGBA(nsColor(0.0f, 0.0f, 0.0f, 0.0f), nsMath::SmallEpsilon<float>()));
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "operator/ (nsColor, float)")
    {
      nsColor vDivVec4Float = op1 / 2;
      NS_TEST_BOOL(vDivVec4Float.IsEqualRGBA(nsColor(-2.0f, 0.1f, -3.5f, -0.0f), nsMath::SmallEpsilon<float>()));
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "operator== (nsColor, nsColor)")
    {
      NS_TEST_BOOL(op1 == op1);
      for (int i = 0; i < 4; ++i)
      {
        NS_TEST_BOOL(!(op1 == (op1 + nsMath::SmallEpsilon<float>() * compArray[i])));
        NS_TEST_BOOL(!(op1 == (op1 - nsMath::SmallEpsilon<float>() * compArray[i])));
      }
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "operator< (nsColor, nsColor)")
    {
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

    NS_TEST_BLOCK(nsTestBlock::Enabled, "operator!= (nsColor, nsColor)")
    {
      NS_TEST_BOOL(!(op1 != op1));
      for (int i = 0; i < 4; ++i)
      {
        NS_TEST_BOOL(op1 != (op1 + nsMath::SmallEpsilon<float>() * compArray[i]));
        NS_TEST_BOOL(op1 != (op1 - nsMath::SmallEpsilon<float>() * compArray[i]));
      }
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "operator= (nsColorLinearUB)")
    {
      nsColor c;
      nsColorLinearUB lin(50, 100, 150, 255);

      c = lin;

      NS_TEST_FLOAT(c.r, 50 / 255.0f, 0.001f);
      NS_TEST_FLOAT(c.g, 100 / 255.0f, 0.001f);
      NS_TEST_FLOAT(c.b, 150 / 255.0f, 0.001f);
      NS_TEST_FLOAT(c.a, 1.0f, 0.001f);
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "operator= (nsColorGammaUB) / constructor(nsColorGammaUB)")
    {
      nsColor c;
      nsColorGammaUB gamma(50, 100, 150, 255);

      c = gamma;
      nsColor c3 = gamma;

      NS_TEST_BOOL(c == c3);

      NS_TEST_FLOAT(c.r, 0.031f, 0.001f);
      NS_TEST_FLOAT(c.g, 0.127f, 0.001f);
      NS_TEST_FLOAT(c.b, 0.304f, 0.001f);
      NS_TEST_FLOAT(c.a, 1.0f, 0.001f);

      nsColorGammaUB c2 = c;

      NS_TEST_INT(c2.r, 50);
      NS_TEST_INT(c2.g, 100);
      NS_TEST_INT(c2.b, 150);
      NS_TEST_INT(c2.a, 255);
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "GetInvertedColor")
    {
      const nsColor c1(0.1f, 0.3f, 0.7f, 0.9f);

      nsColor c2 = c1.GetInvertedColor();

      NS_TEST_BOOL(c2.IsEqualRGBA(nsColor(0.9f, 0.7f, 0.3f, 0.1f), 0.01f));
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "GetLuminance")
    {
      NS_TEST_FLOAT(nsColor::Black.GetLuminance(), 0.0f, 0.001f);
      NS_TEST_FLOAT(nsColor::White.GetLuminance(), 1.0f, 0.001f);

      NS_TEST_FLOAT(nsColor(0.5f, 0.5f, 0.5f).GetLuminance(), 0.2126f * 0.5f + 0.7152f * 0.5f + 0.0722f * 0.5f, 0.001f);
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "GetComplementaryColor")
    {
      // black and white have no complementary colors, or rather, they are their own complementary colors, apparently
      NS_TEST_BOOL(nsColor::Black.GetComplementaryColor().IsEqualRGBA(nsColor::Black, 0.001f));
      NS_TEST_BOOL(nsColor::White.GetComplementaryColor().IsEqualRGBA(nsColor::White, 0.001f));

      NS_TEST_BOOL(nsColor::Red.GetComplementaryColor().IsEqualRGBA(nsColor::Cyan, 0.001f));
      NS_TEST_BOOL(nsColor::Lime.GetComplementaryColor().IsEqualRGBA(nsColor::Magenta, 0.001f));
      NS_TEST_BOOL(nsColor::Blue.GetComplementaryColor().IsEqualRGBA(nsColor::Yellow, 0.001f));
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "GetSaturation")
    {
      NS_TEST_FLOAT(nsColor::Black.GetSaturation(), 0.0f, 0.001f);
      NS_TEST_FLOAT(nsColor::White.GetSaturation(), 0.0f, 0.001f);
      NS_TEST_FLOAT(nsColor::Red.GetSaturation(), 1.0f, 0.001f);
      NS_TEST_FLOAT(nsColor::Lime.GetSaturation(), 1.0f, 0.001f);
      ;
      NS_TEST_FLOAT(nsColor::Blue.GetSaturation(), 1.0f, 0.001f);
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "operator * / *= (nsMat4)")
    {
      nsMat4 m;
      m.SetIdentity();
      m = nsMat4::MakeScaling(nsVec3(0.5f, 0.75f, 0.25f));
      m.SetTranslationVector(nsVec3(0.1f, 0.2f, 0.3f));

      nsColor c1 = m * nsColor::White;

      NS_TEST_BOOL(c1.IsEqualRGBA(nsColor(0.6f, 0.95f, 0.55f, 1.0f), 0.01f));
    }
  }
}
