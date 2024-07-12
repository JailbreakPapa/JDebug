#pragma once

#include <Foundation/Containers/StaticArray.h>
#include <Foundation/Types/Bitflags.h>
#include <Foundation/Utilities/EnumerableClass.h>

#include <Texture/Image/Image.h>

NS_DECLARE_FLAGS(nsUInt8, nsImageConversionFlags, InPlace);

/// A structure describing the pairs of source/target format that may be converted using the conversion routine.
struct nsImageConversionEntry
{
  nsImageConversionEntry(nsImageFormat::Enum source, nsImageFormat::Enum target, nsImageConversionFlags::Enum flags)
    : m_sourceFormat(source)
    , m_targetFormat(target)
    , m_flags(flags)
  {
  }

  const nsImageFormat::Enum m_sourceFormat;
  const nsImageFormat::Enum m_targetFormat;
  const nsBitflags<nsImageConversionFlags> m_flags;

  /// This member adds an additional amount to the cost estimate for this conversion step.
  /// It can be used to bias the choice between steps when there are comparable conversion
  /// steps available.
  float m_additionalPenalty = 0.0f;
};

/// \brief Interface for a single image conversion step.
///
/// The actual functionality is implemented as either nsImageConversionStepLinear or nsImageConversionStepDecompressBlocks.
/// Depending on the types on conversion advertised by GetSupportedConversions(), users of this class need to cast it to a derived type
/// first to access the desired functionality.
class NS_TEXTURE_DLL nsImageConversionStep : public nsEnumerable<nsImageConversionStep>
{
  NS_DECLARE_ENUMERABLE_CLASS(nsImageConversionStep);

protected:
  nsImageConversionStep();
  virtual ~nsImageConversionStep();

public:
  /// \brief Returns an array pointer of supported conversions.
  ///
  /// \note The returned array must have the same entries each time this method is called.
  virtual nsArrayPtr<const nsImageConversionEntry> GetSupportedConversions() const = 0;
};

/// \brief Interface for a single image conversion step where both the source and target format are uncompressed.
class NS_TEXTURE_DLL nsImageConversionStepLinear : public nsImageConversionStep
{
public:
  /// \brief Converts a batch of pixels.
  virtual nsResult ConvertPixels(nsConstByteBlobPtr source, nsByteBlobPtr target, nsUInt64 uiNumElements, nsImageFormat::Enum sourceFormat,
    nsImageFormat::Enum targetFormat) const = 0;
};

/// \brief Interface for a single image conversion step where the source format is compressed and the target format is uncompressed.
class NS_TEXTURE_DLL nsImageConversionStepDecompressBlocks : public nsImageConversionStep
{
public:
  /// \brief Decompresses the given number of blocks.
  virtual nsResult DecompressBlocks(nsConstByteBlobPtr source, nsByteBlobPtr target, nsUInt32 uiNumBlocks, nsImageFormat::Enum sourceFormat,
    nsImageFormat::Enum targetFormat) const = 0;
};

/// \brief Interface for a single image conversion step where the source format is uncompressed and the target format is compressed.
class NS_TEXTURE_DLL nsImageConversionStepCompressBlocks : public nsImageConversionStep
{
public:
  /// \brief Compresses the given number of blocks.
  virtual nsResult CompressBlocks(nsConstByteBlobPtr source, nsByteBlobPtr target, nsUInt32 uiNumBlocksX, nsUInt32 uiNumBlocksY,
    nsImageFormat::Enum sourceFormat, nsImageFormat::Enum targetFormat) const = 0;
};

/// \brief Interface for a single image conversion step from a linear to a planar format.
class NS_TEXTURE_DLL nsImageConversionStepPlanarize : public nsImageConversionStep
{
public:
  /// \brief Converts a batch of pixels into the given target planes.
  virtual nsResult ConvertPixels(const nsImageView& source, nsArrayPtr<nsImage> target, nsUInt32 uiNumPixelsX, nsUInt32 uiNumPixelsY, nsImageFormat::Enum sourceFormat,
    nsImageFormat::Enum targetFormat) const = 0;
};

/// \brief Interface for a single image conversion step from a planar to a linear format.
class NS_TEXTURE_DLL nsImageConversionStepDeplanarize : public nsImageConversionStep
{
public:
  /// \brief Converts a batch of pixels from the given source planes.
  virtual nsResult ConvertPixels(nsArrayPtr<nsImageView> source, nsImage target, nsUInt32 uiNumPixelsX, nsUInt32 uiNumPixelsY, nsImageFormat::Enum sourceFormat,
    nsImageFormat::Enum targetFormat) const = 0;
};


