#include <Texture/TexturePCH.h>

#include <Foundation/Profiling/Profiling.h>
#include <Texture/Image/ImageUtils.h>
#include <Texture/TexConv/TexConvProcessor.h>

nsResult nsTexConvProcessor::LoadInputImages()
{
  NS_PROFILE_SCOPE("Load Images");

  if (m_Descriptor.m_InputImages.IsEmpty() && m_Descriptor.m_InputFiles.IsEmpty())
  {
    nsLog::Error("No input images have been specified.");
    return NS_FAILURE;
  }

  if (!m_Descriptor.m_InputImages.IsEmpty() && !m_Descriptor.m_InputFiles.IsEmpty())
  {
    nsLog::Error("Both input files and input images have been specified. You need to either specify files or images.");
    return NS_FAILURE;
  }

  if (!m_Descriptor.m_InputImages.IsEmpty())
  {
    // make sure the two arrays have the same size
    m_Descriptor.m_InputFiles.SetCount(m_Descriptor.m_InputImages.GetCount());

    nsStringBuilder tmp;
    for (nsUInt32 i = 0; i < m_Descriptor.m_InputFiles.GetCount(); ++i)
    {
      tmp.SetFormat("InputImage{}", nsArgI(i, 2, true));
      m_Descriptor.m_InputFiles[i] = tmp;
    }
  }
  else
  {
    m_Descriptor.m_InputImages.Reserve(m_Descriptor.m_InputFiles.GetCount());

    for (const auto& file : m_Descriptor.m_InputFiles)
    {
      auto& img = m_Descriptor.m_InputImages.ExpandAndGetRef();
      if (img.LoadFrom(file).Failed())
      {
        nsLog::Error("Could not load input file '{0}'.", nsArgSensitive(file, "File"));
        return NS_FAILURE;
      }
    }
  }

  for (nsUInt32 i = 0; i < m_Descriptor.m_InputFiles.GetCount(); ++i)
  {
    const auto& img = m_Descriptor.m_InputImages[i];

    if (img.GetImageFormat() == nsImageFormat::UNKNOWN)
    {
      nsLog::Error("Unknown image format for '{}'", nsArgSensitive(m_Descriptor.m_InputFiles[i], "File"));
      return NS_FAILURE;
    }
  }

  return NS_SUCCESS;
}

nsResult nsTexConvProcessor::ConvertAndScaleImage(nsStringView sImageName, nsImage& inout_Image, nsUInt32 uiResolutionX, nsUInt32 uiResolutionY, nsEnum<nsTexConvUsage> usage)
{
  const bool bSingleChannel = nsImageFormat::GetNumChannels(inout_Image.GetImageFormat()) == 1;

  if (inout_Image.Convert(nsImageFormat::R32G32B32A32_FLOAT).Failed())
  {
    nsLog::Error("Could not convert '{}' to RGBA 32-Bit Float format.", sImageName);
    return NS_FAILURE;
  }

  // some scale operations fail when they are done in place, so use a scratch image as destination for now
  nsImage scratch;
  if (nsImageUtils::Scale(inout_Image, scratch, uiResolutionX, uiResolutionY, nullptr, nsImageAddressMode::Clamp, nsImageAddressMode::Clamp).Failed())
  {
    nsLog::Error("Could not resize '{}' to {}x{}", sImageName, uiResolutionX, uiResolutionY);
    return NS_FAILURE;
  }

  inout_Image.ResetAndMove(std::move(scratch));

  if (usage == nsTexConvUsage::Color && bSingleChannel)
  {
    // replicate single channel ("red" textures) into the other channels
    NS_SUCCEED_OR_RETURN(nsImageUtils::CopyChannel(inout_Image, 1, inout_Image, 0));
    NS_SUCCEED_OR_RETURN(nsImageUtils::CopyChannel(inout_Image, 2, inout_Image, 0));
  }

  return NS_SUCCESS;
}

nsResult nsTexConvProcessor::ConvertAndScaleInputImages(nsUInt32 uiResolutionX, nsUInt32 uiResolutionY, nsEnum<nsTexConvUsage> usage)
{
  NS_PROFILE_SCOPE("ConvertAndScaleInputImages");

  for (nsUInt32 idx = 0; idx < m_Descriptor.m_InputImages.GetCount(); ++idx)
  {
    auto& img = m_Descriptor.m_InputImages[idx];
    nsStringView sName = m_Descriptor.m_InputFiles[idx];

    NS_SUCCEED_OR_RETURN(ConvertAndScaleImage(sName, img, uiResolutionX, uiResolutionY, usage));
  }

  return NS_SUCCESS;
}
