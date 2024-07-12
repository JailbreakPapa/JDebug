#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Math/Color.h>
#include <Foundation/Math/Color8UNorm.h>


NS_CREATE_SIMPLE_TEST(Math, Color8UNorm)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor empty")
  {
    // Placement new of the default constructor should not have any effect on the previous data.
    nsUInt8 testBlock[4] = {0, 64, 128, 255};
    nsColorLinearUB* pDefCtor = ::new ((void*)&testBlock[0]) nsColorLinearUB;
    NS_TEST_BOOL(pDefCtor->r == 0 && pDefCtor->g == 64 && pDefCtor->b == 128 && pDefCtor->a == 255);

    // Make sure the class didn't accidentally change in size
    NS_TEST_BOOL(sizeof(nsColorLinearUB) == sizeof(nsUInt8) * 4);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor components")
  {
    nsColorLinearUB init3(100, 123, 255);
    NS_TEST_BOOL(init3.r == 100 && init3.g == 123 && init3.b == 255 && init3.a == 255);

    nsColorLinearUB init4(100, 123, 255, 42);
    NS_TEST_BOOL(init4.r == 100 && init4.g == 123 && init4.b == 255 && init4.a == 42);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor copy")
  {
    nsColorLinearUB init4(100, 123, 255, 42);
    nsColorLinearUB copy(init4);
    NS_TEST_BOOL(copy.r == 100 && copy.g == 123 && copy.b == 255 && copy.a == 42);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor nsColor")
  {
    nsColorLinearUB fromColor32f(nsColor(0.39f, 0.58f, 0.93f));
    NS_TEST_BOOL(nsMath::IsEqual<nsUInt8>(fromColor32f.r, static_cast<nsUInt8>(nsColor(0.39f, 0.58f, 0.93f).r * 255), 2) &&
                 nsMath::IsEqual<nsUInt8>(fromColor32f.g, static_cast<nsUInt8>(nsColor(0.39f, 0.58f, 0.93f).g * 255), 2) &&
                 nsMath::IsEqual<nsUInt8>(fromColor32f.b, static_cast<nsUInt8>(nsColor(0.39f, 0.58f, 0.93f).b * 255), 2) &&
                 nsMath::IsEqual<nsUInt8>(fromColor32f.a, static_cast<nsUInt8>(nsColor(0.39f, 0.58f, 0.93f).a * 255), 2));
  }

  // conversion
  {
    nsColorLinearUB cornflowerBlue(nsColor(0.39f, 0.58f, 0.93f));

    NS_TEST_BLOCK(nsTestBlock::Enabled, "Conversion nsColor")
    {
      nsColor color32f = cornflowerBlue;
      NS_TEST_BOOL(nsMath::IsEqual<float>(color32f.r, nsColor(0.39f, 0.58f, 0.93f).r, 2.0f / 255.0f) &&
                   nsMath::IsEqual<float>(color32f.g, nsColor(0.39f, 0.58f, 0.93f).g, 2.0f / 255.0f) &&
                   nsMath::IsEqual<float>(color32f.b, nsColor(0.39f, 0.58f, 0.93f).b, 2.0f / 255.0f) &&
                   nsMath::IsEqual<float>(color32f.a, nsColor(0.39f, 0.58f, 0.93f).a, 2.0f / 255.0f));
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "Conversion nsUInt*")
    {
      const nsUInt8* pUIntsConst = cornflowerBlue.GetData();
      NS_TEST_BOOL(pUIntsConst[0] == cornflowerBlue.r && pUIntsConst[1] == cornflowerBlue.g && pUIntsConst[2] == cornflowerBlue.b &&
                   pUIntsConst[3] == cornflowerBlue.a);

      nsUInt8* pUInts = cornflowerBlue.GetData();
      NS_TEST_BOOL(pUInts[0] == cornflowerBlue.r && pUInts[1] == cornflowerBlue.g && pUInts[2] == cornflowerBlue.b && pUInts[3] == cornflowerBlue.a);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsColorGammaUB: Constructor")
  {
    nsColorGammaUB c(50, 150, 200, 100);
    NS_TEST_INT(c.r, 50);
    NS_TEST_INT(c.g, 150);
    NS_TEST_INT(c.b, 200);
    NS_TEST_INT(c.a, 100);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsColorGammaUB: Constructor (nsColor)")
  {
    nsColorGammaUB c2 = nsColor::RebeccaPurple;

    nsColor c3 = c2;

    NS_TEST_BOOL(c3.IsEqualRGBA(nsColor::RebeccaPurple, 0.001f));
  }
}
