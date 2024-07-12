#include <Texture/TexturePCH.h>

#include <Foundation/IO/FileSystem/DeferredFileWriter.h>
#include <Texture/Image/Formats/DdsFileFormat.h>
#include <Texture/Image/ImageUtils.h>
#include <Texture/TexConv/TexConvProcessor.h>
#include <Texture/Utils/TextureAtlasDesc.h>
#include <Texture/Utils/TexturePacker.h>

nsResult nsTexConvProcessor::GenerateTextureAtlas(nsMemoryStreamWriter& stream)
{
  if (m_Descriptor.m_OutputType != nsTexConvOutputType::Atlas)
    return NS_SUCCESS;


  if (m_Descriptor.m_sTextureAtlasDescFile.IsEmpty())
  {
    nsLog::Error("Texture atlas description file is not specified.");
    return NS_FAILURE;
  }

  nsTextureAtlasCreationDesc atlasDesc;
  nsDynamicArray<TextureAtlasItem> atlasItems;

  if (atlasDesc.Load(m_Descriptor.m_sTextureAtlasDescFile).Failed())
  {
    nsLog::Error("Failed to load texture atlas description '{0}'", nsArgSensitive(m_Descriptor.m_sTextureAtlasDescFile, "File"));
    return NS_FAILURE;
  }

  m_Descriptor.m_uiMinResolution = nsMath::Max(32u, m_Descriptor.m_uiMinResolution);

  NS_SUCCEED_OR_RETURN(LoadAtlasInputs(atlasDesc, atlasItems));

  const nsUInt8 uiVersion = 3;
  stream << uiVersion;

  nsDdsFileFormat ddsWriter;
  nsImage atlasImg;

  for (nsUInt32 layerIdx = 0; layerIdx < atlasDesc.m_Layers.GetCount(); ++layerIdx)
  {
    NS_SUCCEED_OR_RETURN(CreateAtlasLayerTexture(atlasDesc, atlasItems, layerIdx, atlasImg));

    if (ddsWriter.WriteImage(stream, atlasImg, "dds").Failed())
    {
      nsLog::Error("Failed to write DDS image to texture atlas file.");
      return NS_FAILURE;
    }

    // debug: write out atlas slices as pure DDS
    if (false)
    {
      nsStringBuilder sOut;
      sOut.SetFormat("D:/atlas_{}.dds", layerIdx);

      nsFileWriter fOut;
      if (fOut.Open(sOut).Succeeded())
      {
        NS_SUCCEED_OR_RETURN(ddsWriter.WriteImage(fOut, atlasImg, "dds"));
      }
    }
  }

  NS_SUCCEED_OR_RETURN(WriteTextureAtlasInfo(atlasItems, atlasDesc.m_Layers.GetCount(), stream));

  return NS_SUCCESS;
}

