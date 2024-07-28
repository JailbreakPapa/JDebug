#include <RendererCore/RendererCorePCH.h>

#include <Foundation/Configuration/CVar.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/Utilities/AssetFileHeader.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererCore/Textures/Texture2DResource.h>
#include <RendererCore/Textures/TextureUtils.h>
#include <RendererFoundation/Resources/Texture.h>
#include <Texture/Image/Formats/DdsFileFormat.h>
#include <Texture/Image/Image.h>
#include <Texture/nsTexFormat/nsTexFormat.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsTexture2DResource, 1, nsRTTIDefaultAllocator<nsTexture2DResource>)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsCVarInt cvar_RenderingOffscreenTargetResolution1("Rendering.Offscreen.TargetResolution1", 256, nsCVarFlags::Default, "Configurable render target resolution");
nsCVarInt cvar_RenderingOffscreenTargetResolution2("Rendering.Offscreen.TargetResolution2", 512, nsCVarFlags::Default, "Configurable render target resolution");

NS_RESOURCE_IMPLEMENT_COMMON_CODE(nsTexture2DResource);

nsTexture2DResource::nsTexture2DResource()
  : nsResource(DoUpdate::OnAnyThread, nsTextureUtils::s_bForceFullQualityAlways ? 1 : 2)
{
}

nsTexture2DResource::nsTexture2DResource(nsResource::DoUpdate ResourceUpdateThread)
  : nsResource(ResourceUpdateThread, nsTextureUtils::s_bForceFullQualityAlways ? 1 : 2)
{
}

nsResourceLoadDesc nsTexture2DResource::UnloadData(Unload WhatToUnload)
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

void nsTexture2DResource::FillOutDescriptor(nsTexture2DResourceDescriptor& ref_td, const nsImage* pImage, bool bSRGB, nsUInt32 uiNumMipLevels,
  nsUInt32& out_uiMemoryUsed, nsHybridArray<nsGALSystemMemoryDescription, 32>& ref_initData)
{
  const nsUInt32 uiHighestMipLevel = pImage->GetNumMipLevels() - uiNumMipLevels;

  const nsGALResourceFormat::Enum format = nsTextureUtils::ImageFormatToGalFormat(pImage->GetImageFormat(), bSRGB);

  ref_td.m_DescGAL.m_Format = format;
  ref_td.m_DescGAL.m_uiWidth = pImage->GetWidth(uiHighestMipLevel);
  ref_td.m_DescGAL.m_uiHeight = pImage->GetHeight(uiHighestMipLevel);
  ref_td.m_DescGAL.m_uiDepth = pImage->GetDepth(uiHighestMipLevel);
  ref_td.m_DescGAL.m_uiMipLevelCount = uiNumMipLevels;
  ref_td.m_DescGAL.m_uiArraySize = pImage->GetNumArrayIndices();

  if (nsImageFormat::GetType(pImage->GetImageFormat()) == nsImageFormatType::BLOCK_COMPRESSED)
  {
    ref_td.m_DescGAL.m_uiWidth = nsMath::RoundUp(ref_td.m_DescGAL.m_uiWidth, 4);
    ref_td.m_DescGAL.m_uiHeight = nsMath::RoundUp(ref_td.m_DescGAL.m_uiHeight, 4);
  }

  if (ref_td.m_DescGAL.m_uiDepth > 1)
    ref_td.m_DescGAL.m_Type = nsGALTextureType::Texture3D;

  if (pImage->GetNumFaces() == 6)
    ref_td.m_DescGAL.m_Type = nsGALTextureType::TextureCube;

  NS_ASSERT_DEV(pImage->GetNumFaces() == 1 || pImage->GetNumFaces() == 6, "Invalid number of image faces");

  out_uiMemoryUsed = 0;

  ref_initData.Clear();

  for (nsUInt32 array_index = 0; array_index < pImage->GetNumArrayIndices(); ++array_index)
  {
    for (nsUInt32 face = 0; face < pImage->GetNumFaces(); ++face)
    {
      for (nsUInt32 mip = uiHighestMipLevel; mip < pImage->GetNumMipLevels(); ++mip)
      {
        nsGALSystemMemoryDescription& id = ref_initData.ExpandAndGetRef();

        id.m_pData = const_cast<nsUInt8*>(pImage->GetPixelPointer<nsUInt8>(mip, face, array_index));

        if (nsImageFormat::GetType(pImage->GetImageFormat()) == nsImageFormatType::BLOCK_COMPRESSED)
        {
          const nsUInt32 uiMemPitchFactor = nsGALResourceFormat::GetBitsPerElement(format) * 4 / 8;

          id.m_uiRowPitch = nsMath::RoundUp(pImage->GetWidth(mip), 4) * uiMemPitchFactor;
        }
        else
        {
          id.m_uiRowPitch = static_cast<nsUInt32>(pImage->GetRowPitch(mip));
        }

        NS_ASSERT_DEV(pImage->GetDepthPitch(mip) < nsMath::MaxValue<nsUInt32>(), "Depth pitch exceeds nsGAL limits.");
        id.m_uiSlicePitch = static_cast<nsUInt32>(pImage->GetDepthPitch(mip));

        out_uiMemoryUsed += id.m_uiSlicePitch;
      }
    }
  }

  const nsArrayPtr<nsGALSystemMemoryDescription> InitDataPtr(ref_initData);

  ref_td.m_InitialContent = InitDataPtr;
}


