#include <FoundationTest/FoundationTestPCH.h>


#include <Foundation/Configuration/Startup.h>
#include <Foundation/IO/FileSystem/DataDirTypeFolder.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/Memory/MemoryTracker.h>
#include <Texture/Image/Formats/BmpFileFormat.h>
#include <Texture/Image/Formats/DdsFileFormat.h>
#include <Texture/Image/Formats/ImageFileFormat.h>
#include <Texture/Image/Image.h>
#include <Texture/Image/ImageConversion.h>

static const nsImageFormat::Enum defaultFormat = nsImageFormat::R32G32B32A32_FLOAT;

class nsImageConversionTest : public nsTestBaseClass
{

public:
  virtual const char* GetTestName() const override { return "Image Conversion"; }

  virtual nsResult GetImage(nsImage& ref_img, const nsSubTestEntry& subTest, nsUInt32 uiImageNumber) override
  {
    ref_img.ResetAndMove(std::move(m_Image));
    return NS_SUCCESS;
  }

private:
  virtual void SetupSubTests() override
  {
    for (nsUInt32 i = 0; i < nsImageFormat::NUM_FORMATS; ++i)
    {
      nsImageFormat::Enum format = static_cast<nsImageFormat::Enum>(i);

      const char* name = nsImageFormat::GetName(format);
      NS_ASSERT_DEV(name != nullptr, "Missing format information for format {}", i);

      bool isEncodable = nsImageConversion::IsConvertible(defaultFormat, format);

      if (!isEncodable)
      {
        // If a format doesn't have an encoder, ignore
        continue;
      }

      AddSubTest(name, i);
    }
  }