nsResult nsTexConvProcessor::LoadAtlasInputs(const nsTextureAtlasCreationDesc& atlasDesc, nsDynamicArray<TextureAtlasItem>& items) const
{
  items.Clear();

  for (const auto& srcItem : atlasDesc.m_Items)
  {
    auto& item = items.ExpandAndGetRef();
    item.m_uiUniqueID = srcItem.m_uiUniqueID;
    item.m_uiFlags = srcItem.m_uiFlags;

    for (nsUInt32 layer = 0; layer < atlasDesc.m_Layers.GetCount(); ++layer)
    {
      if (!srcItem.m_sLayerInput[layer].IsEmpty())
      {
        if (item.m_InputImage[layer].LoadFrom(srcItem.m_sLayerInput[layer]).Failed())
        {
          nsLog::Error("Failed to load texture atlas texture '{0}'", nsArgSensitive(srcItem.m_sLayerInput[layer], "File"));
          return NS_FAILURE;
        }

        if (atlasDesc.m_Layers[layer].m_Usage == nsTexConvUsage::Color)
        {
          // enforce sRGB format for all color textures
          item.m_InputImage[layer].ReinterpretAs(nsImageFormat::AsSrgb(item.m_InputImage[layer].GetImageFormat()));
        }

        nsUInt32 uiResX = 0, uiResY = 0;
        NS_SUCCEED_OR_RETURN(DetermineTargetResolution(item.m_InputImage[layer], nsImageFormat::UNKNOWN, uiResX, uiResY));

        NS_SUCCEED_OR_RETURN(ConvertAndScaleImage(srcItem.m_sLayerInput[layer], item.m_InputImage[layer], uiResX, uiResY, atlasDesc.m_Layers[layer].m_Usage));
      }
    }


    if (!srcItem.m_sAlphaInput.IsEmpty())
    {
      nsImage alphaImg;

      if (alphaImg.LoadFrom(srcItem.m_sAlphaInput).Failed())
      {
        nsLog::Error("Failed to load texture atlas alpha mask '{0}'", srcItem.m_sAlphaInput);
        return NS_FAILURE;
      }

      nsUInt32 uiResX = 0, uiResY = 0;
      NS_SUCCEED_OR_RETURN(DetermineTargetResolution(alphaImg, nsImageFormat::UNKNOWN, uiResX, uiResY));

      NS_SUCCEED_OR_RETURN(ConvertAndScaleImage(srcItem.m_sAlphaInput, alphaImg, uiResX, uiResY, nsTexConvUsage::Linear));


      // layer 0 must have the exact same size as the alpha texture
      NS_SUCCEED_OR_RETURN(ConvertAndScaleImage(srcItem.m_sLayerInput[0], item.m_InputImage[0], uiResX, uiResY, nsTexConvUsage::Linear));

      // copy alpha channel into layer 0
      NS_SUCCEED_OR_RETURN(nsImageUtils::CopyChannel(item.m_InputImage[0], 3, alphaImg, 0));

      // rescale all layers to be no larger than the alpha mask texture
      for (nsUInt32 layer = 1; layer < atlasDesc.m_Layers.GetCount(); ++layer)
      {
        if (item.m_InputImage[layer].GetWidth() <= uiResX && item.m_InputImage[layer].GetHeight() <= uiResY)
          continue;

        NS_SUCCEED_OR_RETURN(ConvertAndScaleImage(srcItem.m_sLayerInput[layer], item.m_InputImage[layer], uiResX, uiResY, nsTexConvUsage::Linear));
      }
    }
  }

  return NS_SUCCESS;
}

nsResult nsTexConvProcessor::WriteTextureAtlasInfo(const nsDynamicArray<TextureAtlasItem>& atlasItems, nsUInt32 uiNumLayers, nsStreamWriter& stream)
{
  nsTextureAtlasRuntimeDesc runtimeAtlas;
  runtimeAtlas.m_uiNumLayers = uiNumLayers;

  runtimeAtlas.m_Items.Reserve(atlasItems.GetCount());

  for (const auto& item : atlasItems)
  {
    auto& e = runtimeAtlas.m_Items[item.m_uiUniqueID];
    e.m_uiFlags = item.m_uiFlags;

    for (nsUInt32 l = 0; l < uiNumLayers; ++l)
    {
      e.m_LayerRects[l] = item.m_AtlasRect[l];
    }
  }

  return runtimeAtlas.Serialize(stream);
}

constexpr nsUInt32 uiAtlasCellSize = 32;

