#include <FoundationTest/FoundationTestPCH.h>


#include <Foundation/IO/FileSystem/DataDirTypeFolder.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Texture/Image/Formats/BmpFileFormat.h>
#include <Texture/Image/Formats/DdsFileFormat.h>
#include <Texture/Image/Formats/ImageFileFormat.h>
#include <Texture/Image/Image.h>
#include <Texture/Image/ImageConversion.h>
#include <Texture/Image/ImageUtils.h>

NS_CREATE_SIMPLE_TEST_GROUP(Image);

NS_CREATE_SIMPLE_TEST(Image, Image)
{
  const nsStringBuilder sReadDir(">sdk/", nsTestFramework::GetInstance()->GetRelTestDataPath());
  const nsStringBuilder sWriteDir = nsTestFramework::GetInstance()->GetAbsOutputPath();

  NS_TEST_BOOL(nsOSFile::CreateDirectoryStructure(sWriteDir) == NS_SUCCESS);

  NS_TEST_BOOL(nsFileSystem::AddDataDirectory(sReadDir, "ImageTest") == NS_SUCCESS);
  NS_TEST_BOOL(nsFileSystem::AddDataDirectory(sWriteDir, "ImageTest", "output", nsFileSystem::AllowWrites) == NS_SUCCESS);

  NS_TEST_BLOCK(nsTestBlock::Enabled, "BMP - Good")
  {
    const char* testImagesGood[] = {
      "BMPTestImages/good/pal1", "BMPTestImages/good/pal1bg", "BMPTestImages/good/pal1wb", "BMPTestImages/good/pal4", "BMPTestImages/good/pal4rle",
      "BMPTestImages/good/pal8", "BMPTestImages/good/pal8-0", "BMPTestImages/good/pal8nonsquare",
      /*"BMPTestImages/good/pal8os2",*/ "BMPTestImages/good/pal8rle",
      /*"BMPTestImages/good/pal8topdown",*/ "BMPTestImages/good/pal8v4", "BMPTestImages/good/pal8v5", "BMPTestImages/good/pal8w124",
      "BMPTestImages/good/pal8w125", "BMPTestImages/good/pal8w126", "BMPTestImages/good/rgb16", "BMPTestImages/good/rgb16-565pal",
      "BMPTestImages/good/rgb24", "BMPTestImages/good/rgb24pal", "BMPTestImages/good/rgb32", /*"BMPTestImages/good/rgb32bf"*/
    };

    for (int i = 0; i < NS_ARRAY_SIZE(testImagesGood); i++)
    {
      nsImage image;
      {
        nsStringBuilder fileName;
        fileName.SetFormat("{0}.bmp", testImagesGood[i]);

        NS_TEST_BOOL_MSG(nsFileSystem::ExistsFile(fileName), "Image file does not exist: '%s'", fileName.GetData());
        NS_TEST_BOOL_MSG(image.LoadFrom(fileName) == NS_SUCCESS, "Reading image failed: '%s'", fileName.GetData());
      }

      {
        nsStringBuilder fileName;
        fileName.SetFormat(":output/{0}_out.bmp", testImagesGood[i]);

        NS_TEST_BOOL_MSG(image.SaveTo(fileName) == NS_SUCCESS, "Writing image failed: '%s'", fileName.GetData());
        NS_TEST_BOOL_MSG(nsFileSystem::ExistsFile(fileName), "Output image file is missing: '%s'", fileName.GetData());
      }
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "BMP - Bad")
  {
    const char* testImagesBad[] = {"BMPTestImages/bad/badbitcount", "BMPTestImages/bad/badbitssize",
      /*"BMPTestImages/bad/baddens1", "BMPTestImages/bad/baddens2", "BMPTestImages/bad/badfilesize", "BMPTestImages/bad/badheadersize",*/
      "BMPTestImages/bad/badpalettesize",
      /*"BMPTestImages/bad/badplanes",*/ "BMPTestImages/bad/badrle", "BMPTestImages/bad/badwidth",
      /*"BMPTestImages/bad/pal2",*/ "BMPTestImages/bad/pal8badindex", "BMPTestImages/bad/reallybig", "BMPTestImages/bad/rletopdown",
      "BMPTestImages/bad/shortfile"};


    for (int i = 0; i < NS_ARRAY_SIZE(testImagesBad); i++)
    {
      nsImage image;
      {
        nsStringBuilder fileName;
        fileName.SetFormat("{0}.bmp", testImagesBad[i]);

        NS_TEST_BOOL_MSG(nsFileSystem::ExistsFile(fileName), "File does not exist: '%s'", fileName.GetData());

        NS_LOG_BLOCK_MUTE();
        NS_TEST_BOOL_MSG(image.LoadFrom(fileName) == NS_FAILURE, "Reading image should have failed: '%s'", fileName.GetData());
      }
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "TGA")
  {
    const char* testImagesGood[] = {"TGATestImages/good/RGB", "TGATestImages/good/RGBA", "TGATestImages/good/RGB_RLE", "TGATestImages/good/RGBA_RLE"};

    for (int i = 0; i < NS_ARRAY_SIZE(testImagesGood); i++)
    {
      nsImage image;
      {
        nsStringBuilder fileName;
        fileName.SetFormat("{0}.tga", testImagesGood[i]);

        NS_TEST_BOOL_MSG(nsFileSystem::ExistsFile(fileName), "Image file does not exist: '%s'", fileName.GetData());
        NS_TEST_BOOL_MSG(image.LoadFrom(fileName) == NS_SUCCESS, "Reading image failed: '%s'", fileName.GetData());
      }

      {
        nsStringBuilder fileName;
        fileName.SetFormat(":output/{0}_out.bmp", testImagesGood[i]);

        nsStringBuilder fileNameExpected;
        fileNameExpected.SetFormat("{0}_expected.bmp", testImagesGood[i]);

        NS_TEST_BOOL_MSG(image.SaveTo(fileName) == NS_SUCCESS, "Writing image failed: '%s'", fileName.GetData());
        NS_TEST_BOOL_MSG(nsFileSystem::ExistsFile(fileName), "Output image file is missing: '%s'", fileName.GetData());

        NS_TEST_FILES(fileName, fileNameExpected, "");
      }

      {
        nsStringBuilder fileName;
        fileName.SetFormat(":output/{0}_out.tga", testImagesGood[i]);

        nsStringBuilder fileNameExpected;
        fileNameExpected.SetFormat("{0}_expected.tga", testImagesGood[i]);

        NS_TEST_BOOL_MSG(image.SaveTo(fileName) == NS_SUCCESS, "Writing image failed: '%s'", fileName.GetData());
        NS_TEST_BOOL_MSG(nsFileSystem::ExistsFile(fileName), "Output image file is missing: '%s'", fileName.GetData());

        NS_TEST_FILES(fileName, fileNameExpected, "");
      }
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Write Image Formats")
  {
    struct ImgTest
    {
      const char* szImage;
      const char* szFormat;
      nsUInt32 uiMSE;
    };

    ImgTest imgTests[] = {
      {"RGB", "tga", 0},
      {"RGBA", "tga", 0},
      {"RGB", "png", 0},
      {"RGBA", "png", 0},
      {"RGB", "jpg", 4650},
      {"RGBA", "jpeg", 16670},
#if NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP)
      {"RGB", "tif", 0},
      {"RGBA", "tif", 0},
#endif
    };

    const char* szTestImagePath = "TGATestImages/good";

    for (int idx = 0; idx < NS_ARRAY_SIZE(imgTests); ++idx)
    {
      nsImage image;
      {
        nsStringBuilder fileName;
        fileName.SetFormat("{}/{}.tga", szTestImagePath, imgTests[idx].szImage);

        NS_TEST_BOOL_MSG(nsFileSystem::ExistsFile(fileName), "Image file does not exist: '%s'", fileName.GetData());
        NS_TEST_BOOL_MSG(image.LoadFrom(fileName) == NS_SUCCESS, "Reading image failed: '%s'", fileName.GetData());
      }

      {
        nsStringBuilder fileName;
        fileName.SetFormat(":output/WriteImageTest/{}.{}", imgTests[idx].szImage, imgTests[idx].szFormat);

        nsFileSystem::DeleteFile(fileName);

        NS_TEST_BOOL_MSG(image.SaveTo(fileName) == NS_SUCCESS, "Writing image failed: '%s'", fileName.GetData());
        NS_TEST_BOOL_MSG(nsFileSystem::ExistsFile(fileName), "Output image file is missing: '%s'", fileName.GetData());

        nsImage image2;
        NS_TEST_BOOL_MSG(image2.LoadFrom(fileName).Succeeded(), "Reading written image failed: '%s'", fileName.GetData());

        image.Convert(nsImageFormat::R8G8B8A8_UNORM_SRGB).IgnoreResult();
        image2.Convert(nsImageFormat::R8G8B8A8_UNORM_SRGB).IgnoreResult();

        nsImage diff;
        nsImageUtils::ComputeImageDifferenceABS(image, image2, diff);

        const nsUInt32 uiMSE = nsImageUtils::ComputeMeanSquareError(diff, 32);

        NS_TEST_BOOL_MSG(uiMSE <= imgTests[idx].uiMSE, "MSE %u is larger than %u for image '%s'", uiMSE, imgTests[idx].uiMSE, fileName.GetData());
      }
    }
  }

  nsFileSystem::RemoveDataDirectoryGroup("ImageTest");
}