nsResourceLoadDesc nsTexture2DResource::UpdateContent(nsStreamReader* Stream)
{
  if (Stream == nullptr)
  {
    nsResourceLoadDesc res;
    res.m_uiQualityLevelsDiscardable = 0;
    res.m_uiQualityLevelsLoadable = 0;
    res.m_State = nsResourceState::LoadedResourceMissing;

    return res;
  }

  nsTexture2DResourceDescriptor td;
  nsImage* pImage = nullptr;
  bool bIsFallback = false;
  nsTexFormat texFormat;

  // load image data
  {
    Stream->ReadBytes(&pImage, sizeof(nsImage*));
    *Stream >> bIsFallback;
    texFormat.ReadHeader(*Stream);

    td.m_SamplerDesc.m_AddressU = texFormat.m_AddressModeU;
    td.m_SamplerDesc.m_AddressV = texFormat.m_AddressModeV;
    td.m_SamplerDesc.m_AddressW = texFormat.m_AddressModeW;
  }

  const bool bIsRenderTarget = texFormat.m_iRenderTargetResolutionX != 0;
  NS_ASSERT_DEV(!bIsRenderTarget, "Render targets are not supported by regular 2D texture resources");

  {

    const nsUInt32 uiNumMipmapsLowRes = nsTextureUtils::s_bForceFullQualityAlways ? pImage->GetNumMipLevels() : nsMath::Min(pImage->GetNumMipLevels(), 6U);
    nsUInt32 uiUploadNumMipLevels = 0;
    bool bCouldLoadMore = false;

    if (bIsFallback)
    {
      if (m_uiLoadedTextures == 0)
      {
        // only upload fallback textures, if we don't have any texture data at all, yet
        bCouldLoadMore = true;
        uiUploadNumMipLevels = uiNumMipmapsLowRes;
      }
      else if (m_uiLoadedTextures == 1)
      {
        // ignore this texture entirely, if we already have low res data
        // but assume we could load a higher resolution version
        bCouldLoadMore = true;
        nsLog::Debug("Ignoring fallback texture data, low-res resource data is already loaded.");
      }
      else
      {
        nsLog::Debug("Ignoring fallback texture data, resource is already fully loaded.");
      }
    }
    else
    {
      if (m_uiLoadedTextures == 0)
      {
        bCouldLoadMore = uiNumMipmapsLowRes < pImage->GetNumMipLevels();
        uiUploadNumMipLevels = uiNumMipmapsLowRes;
      }
      else if (m_uiLoadedTextures == 1)
      {
        uiUploadNumMipLevels = pImage->GetNumMipLevels();
      }
      else
      {
        // ignore the texture, if we already have fully loaded data
        nsLog::Debug("Ignoring texture data, resource is already fully loaded.");
      }
    }

    if (uiUploadNumMipLevels > 0)
    {
      NS_ASSERT_DEBUG(m_uiLoadedTextures < 2, "Invalid texture upload");

      nsHybridArray<nsGALSystemMemoryDescription, 32> initData;
      FillOutDescriptor(td, pImage, texFormat.m_bSRGB, uiUploadNumMipLevels, m_uiMemoryGPU[m_uiLoadedTextures], initData);

      nsTextureUtils::ConfigureSampler(static_cast<nsTextureFilterSetting::Enum>(texFormat.m_TextureFilter.GetValue()), td.m_SamplerDesc);

      // ignore its return value here, we build our own
      CreateResource(std::move(td));
    }

    {
      nsResourceLoadDesc res;
      res.m_uiQualityLevelsDiscardable = m_uiLoadedTextures;
      res.m_uiQualityLevelsLoadable = bCouldLoadMore ? 1 : 0;
      res.m_State = nsResourceState::Loaded;

      return res;
    }
  }
}