nsResult nsTexConvProcessor::TrySortItemsIntoAtlas(nsDynamicArray<TextureAtlasItem>& items, nsUInt32 uiWidth, nsUInt32 uiHeight, nsInt32 layer)
{
  nsTexturePacker packer;

  // TODO: review, currently the texture packer only works on 32 sized cells
  nsUInt32 uiPixelAlign = uiAtlasCellSize;

  packer.SetTextureSize(uiWidth, uiHeight, items.GetCount() * 2);

  for (const auto& item : items)
  {
    if (item.m_InputImage[layer].IsValid())
    {
      packer.AddTexture((item.m_InputImage[layer].GetWidth() + (uiPixelAlign - 1)) / uiPixelAlign, (item.m_InputImage[layer].GetHeight() + (uiPixelAlign - 1)) / uiPixelAlign);
    }
  }

  NS_SUCCEED_OR_RETURN(packer.PackTextures());

  nsUInt32 uiTexIdx = 0;
  for (auto& item : items)
  {
    if (item.m_InputImage[layer].IsValid())
    {
      const auto& tex = packer.GetTextures()[uiTexIdx++];

      item.m_AtlasRect[layer].x = tex.m_Position.x * uiAtlasCellSize;
      item.m_AtlasRect[layer].y = tex.m_Position.y * uiAtlasCellSize;
      item.m_AtlasRect[layer].width = tex.m_Size.x * uiAtlasCellSize;
      item.m_AtlasRect[layer].height = tex.m_Size.y * uiAtlasCellSize;
    }
  }

  return NS_SUCCESS;
}

nsResult nsTexConvProcessor::SortItemsIntoAtlas(nsDynamicArray<TextureAtlasItem>& items, nsUInt32& out_ResX, nsUInt32& out_ResY, nsInt32 layer)
{
  for (nsUInt32 power = 8; power < 14; ++power)
  {
    const nsUInt32 halfRes = 1 << (power - 1);
    const nsUInt32 resolution = 1 << power;
    const nsUInt32 resDivCellSize = resolution / uiAtlasCellSize;
    const nsUInt32 halfResDivCellSize = halfRes / uiAtlasCellSize;

    if (TrySortItemsIntoAtlas(items, resDivCellSize, halfResDivCellSize, layer).Succeeded())
    {
      out_ResX = resolution;
      out_ResY = halfRes;
      return NS_SUCCESS;
    }

    if (TrySortItemsIntoAtlas(items, halfResDivCellSize, resDivCellSize, layer).Succeeded())
    {
      out_ResX = halfRes;
      out_ResY = resolution;
      return NS_SUCCESS;
    }

    if (TrySortItemsIntoAtlas(items, resDivCellSize, resDivCellSize, layer).Succeeded())
    {
      out_ResX = resolution;
      out_ResY = resolution;
      return NS_SUCCESS;
    }
  }

  nsLog::Error("Could not sort items into texture atlas. Too many too large textures.");
  return NS_FAILURE;
}

nsResult nsTexConvProcessor::CreateAtlasTexture(nsDynamicArray<TextureAtlasItem>& items, nsUInt32 uiResX, nsUInt32 uiResY, nsImage& atlas, nsInt32 layer)
{
  nsImageHeader imgHeader;
  imgHeader.SetWidth(uiResX);
  imgHeader.SetHeight(uiResY);
  imgHeader.SetImageFormat(nsImageFormat::R32G32B32A32_FLOAT);
  atlas.ResetAndAlloc(imgHeader);

  // make sure the target texture is filled with all black
  {
    auto pixelData = atlas.GetBlobPtr<nsUInt8>();
    nsMemoryUtils::ZeroFill(pixelData.GetPtr(), static_cast<size_t>(pixelData.GetCount()));
  }

  for (auto& item : items)
  {
    if (item.m_InputImage[layer].IsValid())
    {
      nsImage& itemImage = item.m_InputImage[layer];

      nsRectU32 r;
      r.x = 0;
      r.y = 0;
      r.width = itemImage.GetWidth();
      r.height = itemImage.GetHeight();

      NS_SUCCEED_OR_RETURN(nsImageUtils::Copy(itemImage, r, atlas, nsVec3U32(item.m_AtlasRect[layer].x, item.m_AtlasRect[layer].y, 0)));
    }
  }

  return NS_SUCCESS;
}

