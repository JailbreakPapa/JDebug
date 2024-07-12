#include <Texture/TexturePCH.h>

#include <Texture/TexConv/TexConvProcessor.h>

#include <Foundation/Profiling/Profiling.h>
#include <Texture/Image/Image.h>
#include <Texture/Image/ImageConversion.h>

struct FileSuffixToUsage
{
  const char* m_szSuffix = nullptr;
  const nsTexConvUsage::Enum m_Usage = nsTexConvUsage::Auto;
};

static FileSuffixToUsage suffixToUsageMap[] = {
  //
  {"_d", nsTexConvUsage::Color},          //
  {"diff", nsTexConvUsage::Color},        //
  {"diffuse", nsTexConvUsage::Color},     //
  {"albedo", nsTexConvUsage::Color},      //
  {"col", nsTexConvUsage::Color},         //
  {"color", nsTexConvUsage::Color},       //
  {"emissive", nsTexConvUsage::Color},    //
  {"emit", nsTexConvUsage::Color},        //

  {"_n", nsTexConvUsage::NormalMap},      //
  {"nrm", nsTexConvUsage::NormalMap},     //
  {"norm", nsTexConvUsage::NormalMap},    //
  {"normal", nsTexConvUsage::NormalMap},  //
  {"normals", nsTexConvUsage::NormalMap}, //

  {"_r", nsTexConvUsage::Linear},         //
  {"_rgh", nsTexConvUsage::Linear},       //
  {"_rough", nsTexConvUsage::Linear},     //
  {"roughness", nsTexConvUsage::Linear},  //

  {"_m", nsTexConvUsage::Linear},         //
  {"_met", nsTexConvUsage::Linear},       //
  {"_metal", nsTexConvUsage::Linear},     //
  {"metallic", nsTexConvUsage::Linear},   //

  {"_h", nsTexConvUsage::Linear},         //
  {"height", nsTexConvUsage::Linear},     //
  {"_disp", nsTexConvUsage::Linear},      //

  {"_ao", nsTexConvUsage::Linear},        //
  {"occlusion", nsTexConvUsage::Linear},  //

  {"_alpha", nsTexConvUsage::Linear},     //
};


static nsTexConvUsage::Enum DetectUsageFromFilename(nsStringView sFile)
{
  nsStringBuilder name = nsPathUtils::GetFileName(sFile);
  name.ToLower();

  for (nsUInt32 i = 0; i < NS_ARRAY_SIZE(suffixToUsageMap); ++i)
  {
    if (name.EndsWith_NoCase(suffixToUsageMap[i].m_szSuffix))
    {
      return suffixToUsageMap[i].m_Usage;
    }
  }

  return nsTexConvUsage::Auto;
}

static nsTexConvUsage::Enum DetectUsageFromImage(const nsImage& image)
{
  const nsImageHeader& header = image.GetHeader();
  const nsImageFormat::Enum format = header.GetImageFormat();

  if (header.GetDepth() > 1)
  {
    // unsupported
    return nsTexConvUsage::Auto;
  }

  if (nsImageFormat::IsSrgb(format))
  {
    // already sRGB so must be color
    return nsTexConvUsage::Color;
  }

  if (format == nsImageFormat::BC5_UNORM)
  {
    return nsTexConvUsage::NormalMap;
  }

  if (nsImageFormat::GetBitsPerChannel(format, nsImageFormatChannel::R) > 8 || format == nsImageFormat::BC6H_SF16 ||
      format == nsImageFormat::BC6H_UF16)
  {
    return nsTexConvUsage::Hdr;
  }

  if (nsImageFormat::GetNumChannels(format) <= 2)
  {
    return nsTexConvUsage::Linear;
  }

  const nsImage* pImgRGBA = &image;
  nsImage convertedRGBA;

  if (image.GetImageFormat() != nsImageFormat::R8G8B8A8_UNORM)
  {
    pImgRGBA = &convertedRGBA;
    if (nsImageConversion::Convert(image, convertedRGBA, nsImageFormat::R8G8B8A8_UNORM).Failed())
    {
      // cannot convert to RGBA -> maybe some weird lookup table format
      return nsTexConvUsage::Auto;
    }
  }

  // analyze the image content
  {
    nsUInt32 sr = 0;
    nsUInt32 sg = 0;
    nsUInt32 sb = 0;

    nsUInt32 uiExtremeNormals = 0;

    nsUInt32 uiNumPixels = header.GetWidth() * header.GetHeight();
    NS_ASSERT_DEBUG(uiNumPixels > 0, "Unexpected empty image");

    // Sample no more than 10000 pixels
    nsUInt32 uiStride = nsMath::Max(1U, uiNumPixels / 10000);
    uiNumPixels /= uiStride;

    const nsUInt8* pPixel = pImgRGBA->GetPixelPointer<nsUInt8>();

    for (nsUInt32 uiPixel = 0; uiPixel < uiNumPixels; ++uiPixel)
    {
      // definitely not a normal map, if any Z vector points that much backwards
      uiExtremeNormals += (pPixel[2] < 90) ? 1 : 0;

      sr += pPixel[0];
      sg += pPixel[1];
      sb += pPixel[2];

      pPixel += 4 * uiStride;
    }

    // the average color in the image
    sr /= uiNumPixels; // NOLINT: not a division by zero
    sg /= uiNumPixels; // NOLINT: not a division by zero
    sb /= uiNumPixels; // NOLINT: not a division by zero

    if (sb < 230 || sr < 128 - 60 || sr > 128 + 60 || sg < 128 - 60 || sg > 128 + 60)
    {
      // if the average color is not a proper hue of blue, it cannot be a normal map
      return nsTexConvUsage::Color;
    }

    if (uiExtremeNormals > uiNumPixels / 100)
    {
      // more than 1 percent of normals pointing backwards ? => probably not a normalmap
      return nsTexConvUsage::Color;
    }

    // it might just be a normal map, it does have the proper hue of blue
    return nsTexConvUsage::NormalMap;
  }
}

nsResult nsTexConvProcessor::AdjustUsage(nsStringView sFilename, const nsImage& srcImg, nsEnum<nsTexConvUsage>& inout_Usage)
{
  NS_PROFILE_SCOPE("AdjustUsage");

  if (inout_Usage == nsTexConvUsage::Auto)
  {
    inout_Usage = DetectUsageFromFilename(sFilename);
  }

  if (inout_Usage == nsTexConvUsage::Auto)
  {
    inout_Usage = DetectUsageFromImage(srcImg);
  }

  if (inout_Usage == nsTexConvUsage::Auto)
  {
    nsLog::Error("Failed to deduce target format.");
    return NS_FAILURE;
  }

  return NS_SUCCESS;
}
