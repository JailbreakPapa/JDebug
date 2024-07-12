#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/IO/FileSystem/DataDirTypeFolder.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Texture/Image/ImageUtils.h>


NS_CREATE_SIMPLE_TEST(Image, ImageUtils)
{
  nsStringBuilder sReadDir(">sdk/", nsTestFramework::GetInstance()->GetRelTestDataPath());
  nsStringBuilder sWriteDir = nsTestFramework::GetInstance()->GetAbsOutputPath();

  NS_TEST_BOOL(nsOSFile::CreateDirectoryStructure(sWriteDir.GetData()) == NS_SUCCESS);

  nsResult addDir = nsFileSystem::AddDataDirectory(sReadDir.GetData(), "ImageTest");
  NS_TEST_BOOL(addDir == NS_SUCCESS);

  if (addDir.Failed())
    return;

  addDir = nsFileSystem::AddDataDirectory(sWriteDir.GetData(), "ImageTest", "output", nsFileSystem::AllowWrites);
  NS_TEST_BOOL(addDir == NS_SUCCESS);

  if (addDir.Failed())
    return;

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ComputeImageDifferenceABS RGB")
  {
    nsImage ImageA, ImageB, ImageDiff;
    ImageA.LoadFrom("ImageUtils/ImageA_RGB.tga").IgnoreResult();
    ImageB.LoadFrom("ImageUtils/ImageB_RGB.tga").IgnoreResult();

    nsImageUtils::ComputeImageDifferenceABS(ImageA, ImageB, ImageDiff);

    ImageDiff.SaveTo(":output/ImageUtils/Diff_RGB.tga").IgnoreResult();

    NS_TEST_FILES("ImageUtils/ExpectedDiff_RGB.tga", "ImageUtils/Diff_RGB.tga", "");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ComputeImageDifferenceABS RGBA")
  {
    nsImage ImageA, ImageB, ImageDiff;
    ImageA.LoadFrom("ImageUtils/ImageA_RGBA.tga").IgnoreResult();
    ImageB.LoadFrom("ImageUtils/ImageB_RGBA.tga").IgnoreResult();

    nsImageUtils::ComputeImageDifferenceABS(ImageA, ImageB, ImageDiff);

    ImageDiff.SaveTo(":output/ImageUtils/Diff_RGBA.tga").IgnoreResult();

    NS_TEST_FILES("ImageUtils/ExpectedDiff_RGBA.tga", "ImageUtils/Diff_RGBA.tga", "");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Scaledown Half RGB")
  {
    nsImage ImageA, ImageAc;
    ImageA.LoadFrom("ImageUtils/ImageA_RGB.tga").IgnoreResult();
    nsImageUtils::Scale(ImageA, ImageAc, ImageA.GetWidth() / 2, ImageA.GetHeight() / 2).IgnoreResult();

    ImageAc.SaveTo(":output/ImageUtils/ScaledHalf_RGB.tga").IgnoreResult();

    NS_TEST_FILES("ImageUtils/ExpectedScaledHalf_RGB.tga", "ImageUtils/ScaledHalf_RGB.tga", "");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Scaledown Half RGBA")
  {
    nsImage ImageA, ImageAc;
    ImageA.LoadFrom("ImageUtils/ImageA_RGBA.tga").IgnoreResult();
    nsImageUtils::Scale(ImageA, ImageAc, ImageA.GetWidth() / 2, ImageA.GetHeight() / 2).IgnoreResult();

    ImageAc.SaveTo(":output/ImageUtils/ScaledHalf_RGBA.tga").IgnoreResult();

    NS_TEST_FILES("ImageUtils/ExpectedScaledHalf_RGBA.tga", "ImageUtils/ScaledHalf_RGBA.tga", "");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CropImage RGB")
  {
    nsImage ImageA, ImageAc;
    ImageA.LoadFrom("ImageUtils/ImageA_RGB.tga").IgnoreResult();
    nsImageUtils::CropImage(ImageA, nsVec2I32(100, 50), nsSizeU32(300, 200), ImageAc);

    ImageAc.SaveTo(":output/ImageUtils/Crop_RGB.tga").IgnoreResult();

    NS_TEST_FILES("ImageUtils/ExpectedCrop_RGB.tga", "ImageUtils/Crop_RGB.tga", "");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CropImage RGBA")
  {
    nsImage ImageA, ImageAc;
    ImageA.LoadFrom("ImageUtils/ImageA_RGBA.tga").IgnoreResult();
    nsImageUtils::CropImage(ImageA, nsVec2I32(100, 75), nsSizeU32(300, 180), ImageAc);

    ImageAc.SaveTo(":output/ImageUtils/Crop_RGBA.tga").IgnoreResult();

    NS_TEST_FILES("ImageUtils/ExpectedCrop_RGBA.tga", "ImageUtils/Crop_RGBA.tga", "");
  }


  NS_TEST_BLOCK(nsTestBlock::Enabled, "ComputeMeanSquareError")
  {
    nsImage ImageA, ImageB, ImageDiff;
    ImageA.LoadFrom("ImageUtils/ImageA_RGB.tga").IgnoreResult();
    ImageB.LoadFrom("ImageUtils/ImageB_RGB.tga").IgnoreResult();

    nsImage ImageAc, ImageBc;
    nsImageUtils::Scale(ImageA, ImageAc, ImageA.GetWidth() / 2, ImageA.GetHeight() / 2).IgnoreResult();
    nsImageUtils::Scale(ImageB, ImageBc, ImageB.GetWidth() / 2, ImageB.GetHeight() / 2).IgnoreResult();

    nsImageUtils::ComputeImageDifferenceABS(ImageAc, ImageBc, ImageDiff);

    ImageDiff.SaveTo(":output/ImageUtils/MeanSquareDiff_RGB.tga").IgnoreResult();

    NS_TEST_FILES("ImageUtils/ExpectedMeanSquareDiff_RGB.tga", "ImageUtils/MeanSquareDiff_RGB.tga", "");

    nsUInt32 uiError = nsImageUtils::ComputeMeanSquareError(ImageDiff, 4);
    NS_TEST_INT(uiError, 1433);
  }

  nsFileSystem::RemoveDataDirectoryGroup("ImageTest");
}
