#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Containers/StaticArray.h>
#include <Foundation/IO/FileSystem/DataDirTypeFolder.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Math/Random.h>

// only works when also linking against CoreUtils
// #define USE_NSIMAGE

#ifdef USE_NSIMAGE
#  include <Texture/Image/Image.h>
#endif


NS_CREATE_SIMPLE_TEST(Math, Random)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "UIntInRange")
  {
    nsRandom r;
    r.Initialize(0xAABBCCDDEEFF0011ULL);

    for (nsUInt32 i = 2; i < 10000; ++i)
    {
      const nsUInt32 val = r.UIntInRange(i);
      NS_TEST_BOOL(val < i);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IntInRange")
  {
    nsRandom r;
    r.Initialize(0xBBCCDDEEFF0011AAULL);

    NS_TEST_INT(r.IntInRange(5, 1), 5);
    NS_TEST_INT(r.IntInRange(-5, 1), -5);

    for (nsInt32 i = 2; i < 10000; ++i)
    {
      const nsInt32 val = r.IntInRange(i, i);
      NS_TEST_BOOL(val >= i);
      NS_TEST_BOOL(val < i + i);
    }

    for (nsInt32 i = 2; i < 10000; ++i)
    {
      const nsInt32 val = r.IntInRange(-i, 2 * i);
      NS_TEST_BOOL(val >= -i);
      NS_TEST_BOOL(val < -i + 2 * i);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IntMinMax")
  {
    nsRandom r;
    r.Initialize(0xCCDDEEFF0011AABBULL);

    NS_TEST_INT(r.IntMinMax(5, 5), 5);
    NS_TEST_INT(r.IntMinMax(-5, -5), -5);

    for (nsInt32 i = 2; i < 10000; ++i)
    {
      const nsInt32 val = r.IntMinMax(i, 2 * i);
      NS_TEST_BOOL(val >= i);
      NS_TEST_BOOL(val <= i + i);
    }

    for (nsInt32 i = 2; i < 10000; ++i)
    {
      const nsInt32 val = r.IntMinMax(-i, i);
      NS_TEST_BOOL(val >= -i);
      NS_TEST_BOOL(val <= i);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Bool")
  {
    nsRandom r;
    r.Initialize(0x11AABBCCDDEEFFULL);

    nsUInt32 falseCount = 0;
    nsUInt32 trueCount = 0;
    nsDynamicArray<bool> values;
    values.SetCount(1000);

    for (int i = 0; i < 1000; ++i)
    {
      values[i] = r.Bool();
      if (values[i])
      {
        ++trueCount;
      }
      else
      {
        ++falseCount;
      }
    }

    // This could be more elaborate, one could also test the variance
    // and assert that approximately an uniform distribution is yielded
    NS_TEST_BOOL(trueCount > 0 && falseCount > 0);

    nsRandom r2;
    r2.Initialize(0x11AABBCCDDEEFFULL);

    for (int i = 0; i < 1000; ++i)
    {
      NS_TEST_BOOL(values[i] == r2.Bool());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "DoubleZeroToOneExclusive")
  {
    nsRandom r;
    r.Initialize(0xDDEEFF0011AABBCCULL);

    for (nsInt32 i = 2; i < 10000; ++i)
    {
      const double val = r.DoubleZeroToOneExclusive();
      NS_TEST_BOOL(val >= 0.0);
      NS_TEST_BOOL(val < 1.0);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "DoubleZeroToOneInclusive")
  {
    nsRandom r;
    r.Initialize(0xEEFF0011AABBCCDDULL);

    for (nsInt32 i = 2; i < 10000; ++i)
    {
      const double val = r.DoubleZeroToOneInclusive();
      NS_TEST_BOOL(val >= 0.0);
      NS_TEST_BOOL(val <= 1.0);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "DoubleInRange")
  {
    nsRandom r;
    r.Initialize(0xFF0011AABBCCDDEEULL);

    NS_TEST_DOUBLE(r.DoubleInRange(5, 0), 5, 0.0);
    NS_TEST_DOUBLE(r.DoubleInRange(-5, 0), -5, 0.0);

    for (nsInt32 i = 2; i < 10000; ++i)
    {
      const double val = r.DoubleInRange(i, i);
      NS_TEST_BOOL(val >= i);
      NS_TEST_BOOL(val < i + i);
    }

    for (nsInt32 i = 2; i < 10000; ++i)
    {
      const double val = r.DoubleInRange(-i, 2 * i);
      NS_TEST_BOOL(val >= -i);
      NS_TEST_BOOL(val < -i + 2 * i);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "DoubleMinMax")
  {
    nsRandom r;
    r.Initialize(0x0011AABBCCDDEEFFULL);

    NS_TEST_DOUBLE(r.DoubleMinMax(5, 5), 5, 0.0);
    NS_TEST_DOUBLE(r.DoubleMinMax(-5, -5), -5, 0.0);

    for (nsInt32 i = 2; i < 10000; ++i)
    {
      const double val = r.DoubleMinMax(i, 2 * i);
      NS_TEST_BOOL(val >= i);
      NS_TEST_BOOL(val <= i + i);
    }

    for (nsInt32 i = 2; i < 10000; ++i)
    {
      const double val = r.DoubleMinMax(-i, i);
      NS_TEST_BOOL(val >= -i);
      NS_TEST_BOOL(val <= i);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "FloatZeroToOneExclusive")
  {
    nsRandom r;
    r.Initialize(0xDDEEFF0011AABBCCULL);

    for (nsInt32 i = 2; i < 10000; ++i)
    {
      const float val = r.FloatZeroToOneExclusive();
      NS_TEST_BOOL(val >= 0.f);
      NS_TEST_BOOL(val < 1.f);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "FloatZeroToOneInclusive")
  {
    nsRandom r;
    r.Initialize(0xEEFF0011AABBCCDDULL);

    for (nsInt32 i = 2; i < 10000; ++i)
    {
      const float val = r.FloatZeroToOneInclusive();
      NS_TEST_BOOL(val >= 0.f);
      NS_TEST_BOOL(val <= 1.f);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "FloatInRange")
  {
    nsRandom r;
    r.Initialize(0xFF0011AABBCCDDEEULL);

    NS_TEST_FLOAT(r.FloatInRange(5, 0), 5, 0.f);
    NS_TEST_FLOAT(r.FloatInRange(-5, 0), -5, 0.f);

    for (nsInt32 i = 2; i < 10000; ++i)
    {
      const float val = r.FloatInRange(static_cast<float>(i), static_cast<float>(i));
      NS_TEST_BOOL(val >= i);
      NS_TEST_BOOL(val < i + i);
    }

    for (nsInt32 i = 2; i < 10000; ++i)
    {
      const float val = r.FloatInRange(static_cast<float>(-i), 2 * static_cast<float>(i));
      NS_TEST_BOOL(val >= -i);
      NS_TEST_BOOL(val < -i + 2 * i);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "FloatMinMax")
  {
    nsRandom r;
    r.Initialize(0x0011AABBCCDDEEFFULL);

    NS_TEST_FLOAT(r.FloatMinMax(5, 5), 5, 0.f);
    NS_TEST_FLOAT(r.FloatMinMax(-5, -5), -5, 0.f);

    for (nsInt32 i = 2; i < 10000; ++i)
    {
      const float val = r.FloatMinMax(static_cast<float>(i), static_cast<float>(2 * i));
      NS_TEST_BOOL(val >= i);
      NS_TEST_BOOL(val <= i + i);
    }

    for (nsInt32 i = 2; i < 10000; ++i)
    {
      const float val = r.FloatMinMax(static_cast<float>(-i), static_cast<float>(i));
      NS_TEST_BOOL(val >= -i);
      NS_TEST_BOOL(val <= i);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Save / Load")
  {
    nsRandom r, r2;
    r.Initialize(0x0011AABBCCDDE11FULL);

    for (int i = 0; i < 1000; ++i)
      r.UInt();

    nsDefaultMemoryStreamStorage storage;
    nsMemoryStreamWriter writer(&storage);
    nsMemoryStreamReader reader(&storage);

    r.Save(writer);

    nsDynamicArray<nsUInt32> temp;
    temp.SetCountUninitialized(1000);

    for (int i = 0; i < 1000; ++i)
      temp[i] = r.UInt();

    r2.Load(reader);

    for (int i = 0; i < 1000; ++i)
    {
      NS_TEST_INT(temp[i], r2.UInt());
    }
  }
}

static void SaveToImage(nsDynamicArray<nsUInt32>& ref_values, nsUInt32 uiMaxValue, const char* szFile)
{
#ifdef USE_NSIMAGE
  NS_TEST_BOOL(nsFileSystem::AddDataDirectory("", nsFileSystem::AllowWrites, "Clear") == NS_SUCCESS);

  nsImage img;
  img.SetWidth(Values.GetCount());
  img.SetHeight(100);
  img.SetImageFormat(nsImageFormat::B8G8R8A8_UNORM);
  img.AllocateImageData();

  for (nsUInt32 y = 0; y < img.GetHeight(); ++y)
  {
    for (nsUInt32 x = 0; x < img.GetWidth(); ++x)
    {
      nsUInt32* pPixel = img.GetPixelPointer<nsUInt32>(0, 0, 0, x, y);
      *pPixel = 0xFF000000;
    }
  }

  for (nsUInt32 i = 0; i < Values.GetCount(); ++i)
  {
    double val = ((double)Values[i] / (double)uiMaxValue) * 100.0;
    nsUInt32 y = 99 - nsMath::Clamp<nsUInt32>((nsUInt32)val, 0, 99);

    nsUInt32* pPixel = img.GetPixelPointer<nsUInt32>(0, 0, 0, i, y);
    *pPixel = 0xFFFFFFFF;
  }

  img.SaveTo(szFile);

  nsFileSystem::RemoveDataDirectoryGroup("Clear");
#endif
}

NS_CREATE_SIMPLE_TEST(Math, RandomGauss)
{
  const float fVariance = 1.0f;

  NS_TEST_BLOCK(nsTestBlock::Enabled, "UnsignedValue")
  {
    nsRandomGauss r;
    r.Initialize(0xABCDEF0012345678ULL, 100, fVariance);

    nsDynamicArray<nsUInt32> Values;
    Values.SetCount(100);

    nsUInt32 uiMaxValue = 0;

    const nsUInt32 factor = 10; // with a factor of 100 the bell curve becomes more pronounced, with less samples it has more exceptions
    for (nsUInt32 i = 0; i < 10000 * factor; ++i)
    {
      auto val = r.UnsignedValue();

      NS_TEST_BOOL(val < 100);

      if (val < Values.GetCount())
      {
        Values[val]++;

        uiMaxValue = nsMath::Max(uiMaxValue, Values[val]);
      }
    }

    SaveToImage(Values, uiMaxValue, "D:/GaussUnsigned.tga");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SignedValue")
  {
    nsRandomGauss r;
    r.Initialize(0xABCDEF0012345678ULL, 100, fVariance);

    nsDynamicArray<nsUInt32> Values;
    Values.SetCount(2 * 100);

    nsUInt32 uiMaxValue = 0;

    const nsUInt32 factor = 10; // with a factor of 100 the bell curve becomes more pronounced, with less samples it has more exceptions
    for (nsUInt32 i = 0; i < 10000 * factor; ++i)
    {
      auto val = r.SignedValue();

      NS_TEST_BOOL(val > -100 && val < 100);

      val += 100;

      if (val < (nsInt32)Values.GetCount())
      {
        Values[val]++;

        uiMaxValue = nsMath::Max(uiMaxValue, Values[val]);
      }
    }

    SaveToImage(Values, uiMaxValue, "D:/GaussSigned.tga");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Save / Load")
  {
    nsRandomGauss r, r2;
    r.Initialize(0x0011AABBCCDDE11FULL, 1000, 1.7f);

    for (int i = 0; i < 1000; ++i)
      r.UnsignedValue();

    nsDefaultMemoryStreamStorage storage;
    nsMemoryStreamWriter writer(&storage);
    nsMemoryStreamReader reader(&storage);

    r.Save(writer);

    nsDynamicArray<nsUInt32> temp;
    temp.SetCountUninitialized(1000);

    for (int i = 0; i < 1000; ++i)
      temp[i] = r.UnsignedValue();

    r2.Load(reader);

    for (int i = 0; i < 1000; ++i)
    {
      NS_TEST_INT(temp[i], r2.UnsignedValue());
    }
  }
}