/// \brief Helper class containing utilities to convert between different image formats and layouts.
class NS_TEXTURE_DLL nsImageConversion
{
public:
  /// \brief Checks if there is a known conversion path between the two formats
  static bool IsConvertible(nsImageFormat::Enum sourceFormat, nsImageFormat::Enum targetFormat);

  /// \brief Finds the image format from a given list of formats which is the cheapest to convert to.
  static nsImageFormat::Enum FindClosestCompatibleFormat(nsImageFormat::Enum format, nsArrayPtr<const nsImageFormat::Enum> compatibleFormats);

  /// \brief A single node along a computed conversion path.
  struct ConversionPathNode
  {
    NS_DECLARE_POD_TYPE();

    const nsImageConversionStep* m_step;
    nsImageFormat::Enum m_sourceFormat;
    nsImageFormat::Enum m_targetFormat;
    nsUInt32 m_sourceBufferIndex;
    nsUInt32 m_targetBufferIndex;
    bool m_inPlace;
  };

  /// \brief Precomputes an optimal conversion path between two formats and the minimal number of required scratch buffers.
  ///
  /// The generated path can be cached by the user if the same conversion is performed multiple times. The path must not be reused if the
  /// set of supported conversions changes, e.g. when plugins are loaded or unloaded.
  ///
  /// \param sourceFormat           The source format.
  /// \param targetFormat           The target format.
  /// \param sourceEqualsTarget     If true, the generated path is applicable if source and target memory regions are equal, and may contain
  /// additional copy-steps if the conversion can't be performed in-place.
  ///                               A path generated with sourceEqualsTarget == true will work correctly even if source and target are not
  ///                               the same, but may not be optimal. A path generated with sourceEqualsTarget == false will not work
  ///                               correctly when source and target are the same.
  /// \param path_out               The generated path.
  /// \param numScratchBuffers_out The number of scratch buffers required for the conversion path.
  /// \returns                      ns_SUCCESS if a path was found, ns_FAILURE otherwise.
  static nsResult BuildPath(nsImageFormat::Enum sourceFormat, nsImageFormat::Enum targetFormat, bool bSourceEqualsTarget,
    nsHybridArray<ConversionPathNode, 16>& ref_path_out, nsUInt32& ref_uiNumScratchBuffers_out);

  /// \brief  Converts the source image into a target image with the given format. Source and target may be the same.
  static nsResult Convert(const nsImageView& source, nsImage& ref_target, nsImageFormat::Enum targetFormat);

  /// \brief Converts the source image into a target image using a precomputed conversion path.
  static nsResult Convert(const nsImageView& source, nsImage& ref_target, nsArrayPtr<ConversionPathNode> path, nsUInt32 uiNumScratchBuffers);

  /// \brief Converts the raw source data into a target data buffer with the given format. Source and target may be the same.
  static nsResult ConvertRaw(
    nsConstByteBlobPtr source, nsByteBlobPtr target, nsUInt32 uiNumElements, nsImageFormat::Enum sourceFormat, nsImageFormat::Enum targetFormat);

  /// \brief Converts the raw source data into a target data buffer using a precomputed conversion path.
  static nsResult ConvertRaw(
    nsConstByteBlobPtr source, nsByteBlobPtr target, nsUInt32 uiNumElements, nsArrayPtr<ConversionPathNode> path, nsUInt32 uiNumScratchBuffers);

private:
  nsImageConversion();
  nsImageConversion(const nsImageConversion&);

  static nsResult ConvertSingleStep(const nsImageConversionStep* pStep, const nsImageView& source, nsImage& target, nsImageFormat::Enum targetFormat);

  static nsResult ConvertSingleStepDecompress(const nsImageView& source, nsImage& target, nsImageFormat::Enum sourceFormat,
    nsImageFormat::Enum targetFormat, const nsImageConversionStep* pStep);

  static nsResult ConvertSingleStepCompress(const nsImageView& source, nsImage& target, nsImageFormat::Enum sourceFormat,
    nsImageFormat::Enum targetFormat, const nsImageConversionStep* pStep);

  static nsResult ConvertSingleStepDeplanarize(const nsImageView& source, nsImage& target, nsImageFormat::Enum sourceFormat,
    nsImageFormat::Enum targetFormat, const nsImageConversionStep* pStep);

  static nsResult ConvertSingleStepPlanarize(const nsImageView& source, nsImage& target, nsImageFormat::Enum sourceFormat,
    nsImageFormat::Enum targetFormat, const nsImageConversionStep* pStep);

  static void RebuildConversionTable();
};
