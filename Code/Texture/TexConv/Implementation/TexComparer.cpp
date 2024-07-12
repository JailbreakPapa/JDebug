#include <Texture/TexturePCH.h>

#include <Foundation/Profiling/Profiling.h>
#include <Texture/Image/ImageConversion.h>
#include <Texture/Image/ImageUtils.h>
#include <Texture/TexConv/TexComparer.h>

nsTexComparer::nsTexComparer() = default;

nsResult nsTexComparer::Compare()
{
  NS_PROFILE_SCOPE("Compare");

  NS_SUCCEED_OR_RETURN(LoadInputImages());

  if ((m_Descriptor.m_ActualImage.GetWidth() != m_Descriptor.m_ExpectedImage.GetWidth()) ||
      (m_Descriptor.m_ActualImage.GetHeight() != m_Descriptor.m_ExpectedImage.GetHeight()))
  {
    nsLog::Error("Image sizes are not identical: {}x{} != {}x{}", m_Descriptor.m_ActualImage.GetWidth(), m_Descriptor.m_ActualImage.GetHeight(), m_Descriptor.m_ExpectedImage.GetWidth(), m_Descriptor.m_ExpectedImage.GetHeight());
    return NS_FAILURE;
  }

  NS_SUCCEED_OR_RETURN(ComputeMSE());

  if (m_OutputMSE > m_Descriptor.m_MeanSquareErrorThreshold)
  {
    m_bExceededMSE = true;

    NS_SUCCEED_OR_RETURN(ExtractImages());
  }

  return NS_SUCCESS;
}

nsResult nsTexComparer::LoadInputImages()
{
  NS_PROFILE_SCOPE("Load Images");

  if (!m_Descriptor.m_sActualFile.IsEmpty())
  {
    if (m_Descriptor.m_ActualImage.LoadFrom(m_Descriptor.m_sActualFile).Failed())
    {
      nsLog::Error("Could not load image file '{0}'.", nsArgSensitive(m_Descriptor.m_sActualFile, "File"));
      return NS_FAILURE;
    }
  }

  if (!m_Descriptor.m_sExpectedFile.IsEmpty())
  {
    if (m_Descriptor.m_ExpectedImage.LoadFrom(m_Descriptor.m_sExpectedFile).Failed())
    {
      nsLog::Error("Could not load reference file '{0}'.", nsArgSensitive(m_Descriptor.m_sExpectedFile, "File"));
      return NS_FAILURE;
    }
  }

  if (!m_Descriptor.m_ActualImage.IsValid())
  {
    nsLog::Error("No image available.");
    return NS_FAILURE;
  }

  if (!m_Descriptor.m_ExpectedImage.IsValid())
  {
    nsLog::Error("No reference image available.");
    return NS_FAILURE;
  }

  if (m_Descriptor.m_ActualImage.GetImageFormat() == nsImageFormat::UNKNOWN)
  {
    nsLog::Error("Unknown image format for '{}'", nsArgSensitive(m_Descriptor.m_sActualFile, "File"));
    return NS_FAILURE;
  }

  if (m_Descriptor.m_ExpectedImage.GetImageFormat() == nsImageFormat::UNKNOWN)
  {
    nsLog::Error("Unknown image format for '{}'", nsArgSensitive(m_Descriptor.m_sExpectedFile, "File"));
    return NS_FAILURE;
  }

  if (nsImageConversion::Convert(m_Descriptor.m_ActualImage, m_Descriptor.m_ActualImage, nsImageFormat::R8G8B8A8_UNORM).Failed())
  {
    nsLog::Error("Could not convert to RGBA8: '{}'", nsArgSensitive(m_Descriptor.m_sActualFile, "File"));
    return NS_FAILURE;
  }

  if (nsImageConversion::Convert(m_Descriptor.m_ExpectedImage, m_Descriptor.m_ExpectedImage, nsImageFormat::R8G8B8A8_UNORM).Failed())
  {
    nsLog::Error("Could not convert to RGBA8: '{}'", nsArgSensitive(m_Descriptor.m_sExpectedFile, "File"));
    return NS_FAILURE;
  }

  return NS_SUCCESS;
}

nsResult nsTexComparer::ComputeMSE()
{
  NS_PROFILE_SCOPE("ComputeMSE");

  if (m_Descriptor.m_bRelaxedComparison)
    nsImageUtils::ComputeImageDifferenceABSRelaxed(m_Descriptor.m_ActualImage, m_Descriptor.m_ExpectedImage, m_OutputImageDiff);
  else
    nsImageUtils::ComputeImageDifferenceABS(m_Descriptor.m_ActualImage, m_Descriptor.m_ExpectedImage, m_OutputImageDiff);

  m_OutputMSE = nsImageUtils::ComputeMeanSquareError(m_OutputImageDiff, 32);

  return NS_SUCCESS;
}

nsResult nsTexComparer::ExtractImages()
{
  NS_PROFILE_SCOPE("ExtractImages");

  nsImageUtils::Normalize(m_OutputImageDiff, m_uiOutputMinDiffRgb, m_uiOutputMaxDiffRgb, m_uiOutputMinDiffAlpha, m_uiOutputMaxDiffAlpha);

  NS_SUCCEED_OR_RETURN(nsImageConversion::Convert(m_OutputImageDiff, m_OutputImageDiffRgb, nsImageFormat::R8G8B8_UNORM));

  nsImageUtils::ExtractAlphaChannel(m_OutputImageDiff, m_OutputImageDiffAlpha);

  NS_SUCCEED_OR_RETURN(nsImageConversion::Convert(m_Descriptor.m_ActualImage, m_ExtractedActualRgb, nsImageFormat::R8G8B8_UNORM));
  nsImageUtils::ExtractAlphaChannel(m_Descriptor.m_ActualImage, m_ExtractedActualAlpha);

  NS_SUCCEED_OR_RETURN(nsImageConversion::Convert(m_Descriptor.m_ExpectedImage, m_ExtractedExpectedRgb, nsImageFormat::R8G8B8_UNORM));
  nsImageUtils::ExtractAlphaChannel(m_Descriptor.m_ExpectedImage, m_ExtractedExpectedAlpha);

  return NS_SUCCESS;
}