  virtual nsTestAppRun RunSubTest(nsInt32 iIdentifier, nsUInt32 uiInvocationCount) override
  {
    nsImageFormat::Enum format = static_cast<nsImageFormat::Enum>(iIdentifier);

    bool isDecodable = nsImageConversion::IsConvertible(format, defaultFormat);

    if (!isDecodable)
    {
      NS_TEST_BOOL_MSG(false, "Format %s can be encoded from %s but not decoded - add a decoder for this format please", nsImageFormat::GetName(format), nsImageFormat::GetName(defaultFormat));

      return nsTestAppRun::Quit;
    }

    {
      nsHybridArray<nsImageConversion::ConversionPathNode, 16> decodingPath;
      nsUInt32 decodingPathScratchBuffers;
      nsImageConversion::BuildPath(format, defaultFormat, false, decodingPath, decodingPathScratchBuffers).IgnoreResult();

      // the [test] tag tells the test framework to output the log message in the GUI
      nsLog::Info("[test]Default decoding Path:");
      for (nsUInt32 i = 0; i < decodingPath.GetCount(); ++i)
      {
        nsLog::Info("[test]  {} -> {}", nsImageFormat::GetName(decodingPath[i].m_sourceFormat), nsImageFormat::GetName(decodingPath[i].m_targetFormat));
      }
    }

    {
      nsHybridArray<nsImageConversion::ConversionPathNode, 16> encodingPath;
      nsUInt32 encodingPathScratchBuffers;
      nsImageConversion::BuildPath(defaultFormat, format, false, encodingPath, encodingPathScratchBuffers).IgnoreResult();

      // the [test] tag tells the test framework to output the log message in the GUI
      nsLog::Info("[test]Default encoding Path:");
      for (nsUInt32 i = 0; i < encodingPath.GetCount(); ++i)
      {
        nsLog::Info("[test]  {} -> {}", nsImageFormat::GetName(encodingPath[i].m_sourceFormat), nsImageFormat::GetName(encodingPath[i].m_targetFormat));
      }
    }

    // Test LDR: Load, encode to target format, then do image comparison (which internally decodes to BGR8_UNORM again).
    // This visualizes quantization for low bit formats, block compression artifacts, or whether formats have fewer than 3 channels.
    {
      NS_TEST_BOOL(m_Image.LoadFrom("ImageConversions/reference.png").Succeeded());

      NS_TEST_BOOL(m_Image.Convert(format).Succeeded());

      NS_TEST_IMAGE(iIdentifier * 2, nsImageFormat::IsCompressed(format) ? 10 : 0);
    }

    // Test HDR: Load, decode to FLOAT32, stretch to [-range, range] and encode;
    // then decode to FLOAT32 again, bring back into LDR range and do image comparison.
    // If the format doesn't support negative values, the left half of the image will be black.
    // If the format doesn't support values with absolute value > 1, the image will appear clipped to fullbright.
    // Also, fill the first few rows in the top left with Infinity, -Infinity, and NaN, which should
    // show up as White, White, and Black, resp., in the comparison.
    {
      const float range = 8;

      NS_TEST_BOOL(m_Image.LoadFrom("ImageConversions/reference.png").Succeeded());

      NS_TEST_BOOL(m_Image.Convert(nsImageFormat::R32G32B32A32_FLOAT).Succeeded());

      const float posInf = +nsMath::Infinity<float>();
      const float negInf = -nsMath::Infinity<float>();
      const float NaN = nsMath::NaN<float>();

      for (nsUInt32 y = 0; y < m_Image.GetHeight(); ++y)
      {
        nsColor* pPixelPointer = m_Image.GetPixelPointer<nsColor>(0, 0, 0, 0, y);

        for (nsUInt32 x = 0; x < m_Image.GetWidth(); ++x)
        {
          // Fill with Inf or Nan resp. scale the image into positive and negative HDR range
          if (x < 30 && y < 10)
          {
            *pPixelPointer = nsColor(posInf, posInf, posInf, posInf);
          }
          else if (x < 30 && y < 20)
          {
            *pPixelPointer = nsColor(negInf, negInf, negInf, negInf);
          }
          else if (x < 30 && y < 30)
          {
            *pPixelPointer = nsColor(NaN, NaN, NaN, NaN);
          }
          else
          {
            float scale = (x / float(m_Image.GetWidth()) - 0.5f) * 2.0f * range;

            if (nsMath::Abs(scale) > 0.5)
            {
              *pPixelPointer *= scale;
            }
          }

          pPixelPointer++;
        }
      }

      NS_TEST_BOOL(m_Image.Convert(format).Succeeded());

      NS_TEST_BOOL(m_Image.Convert(nsImageFormat::R32G32B32A32_FLOAT).Succeeded());

      for (nsUInt32 y = 0; y < m_Image.GetHeight(); ++y)
      {
        nsColor* pPixelPointer = m_Image.GetPixelPointer<nsColor>(0, 0, 0, 0, y);

        for (nsUInt32 x = 0; x < m_Image.GetWidth(); ++x)
        {
          // Scale the image back into LDR range if possible
          if (x < 30 && y < 10)
          {
            // Leave pos inf as is - this should be clipped to 1 in the LDR conversion for img cmp
          }
          else if (x < 30 && y < 20)
          {
            // Flip neg inf to pos inf
            *pPixelPointer *= -1.0f;
          }
          else if (x < 30 && y < 30)
          {
            // Leave nan as is - this should be clipped to 0 in the LDR conversion for img cmp
          }
          else
          {
            float scale = (x / float(m_Image.GetWidth()) - 0.5f) * 2.0f * range;
            if (nsMath::Abs(scale) > 0.5)
            {
              *pPixelPointer /= scale;
            }
          }

          pPixelPointer++;
        }
      }

      NS_TEST_IMAGE(iIdentifier * 2 + 1, nsImageFormat::IsCompressed(format) ? 10 : 0);
    }

    return nsTestAppRun::Quit;
  }

  virtual nsResult InitializeTest() override
  {
    nsStartup::StartupCoreSystems();

    const nsStringBuilder sReadDir(">sdk/", nsTestFramework::GetInstance()->GetRelTestDataPath());

    if (nsFileSystem::AddDataDirectory(sReadDir.GetData(), "ImageConversionTest").Failed())
    {
      return NS_FAILURE;
    }

    nsFileSystem::AddDataDirectory(">nstest/", "ImageComparisonDataDir", "imgout", nsFileSystem::AllowWrites).IgnoreResult();

#if NS_ENABLED(NS_PLATFORM_LINUX)
    // On linux we use CPU based BC6 and BC7 compression, which sometimes gives slightly different results from the GPU compression on Windows.
    nsTestFramework::GetInstance()->SetImageReferenceOverrideFolderName("Images_Reference_Linux");
#endif

    return NS_SUCCESS;
  }

  virtual nsResult DeInitializeTest() override
  {
    nsFileSystem::RemoveDataDirectoryGroup("ImageConversionTest");
    nsFileSystem::RemoveDataDirectoryGroup("ImageComparisonDataDir");

    nsStartup::ShutdownCoreSystems();
    nsMemoryTracker::DumpMemoryLeaks();

    return NS_SUCCESS;
  }

  virtual nsResult InitializeSubTest(nsInt32 iIdentifier) override { return NS_SUCCESS; }

  virtual nsResult DeInitializeSubTest(nsInt32 iIdentifier) override { return NS_SUCCESS; }

  nsImage m_Image;
};

static nsImageConversionTest s_ImageConversionTest;