void nsTexture2DResource::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
{
  out_NewMemoryUsage.m_uiMemoryCPU = sizeof(nsTexture2DResource);
  out_NewMemoryUsage.m_uiMemoryGPU = m_uiMemoryGPU[0] + m_uiMemoryGPU[1];
}

NS_RESOURCE_IMPLEMENT_CREATEABLE(nsTexture2DResource, nsTexture2DResourceDescriptor)
{
  nsResourceLoadDesc ret;
  ret.m_uiQualityLevelsDiscardable = descriptor.m_uiQualityLevelsDiscardable;
  ret.m_uiQualityLevelsLoadable = descriptor.m_uiQualityLevelsLoadable;
  ret.m_State = nsResourceState::Loaded;

  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

  m_Type = descriptor.m_DescGAL.m_Type;
  m_Format = descriptor.m_DescGAL.m_Format;
  m_uiWidth = descriptor.m_DescGAL.m_uiWidth;
  m_uiHeight = descriptor.m_DescGAL.m_uiHeight;

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

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// TODO (resources): move into separate file

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsRenderToTexture2DResource, 1, nsRTTIDefaultAllocator<nsRenderToTexture2DResource>);
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_SUBSYSTEM_DECLARATION(RendererCore, Texture2D)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "ResourceManager" 
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP 
  {
    nsResourceManager::RegisterResourceOverrideType(nsGetStaticRTTI<nsRenderToTexture2DResource>(), [](const nsStringBuilder& sResourceID) -> bool  {
        return sResourceID.HasExtension(".nsRenderTarget");
      });
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    nsResourceManager::UnregisterResourceOverrideType(nsGetStaticRTTI<nsRenderToTexture2DResource>());
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

NS_RESOURCE_IMPLEMENT_COMMON_CODE(nsRenderToTexture2DResource);

NS_RESOURCE_IMPLEMENT_CREATEABLE(nsRenderToTexture2DResource, nsRenderToTexture2DResourceDescriptor)
{
  nsResourceLoadDesc ret;
  ret.m_uiQualityLevelsDiscardable = 0;
  ret.m_uiQualityLevelsLoadable = 0;
  ret.m_State = nsResourceState::Loaded;

  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

  m_Type = nsGALTextureType::Texture2D;
  m_Format = descriptor.m_Format;
  m_uiWidth = descriptor.m_uiWidth;
  m_uiHeight = descriptor.m_uiHeight;

  nsGALTextureCreationDescription descGAL;
  descGAL.SetAsRenderTarget(m_uiWidth, m_uiHeight, m_Format, descriptor.m_SampleCount);

  m_hGALTexture[m_uiLoadedTextures] = pDevice->CreateTexture(descGAL, descriptor.m_InitialContent);
  NS_ASSERT_DEV(!m_hGALTexture[m_uiLoadedTextures].IsInvalidated(), "Texture data could not be uploaded to the GPU");

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

nsResourceLoadDesc nsRenderToTexture2DResource::UnloadData(Unload WhatToUnload)
{
  for (nsInt32 r = 0; r < 2; ++r)
  {
    if (!m_hGALTexture[r].IsInvalidated())
    {
      nsGALDevice::GetDefaultDevice()->DestroyTexture(m_hGALTexture[r]);
      m_hGALTexture[r].Invalidate();
    }

    m_uiMemoryGPU[r] = 0;
  }

  m_uiLoadedTextures = 0;

  if (!m_hSamplerState.IsInvalidated())
  {
    nsGALDevice::GetDefaultDevice()->DestroySamplerState(m_hSamplerState);
    m_hSamplerState.Invalidate();
  }

  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = m_uiLoadedTextures;
  res.m_uiQualityLevelsLoadable = 2 - m_uiLoadedTextures;
  res.m_State = nsResourceState::Unloaded;
  return res;
}

nsGALRenderTargetViewHandle nsRenderToTexture2DResource::GetRenderTargetView() const
{
  return nsGALDevice::GetDefaultDevice()->GetDefaultRenderTargetView(m_hGALTexture[0]);
}

void nsRenderToTexture2DResource::AddRenderView(nsViewHandle hView)
{
  m_RenderViews.PushBack(hView);
}

void nsRenderToTexture2DResource::RemoveRenderView(nsViewHandle hView)
{
  m_RenderViews.RemoveAndSwap(hView);
}

const nsDynamicArray<nsViewHandle>& nsRenderToTexture2DResource::GetAllRenderViews() const
{
  return m_RenderViews;
}

static nsUInt16 GetNextBestResolution(float fRes)
{
  fRes = nsMath::Clamp(fRes, 8.0f, 4096.0f);

  int mulEight = (int)nsMath::Floor((fRes + 7.9f) / 8.0f);

  return static_cast<nsUInt16>(mulEight * 8);
}

nsResourceLoadDesc nsRenderToTexture2DResource::UpdateContent(nsStreamReader* Stream)
{
  if (Stream == nullptr)
  {
    nsResourceLoadDesc res;
    res.m_uiQualityLevelsDiscardable = 0;
    res.m_uiQualityLevelsLoadable = 0;
    res.m_State = nsResourceState::LoadedResourceMissing;

    return res;
  }

  nsRenderToTexture2DResourceDescriptor td;
  nsImage* pImage = nullptr;
  bool bIsFallback = false;
  nsTexFormat texFormat;

  // load image data
  {
    Stream->ReadBytes(&pImage, sizeof(nsImage*));
    *Stream >> bIsFallback;
    texFormat.ReadHeader(*Stream);

    td.m_SamplerDesc.m_AddressU = texFormat.m_AddressModeU;
    td.m_SamplerDesc.m_AddressV = texFormat.m_AddressModeV;
    td.m_SamplerDesc.m_AddressW = texFormat.m_AddressModeW;
  }

  const bool bIsRenderTarget = texFormat.m_iRenderTargetResolutionX != 0;

  NS_ASSERT_DEV(bIsRenderTarget, "Trying to create a RenderToTexture resource from data that is not set up as a render-target");

  {
    NS_ASSERT_DEV(m_uiLoadedTextures == 0, "not implemented");

    if (texFormat.m_iRenderTargetResolutionX == -1)
    {
      if (texFormat.m_iRenderTargetResolutionY == 1)
      {
        texFormat.m_iRenderTargetResolutionX = GetNextBestResolution(cvar_RenderingOffscreenTargetResolution1 * texFormat.m_fResolutionScale);
        texFormat.m_iRenderTargetResolutionY = texFormat.m_iRenderTargetResolutionX;
      }
      else if (texFormat.m_iRenderTargetResolutionY == 2)
      {
        texFormat.m_iRenderTargetResolutionX = GetNextBestResolution(cvar_RenderingOffscreenTargetResolution2 * texFormat.m_fResolutionScale);
        texFormat.m_iRenderTargetResolutionY = texFormat.m_iRenderTargetResolutionX;
      }
      else
      {
        NS_REPORT_FAILURE(
          "Invalid render target configuration: {0} x {1}", texFormat.m_iRenderTargetResolutionX, texFormat.m_iRenderTargetResolutionY);
      }
    }

    td.m_Format = static_cast<nsGALResourceFormat::Enum>(texFormat.m_GalRenderTargetFormat);
    td.m_uiWidth = texFormat.m_iRenderTargetResolutionX;
    td.m_uiHeight = texFormat.m_iRenderTargetResolutionY;

    nsTextureUtils::ConfigureSampler(static_cast<nsTextureFilterSetting::Enum>(texFormat.m_TextureFilter.GetValue()), td.m_SamplerDesc);

    m_uiLoadedTextures = 0;

    CreateResource(std::move(td));
  }

  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;
  res.m_State = nsResourceState::Loaded;
  return res;
}

void nsRenderToTexture2DResource::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
{
  out_NewMemoryUsage.m_uiMemoryCPU = sizeof(nsRenderToTexture2DResource);
  out_NewMemoryUsage.m_uiMemoryGPU = m_uiMemoryGPU[0] + m_uiMemoryGPU[1];
}

NS_STATICLINK_FILE(RendererCore, RendererCore_Textures_Texture2DResource);
