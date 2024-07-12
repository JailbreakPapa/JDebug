#pragma once

/// \brief Input options for nsTexComparer
class NS_TEXTURE_DLL nsTexCompareDesc
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsTexCompareDesc);

public:
  nsTexCompareDesc() = default;

  /// Path to a file to load as a reference image. Optional, if m_ExpectedImage is already filled out.
  nsString m_sExpectedFile;

  /// Path to a file to load as the input image. Optional, if m_ActualImage is already filled out.
  nsString m_sActualFile;

  /// The reference image to compare. Ignored if m_sExpectedFile is filled out.
  nsImage m_ExpectedImage;

  /// The image to compare. Ignored if m_sActualFile is filled out.
  nsImage m_ActualImage;

  /// If enabled, the image comparison allows for more wiggle room.
  /// For images containing single-pixel rasterized lines.
  bool m_bRelaxedComparison = false;

  /// If the comparison yields a larger MSE than this, the images are considered to be too different.
  nsUInt32 m_MeanSquareErrorThreshold = 100;
};

/// \brief Compares two images and generates various outputs.
class NS_TEXTURE_DLL nsTexComparer
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsTexComparer);

public:
  nsTexComparer();

  /// The input data to compare.
  nsTexCompareDesc m_Descriptor;

  /// Executes the comparison and fill out the public variables to describe the result.
  nsResult Compare();

  /// If true, the mean-square error of the difference was larger than the threshold.
  bool m_bExceededMSE = false;
  /// The MSE of the difference image.
  nsUInt32 m_OutputMSE = 0;

  /// The (normalized) difference image.
  nsImage m_OutputImageDiff;
  /// Only the RGB part of the (normalized) difference image.
  nsImage m_OutputImageDiffRgb;
  /// Only the Alpha part of the (normalized) difference image.
  nsImage m_OutputImageDiffAlpha;

  /// Only the RGB part of the actual input image.
  nsImage m_ExtractedActualRgb;
  /// Only the RGB part of the reference input image.
  nsImage m_ExtractedExpectedRgb;
  /// Only the Alpha part of the actual input image.
  nsImage m_ExtractedActualAlpha;
  /// Only the Alpha part of the reference input image.
  nsImage m_ExtractedExpectedAlpha;

  /// Min/Max difference of the RGB and Alpha images.
  nsUInt8 m_uiOutputMinDiffRgb = 0;
  nsUInt8 m_uiOutputMaxDiffRgb = 0;
  nsUInt8 m_uiOutputMinDiffAlpha = 0;
  nsUInt8 m_uiOutputMaxDiffAlpha = 0;

private:
  nsResult LoadInputImages();
  nsResult ComputeMSE();
  nsResult ExtractImages();
};
