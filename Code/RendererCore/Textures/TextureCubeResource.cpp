#include <RendererCore/RendererCorePCH.h>

#include <Foundation/Utilities/AssetFileHeader.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererCore/Textures/TextureCubeResource.h>
#include <RendererCore/Textures/TextureUtils.h>
#include <RendererFoundation/Resources/Texture.h>
#include <Texture/Image/Formats/DdsFileFormat.h>
#include <Texture/nsTexFormat/nsTexFormat.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsTextureCubeResource, 1, nsRTTIDefaultAllocator<nsTextureCubeResource>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_RESOURCE_IMPLEMENT_COMMON_CODE(nsTextureCubeResource);
// clang-format on

nsTextureCubeResource::nsTextureCubeResource()
  : nsResource(DoUpdate::OnAnyThread, nsTextureUtils::s_bForceFullQualityAlways ? 1 : 2)
{
  m_uiLoadedTextures = 0;
  m_uiMemoryGPU[0] = 0;
  m_uiMemoryGPU[1] = 0;
  m_Format = nsGALResourceFormat::Invalid;
  m_uiWidthAndHeight = 0;
}

nsResourceLoadDesc nsTextureCubeResource::UnloadData(Unload WhatToUnload)
{
  if (m_uiLoadedTextures > 0)
  {
    for (nsInt32 r = 0; r < 2; ++r)
    {
      --m_uiLoadedTextures;

      if (!m_hGALTexture[m_uiLoadedTextures].IsInvalidated())
      {
        nsGALDevice::GetDefaultDevice()->DestroyTexture(m_hGALTexture[m_uiLoadedTextures]);
        m_hGALTexture[m_uiLoadedTextures].Invalidate();
      }

      m_uiMemoryGPU[m_uiLoadedTextures] = 0;

      if (WhatToUnload == Unload::OneQualityLevel || m_uiLoadedTextures == 0)
        break;
    }
  }

  if (WhatToUnload == Unload::AllQualityLevels)
  {
    if (!m_hSamplerState.IsInvalidated())
    {
      nsGALDevice::GetDefaultDevice()->DestroySamplerState(m_hSamplerState);
      m_hSamplerState.Invalidate();
    }
  }

  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = m_uiLoadedTextures;
  res.m_uiQualityLevelsLoadable = 2 - m_uiLoadedTextures;
  res.m_State = m_uiLoadedTextures == 0 ? nsResourceState::Unloaded : nsResourceState::Loaded;

  return res;
}

