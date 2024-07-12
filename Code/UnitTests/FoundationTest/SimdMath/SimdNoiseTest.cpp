#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/IO/FileSystem/DataDirTypeFolder.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/SimdMath/SimdNoise.h>
#include <Foundation/SimdMath/SimdRandom.h>
#include <Texture/Image/Image.h>

NS_CREATE_SIMPLE_TEST(SimdMath, SimdNoise)
{
  nsStringBuilder sReadDir(">sdk/", nsTestFramework::GetInstance()->GetRelTestDataPath());
  nsStringBuilder sWriteDir = nsTestFramework::GetInstance()->GetAbsOutputPath();

  NS_TEST_BOOL(nsFileSystem::AddDataDirectory(sReadDir, "SimdNoise") == NS_SUCCESS);
  NS_TEST_BOOL_MSG(nsFileSystem::AddDataDirectory(sWriteDir, "SimdNoise", "output", nsFileSystem::AllowWrites) == NS_SUCCESS,
    "Failed to mount data dir '%s'", sWriteDir.GetData());

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Perlin")
  {
    const nsUInt32 uiSize = 128;

    nsImageHeader imageHeader;
    imageHeader.SetWidth(uiSize);
    imageHeader.SetHeight(uiSize);
    imageHeader.SetImageFormat(nsImageFormat::R8G8B8A8_UNORM);

    nsImage image;
    image.ResetAndAlloc(imageHeader);

    nsSimdPerlinNoise perlin(12345);
    nsSimdVec4f xOffset(0, 1, 2, 3);
    nsSimdFloat scale(100);

    for (nsUInt32 uiNumOctaves = 1; uiNumOctaves <= 6; ++uiNumOctaves)
    {
      nsColorLinearUB* data = image.GetPixelPointer<nsColorLinearUB>();
      for (nsUInt32 y = 0; y < uiSize; ++y)
      {
        for (nsUInt32 x = 0; x < uiSize / 4; ++x)
        {
          nsSimdVec4f sX = (nsSimdVec4f(x * 4.0f) + xOffset) / scale;
          nsSimdVec4f sY = nsSimdVec4f(y * 1.0f) / scale;

          nsSimdVec4f noise = perlin.NoiseZeroToOne(sX, sY, nsSimdVec4f::MakeZero(), uiNumOctaves);
          float p[4];
          p[0] = noise.x();
          p[1] = noise.y();
          p[2] = noise.z();
          p[3] = noise.w();

          nsUInt32 uiPixelIndex = y * uiSize + x * 4;
          for (nsUInt32 i = 0; i < 4; ++i)
          {
            data[uiPixelIndex + i] = nsColor(p[i], p[i], p[i]);
          }
        }
      }

      nsStringBuilder sOutFile;
      sOutFile.SetFormat(":output/SimdNoise/result-perlin_{}.tga", uiNumOctaves);

      NS_TEST_BOOL(image.SaveTo(sOutFile).Succeeded());

      nsStringBuilder sInFile;
      sInFile.SetFormat("SimdNoise/perlin_{}.tga", uiNumOctaves);
      NS_TEST_BOOL_MSG(nsFileSystem::ExistsFile(sInFile), "Noise image file is missing: '%s'", sInFile.GetData());

      NS_TEST_FILES(sOutFile, sInFile, "");
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Random")
  {
    nsUInt32 histogram[256] = {};

    for (nsUInt32 i = 0; i < 10000; ++i)
    {
      nsSimdVec4u seed = nsSimdVec4u(i);
      nsSimdVec4f randomValues = nsSimdRandom::FloatMinMax(nsSimdVec4i(0, 1, 2, 3), nsSimdVec4f::MakeZero(), nsSimdVec4f(256.0f), seed);
      nsSimdVec4i randomValuesAsInt = nsSimdVec4i::Truncate(randomValues);

      ++histogram[randomValuesAsInt.x()];
      ++histogram[randomValuesAsInt.y()];
      ++histogram[randomValuesAsInt.z()];
      ++histogram[randomValuesAsInt.w()];

      randomValues = nsSimdRandom::FloatMinMax(nsSimdVec4i(32, 33, 34, 35), nsSimdVec4f::MakeZero(), nsSimdVec4f(256.0f), seed);
      randomValuesAsInt = nsSimdVec4i::Truncate(randomValues);

      ++histogram[randomValuesAsInt.x()];
      ++histogram[randomValuesAsInt.y()];
      ++histogram[randomValuesAsInt.z()];
      ++histogram[randomValuesAsInt.w()];
    }

    const char* szOutFile = ":output/SimdNoise/result-random.csv";
    {
      nsFileWriter fileWriter;
      NS_TEST_BOOL(fileWriter.Open(szOutFile).Succeeded());

      nsStringBuilder sLine;
      for (nsUInt32 i = 0; i < NS_ARRAY_SIZE(histogram); ++i)
      {
        sLine.SetFormat("{},\n", histogram[i]);
        fileWriter.WriteBytes(sLine.GetData(), sLine.GetElementCount()).IgnoreResult();
      }
    }

    const char* szInFile = "SimdNoise/random.csv";
    NS_TEST_BOOL_MSG(nsFileSystem::ExistsFile(szInFile), "Random histogram file is missing: '%s'", szInFile);

    NS_TEST_TEXT_FILES(szOutFile, szInFile, "");
  }

  nsFileSystem::RemoveDataDirectoryGroup("SimdNoise");
}
