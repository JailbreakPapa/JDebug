#pragma once

#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Math/Rect.h>
#include <Texture/TexConv/TexConvDesc.h>

struct nsTextureAtlasCreationDesc;

class NS_TEXTURE_DLL nsTexConvProcessor
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsTexConvProcessor);

public:
  nsTexConvProcessor();

  nsTexConvDesc m_Descriptor;

  nsResult Process();

  nsImage m_OutputImage;
  nsImage m_LowResOutputImage;
  nsImage m_ThumbnailOutputImage;
  nsDefaultMemoryStreamStorage m_TextureAtlas;

private:
  //////////////////////////////////////////////////////////////////////////
  // Modifying the Descriptor

  nsResult LoadInputImages();
  nsResult ForceSRGBFormats();
  nsResult ConvertAndScaleInputImages(nsUInt32 uiResolutionX, nsUInt32 uiResolutionY, nsEnum<nsTexConvUsage> usage);
  nsResult ConvertToNormalMap(nsImage& bumpMap) const;
  nsResult ConvertToNormalMap(nsArrayPtr<nsImage> bumpMap) const;
  nsResult ClampInputValues(nsArrayPtr<nsImage> images, float maxValue) const;
  nsResult ClampInputValues(nsImage& image, float maxValue) const;
  nsResult DetectNumChannels(nsArrayPtr<const nsTexConvSliceChannelMapping> channelMapping, nsUInt32& uiNumChannels);
  nsResult InvertNormalMap(nsImage& img);

  //////////////////////////////////////////////////////////////////////////
  // Reading from the descriptor

  enum class MipmapChannelMode
  {
    AllChannels,
    SingleChannel
  };

  nsResult ChooseOutputFormat(nsEnum<nsImageFormat>& out_Format, nsEnum<nsTexConvUsage> usage, nsUInt32 uiNumChannels) const;
  nsResult DetermineTargetResolution(
    const nsImage& image, nsEnum<nsImageFormat> OutputImageFormat, nsUInt32& out_uiTargetResolutionX, nsUInt32& out_uiTargetResolutionY) const;
  nsResult Assemble2DTexture(const nsImageHeader& refImg, nsImage& dst) const;
  nsResult AssembleCubemap(nsImage& dst) const;
  nsResult Assemble3DTexture(nsImage& dst) const;
  nsResult AdjustHdrExposure(nsImage& img) const;
  nsResult PremultiplyAlpha(nsImage& image) const;
  nsResult DilateColor2D(nsImage& img) const;
  nsResult Assemble2DSlice(const nsTexConvSliceChannelMapping& mapping, nsUInt32 uiResolutionX, nsUInt32 uiResolutionY, nsColor* pPixelOut) const;
  nsResult GenerateMipmaps(nsImage& img, nsUInt32 uiNumMips /* =0 */, MipmapChannelMode channelMode = MipmapChannelMode::AllChannels) const;

  //////////////////////////////////////////////////////////////////////////
  // Purely functional
  static nsResult AdjustUsage(nsStringView sFilename, const nsImage& srcImg, nsEnum<nsTexConvUsage>& inout_Usage);
  static nsResult ConvertAndScaleImage(nsStringView sImageName, nsImage& inout_Image, nsUInt32 uiResolutionX, nsUInt32 uiResolutionY, nsEnum<nsTexConvUsage> usage);

  //////////////////////////////////////////////////////////////////////////
  // Output Generation

  static nsResult GenerateOutput(nsImage&& src, nsImage& dst, nsEnum<nsImageFormat> format);
  static nsResult GenerateThumbnailOutput(const nsImage& srcImg, nsImage& dstImg, nsUInt32 uiTargetRes);
  static nsResult GenerateLowResOutput(const nsImage& srcImg, nsImage& dstImg, nsUInt32 uiLowResMip);

  //////////////////////////////////////////////////////////////////////////
  // Texture Atlas

  struct TextureAtlasItem
  {
    nsUInt32 m_uiUniqueID = 0;
    nsUInt32 m_uiFlags = 0;
    nsImage m_InputImage[4];
    nsRectU32 m_AtlasRect[4];
  };

  nsResult LoadAtlasInputs(const nsTextureAtlasCreationDesc& atlasDesc, nsDynamicArray<TextureAtlasItem>& items) const;
  nsResult CreateAtlasLayerTexture(
    const nsTextureAtlasCreationDesc& atlasDesc, nsDynamicArray<TextureAtlasItem>& atlasItems, nsInt32 layer, nsImage& dstImg);

  static nsResult WriteTextureAtlasInfo(const nsDynamicArray<TextureAtlasItem>& atlasItems, nsUInt32 uiNumLayers, nsStreamWriter& stream);
  static nsResult TrySortItemsIntoAtlas(nsDynamicArray<TextureAtlasItem>& items, nsUInt32 uiWidth, nsUInt32 uiHeight, nsInt32 layer);
  static nsResult SortItemsIntoAtlas(nsDynamicArray<TextureAtlasItem>& items, nsUInt32& out_ResX, nsUInt32& out_ResY, nsInt32 layer);
  static nsResult CreateAtlasTexture(nsDynamicArray<TextureAtlasItem>& items, nsUInt32 uiResX, nsUInt32 uiResY, nsImage& atlas, nsInt32 layer);
  static nsResult FillAtlasBorders(nsDynamicArray<TextureAtlasItem>& items, nsImage& atlas, nsInt32 layer);

  //////////////////////////////////////////////////////////////////////////
  // Texture Atlas

  nsResult GenerateTextureAtlas(nsMemoryStreamWriter& stream);
};