nsResourceLoadDesc nsTextureCubeResource::UpdateContent(nsStreamReader* Stream)
{
  if (Stream == nullptr)
  {
    nsResourceLoadDesc res;
    res.m_uiQualityLevelsDiscardable = 0;
    res.m_uiQualityLevelsLoadable = 0;
    res.m_State = nsResourceState::LoadedResourceMissing;

    return res;
  }

  nsImage* pImage = nullptr;
  Stream->ReadBytes(&pImage, sizeof(nsImage*));

  bool bIsFallback = false;
  *Stream >> bIsFallback;

  nsTexFormat texFormat;
  texFormat.ReadHeader(*Stream);

  const nsUInt32 uiNumMipmapsLowRes = nsTextureUtils::s_bForceFullQualityAlways ? pImage->GetNumMipLevels() : 6;

  const nsUInt32 uiNumMipLevels = nsMath::Min(m_uiLoadedTextures == 0 ? uiNumMipmapsLowRes : pImage->GetNumMipLevels(), pImage->GetNumMipLevels());
  const nsUInt32 uiHighestMipLevel = pImage->GetNumMipLevels() - uiNumMipLevels;

  if (pImage->GetWidth(uiHighestMipLevel) != pImage->GetHeight(uiHighestMipLevel))
  {
    nsLog::Error("Cubemap width '{0}' is not identical to height '{1}'", pImage->GetWidth(uiHighestMipLevel), pImage->GetHeight(uiHighestMipLevel));

    nsResourceLoadDesc res;
    res.m_uiQualityLevelsDiscardable = 0;
    res.m_uiQualityLevelsLoadable = 0;
    res.m_State = nsResourceState::LoadedResourceMissing;

    return res;
  }

  m_Format = nsTextureUtils::ImageFormatToGalFormat(pImage->GetImageFormat(), texFormat.m_bSRGB);
  m_uiWidthAndHeight = pImage->GetWidth(uiHighestMipLevel);

  nsGALTextureCreationDescription texDesc;
  texDesc.m_Format = m_Format;
  texDesc.m_uiWidth = m_uiWidthAndHeight;
  texDesc.m_uiHeight = m_uiWidthAndHeight;
  texDesc.m_uiDepth = pImage->GetDepth(uiHighestMipLevel);
  texDesc.m_uiMipLevelCount = uiNumMipLevels;
  texDesc.m_uiArraySize = pImage->GetNumArrayIndices();

  if (texDesc.m_uiDepth > 1)
    texDesc.m_Type = nsGALTextureType::Texture3D;

  if (pImage->GetNumFaces() == 6)
    texDesc.m_Type = nsGALTextureType::TextureCube;

  NS_ASSERT_DEV(pImage->GetNumFaces() == 1 || pImage->GetNumFaces() == 6, "Invalid number of image faces (resource: '{0}')", GetResourceID());

  m_uiMemoryGPU[m_uiLoadedTextures] = 0;

  nsHybridArray<nsGALSystemMemoryDescription, 32> InitData;

  for (nsUInt32 array_index = 0; array_index < pImage->GetNumArrayIndices(); ++array_index)
  {
    for (nsUInt32 face = 0; face < pImage->GetNumFaces(); ++face)
    {
      for (nsUInt32 mip = uiHighestMipLevel; mip < pImage->GetNumMipLevels(); ++mip)
      {
        nsGALSystemMemoryDescription& id = InitData.ExpandAndGetRef();

        id.m_pData = pImage->GetPixelPointer<nsUInt8>(mip, face, array_index);

        NS_ASSERT_DEV(pImage->GetDepthPitch(mip) < nsMath::MaxValue<nsUInt32>(), "Depth pitch exceeds nsGAL limits.");

        if (nsImageFormat::GetType(pImage->GetImageFormat()) == nsImageFormatType::BLOCK_COMPRESSED)
        {
          const nsUInt32 uiMemPitchFactor = nsGALResourceFormat::GetBitsPerElement(m_Format) * 4 / 8;

          id.m_uiRowPitch = nsMath::Max<nsUInt32>(4, pImage->GetWidth(mip)) * uiMemPitchFactor;
        }
        else
        {
          id.m_uiRowPitch = static_cast<nsUInt32>(pImage->GetRowPitch(mip));
        }

        id.m_uiSlicePitch = static_cast<nsUInt32>(pImage->GetDepthPitch(mip));

        m_uiMemoryGPU[m_uiLoadedTextures] += id.m_uiSlicePitch;
      }
    }
  }

  const nsArrayPtr<nsGALSystemMemoryDescription> InitDataPtr(InitData);

  nsTextureCubeResourceDescriptor td;
  td.m_DescGAL = texDesc;
  td.m_SamplerDesc.m_AddressU = texFormat.m_AddressModeU;
  td.m_SamplerDesc.m_AddressV = texFormat.m_AddressModeV;
  td.m_SamplerDesc.m_AddressW = texFormat.m_AddressModeW;
  td.m_InitialContent = InitDataPtr;

  nsTextureUtils::ConfigureSampler(static_cast<nsTextureFilterSetting::Enum>(texFormat.m_TextureFilter.GetValue()), td.m_SamplerDesc);

  // ignore its return value here, we build our own
  CreateResource(std::move(td));

  {
    nsResourceLoadDesc res;
    res.m_uiQualityLevelsDiscardable = m_uiLoadedTextures;

    if (uiHighestMipLevel == 0)
      res.m_uiQualityLevelsLoadable = 0;
    else
      res.m_uiQualityLevelsLoadable = 1;

    res.m_State = nsResourceState::Loaded;

    return res;
  }
}

void nsTextureCubeResource::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
{
  out_NewMemoryUsage.m_uiMemoryCPU = sizeof(nsTextureCubeResource);
  out_NewMemoryUsage.m_uiMemoryGPU = m_uiMemoryGPU[0] + m_uiMemoryGPU[1];
}

NS_RESOURCE_IMPLEMENT_CREATEABLE(nsTextureCubeResource, nsTextureCubeResourceDescriptor)
{
  nsResourceLoadDesc ret;
  ret.m_uiQualityLevelsDiscardable = descriptor.m_uiQualityLevelsDiscardable;
  ret.m_uiQualityLevelsLoadable = descriptor.m_uiQualityLevelsLoadable;
  ret.m_State = nsResourceState::Loaded;

  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

  NS_ASSERT_DEV(descriptor.m_DescGAL.m_uiWidth == descriptor.m_DescGAL.m_uiHeight, "Cubemap width and height must be identical");

  m_Format = descriptor.m_DescGAL.m_Format;
  m_uiWidthAndHeight = descriptor.m_DescGAL.m_uiWidth;

  m_hGALTexture[m_uiLoadedTextures] = pDevice->CreateTexture(descriptor.m_DescGAL, descriptor.m_InitialContent);
  NS_ASSERT_DEV(!m_hGALTexture[m_uiLoadedTextures].IsInvalidated(), "Texture Data could not be uploaded to the GPU");

  pDevice->GetTexture(m_hGALTexture[m_uiLoadedTextures])->SetDebugName(GetResourceDescription());

  if (!m_hSamplerState.IsInvalidated())
  {
    pDevice->DestroySamplerState(m_hSamplerState);
  }

  m_hSamplerState = pDevice->CreateSamplerState(descriptor.m_SamplerDesc);
  NS_ASSERT_DEV(!m_hSamplerState.IsInvalidated(), "Sampler state error");

  ++m_uiLoadedTextures;

  return ret;
}



NS_STATICLINK_FILE(RendererCore, RendererCore_Textures_TextureCubeResource);