nsResult nsTexConvProcessor::FillAtlasBorders(nsDynamicArray<TextureAtlasItem>& items, nsImage& atlas, nsInt32 layer)
{
  const nsUInt32 uiBorderPixels = 2;

  const nsUInt32 uiNumMipmaps = atlas.GetHeader().GetNumMipLevels();
  for (nsUInt32 uiMipLevel = 0; uiMipLevel < uiNumMipmaps; ++uiMipLevel)
  {
    for (auto& item : items)
    {
      if (!item.m_InputImage[layer].IsValid())
        continue;

      nsRectU32& itemRect = item.m_AtlasRect[layer];
      const nsUInt32 uiRectX = itemRect.x >> uiMipLevel;
      const nsUInt32 uiRectY = itemRect.y >> uiMipLevel;
      const nsUInt32 uiWidth = nsMath::Max(1u, itemRect.width >> uiMipLevel);
      const nsUInt32 uiHeight = nsMath::Max(1u, itemRect.height >> uiMipLevel);

      // fill the border of the item rect with alpha 0 to prevent bleeding into other decals in the atlas
      if (uiWidth <= 2 * uiBorderPixels || uiHeight <= 2 * uiBorderPixels)
      {
        for (nsUInt32 y = 0; y < uiHeight; ++y)
        {
          for (nsUInt32 x = 0; x < uiWidth; ++x)
          {
            const nsUInt32 xClamped = nsMath::Min(uiRectX + x, atlas.GetWidth(uiMipLevel));
            const nsUInt32 yClamped = nsMath::Min(uiRectY + y, atlas.GetHeight(uiMipLevel));
            atlas.GetPixelPointer<nsColor>(uiMipLevel, 0, 0, xClamped, yClamped)->a = 0.0f;
          }
        }
      }
      else
      {
        for (nsUInt32 i = 0; i < uiBorderPixels; ++i)
        {
          for (nsUInt32 y = 0; y < uiHeight; ++y)
          {
            atlas.GetPixelPointer<nsColor>(uiMipLevel, 0, 0, uiRectX + i, uiRectY + y)->a = 0.0f;
            atlas.GetPixelPointer<nsColor>(uiMipLevel, 0, 0, uiRectX + uiWidth - 1 - i, uiRectY + y)->a = 0.0f;
          }

          for (nsUInt32 x = 0; x < uiWidth; ++x)
          {
            atlas.GetPixelPointer<nsColor>(uiMipLevel, 0, 0, uiRectX + x, uiRectY + i)->a = 0.0f;
            atlas.GetPixelPointer<nsColor>(uiMipLevel, 0, 0, uiRectX + x, uiRectY + uiHeight - 1 - i)->a = 0.0f;
          }
        }
      }
    }
  }

  return NS_SUCCESS;
}

nsResult nsTexConvProcessor::CreateAtlasLayerTexture(const nsTextureAtlasCreationDesc& atlasDesc, nsDynamicArray<TextureAtlasItem>& atlasItems, nsInt32 layer, nsImage& dstImg)
{
  nsUInt32 uiTexWidth, uiTexHeight;
  NS_SUCCEED_OR_RETURN(SortItemsIntoAtlas(atlasItems, uiTexWidth, uiTexHeight, layer));

  nsLog::Success("Required Resolution for Texture Atlas: {0} x {1}", uiTexWidth, uiTexHeight);

  nsImage atlasImg;
  NS_SUCCEED_OR_RETURN(CreateAtlasTexture(atlasItems, uiTexWidth, uiTexHeight, atlasImg, layer));

  nsUInt32 uiNumMipmaps = atlasImg.GetHeader().ComputeNumberOfMipMaps();
  NS_SUCCEED_OR_RETURN(GenerateMipmaps(atlasImg, uiNumMipmaps));

  if (atlasDesc.m_Layers[layer].m_uiNumChannels == 4)
  {
    NS_SUCCEED_OR_RETURN(FillAtlasBorders(atlasItems, atlasImg, layer));
  }

  nsEnum<nsImageFormat> OutputImageFormat;

  NS_SUCCEED_OR_RETURN(ChooseOutputFormat(OutputImageFormat, atlasDesc.m_Layers[layer].m_Usage, atlasDesc.m_Layers[layer].m_uiNumChannels));

  NS_SUCCEED_OR_RETURN(GenerateOutput(std::move(atlasImg), dstImg, OutputImageFormat));

  return NS_SUCCESS;
}
