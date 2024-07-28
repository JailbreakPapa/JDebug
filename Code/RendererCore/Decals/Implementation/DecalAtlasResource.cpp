#include <RendererCore/RendererCorePCH.h>

#include <Core/ResourceManager/ResourceManager.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/Math/Rect.h>
#include <Foundation/Utilities/AssetFileHeader.h>
#include <RendererCore/Decals/DecalAtlasResource.h>
#include <RendererCore/Textures/Texture2DResource.h>
#include <RendererCore/Textures/TextureUtils.h>
#include <Texture/Image/Formats/DdsFileFormat.h>
#include <Texture/Image/Image.h>

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(RendererCore, DecalAtlasResource)

  BEGIN_SUBSYSTEM_DEPENDENCIES
  "Foundation",
  "Core",
  "TextureResource"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    nsDecalAtlasResourceDescriptor desc;
    nsDecalAtlasResourceHandle hFallback = nsResourceManager::CreateResource<nsDecalAtlasResource>("Fallback Decal Atlas", std::move(desc), "Empty Decal Atlas for loading and missing decals");

    nsResourceManager::SetResourceTypeLoadingFallback<nsDecalAtlasResource>(hFallback);
    nsResourceManager::SetResourceTypeMissingFallback<nsDecalAtlasResource>(hFallback);
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    nsResourceManager::SetResourceTypeLoadingFallback<nsDecalAtlasResource>(nsDecalAtlasResourceHandle());
    nsResourceManager::SetResourceTypeMissingFallback<nsDecalAtlasResource>(nsDecalAtlasResourceHandle());
  }

  ON_HIGHLEVELSYSTEMS_STARTUP
  {
  }

  ON_HIGHLEVELSYSTEMS_SHUTDOWN
  {
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on


//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsDecalAtlasResource, 1, nsRTTIDefaultAllocator<nsDecalAtlasResource>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_RESOURCE_IMPLEMENT_COMMON_CODE(nsDecalAtlasResource);
// clang-format on

nsUInt32 nsDecalAtlasResource::s_uiDecalAtlasResources = 0;

nsDecalAtlasResource::nsDecalAtlasResource()
  : nsResource(DoUpdate::OnAnyThread, 1)
  , m_vBaseColorSize(nsVec2U32::MakeZero())
  , m_vNormalSize(nsVec2U32::MakeZero())
{
}

nsDecalAtlasResourceHandle nsDecalAtlasResource::GetDecalAtlasResource()
{
  return nsResourceManager::LoadResource<nsDecalAtlasResource>("{ ProjectDecalAtlas }");
}

nsResourceLoadDesc nsDecalAtlasResource::UnloadData(Unload WhatToUnload)
{
  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;
  res.m_State = nsResourceState::Unloaded;

  return res;
}

nsResourceLoadDesc nsDecalAtlasResource::UpdateContent(nsStreamReader* Stream)
{
  NS_LOG_BLOCK("nsDecalAtlasResource::UpdateContent", GetResourceIdOrDescription());

  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;
  res.m_State = nsResourceState::LoadedResourceMissing;

  if (Stream == nullptr)
    return res;

  // skip the absolute file path data that the standard file reader writes into the stream
  {
    nsStringBuilder sAbsFilePath;
    (*Stream) >> sAbsFilePath;
  }

  // skip the asset header
  {
    nsAssetFileHeader header;
    header.Read(*Stream).IgnoreResult();
  }

  {
    nsUInt8 uiVersion = 0;
    *Stream >> uiVersion;
    NS_ASSERT_DEV(uiVersion <= 3, "Invalid decal atlas version {0}", uiVersion);

    // this version is now incompatible
    if (uiVersion < 3)
      return res;
  }

  // read the textures
  {
    nsDdsFileFormat dds;
    nsImage baseColor, normal, orm;

    if (dds.ReadImage(*Stream, baseColor, "dds").Failed())
    {
      nsLog::Error("Failed to load baseColor image for decal atlas");
      return res;
    }

    if (dds.ReadImage(*Stream, normal, "dds").Failed())
    {
      nsLog::Error("Failed to load normal image for decal atlas");
      return res;
    }

    if (dds.ReadImage(*Stream, orm, "dds").Failed())
    {
      nsLog::Error("Failed to load normal image for decal atlas");
      return res;
    }

    CreateLayerTexture(baseColor, true, m_hBaseColor);
    CreateLayerTexture(normal, false, m_hNormal);
    CreateLayerTexture(orm, false, m_hORM);

    m_vBaseColorSize = nsVec2U32(baseColor.GetWidth(), baseColor.GetHeight());
    m_vNormalSize = nsVec2U32(normal.GetWidth(), normal.GetHeight());
    m_vORMSize = nsVec2U32(orm.GetWidth(), orm.GetHeight());
  }

  ReadDecalInfo(Stream);

  res.m_State = nsResourceState::Loaded;
  return res;
}

void nsDecalAtlasResource::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
{
  out_NewMemoryUsage.m_uiMemoryCPU = sizeof(nsDecalAtlasResource) + (nsUInt32)m_Atlas.m_Items.GetHeapMemoryUsage();
  out_NewMemoryUsage.m_uiMemoryGPU = 0;
}

NS_RESOURCE_IMPLEMENT_CREATEABLE(nsDecalAtlasResource, nsDecalAtlasResourceDescriptor)
{
  nsResourceLoadDesc ret;
  ret.m_uiQualityLevelsDiscardable = 0;
  ret.m_uiQualityLevelsLoadable = 0;
  ret.m_State = nsResourceState::Loaded;

  m_Atlas.Clear();

  return ret;
}

void nsDecalAtlasResource::CreateLayerTexture(const nsImage& img, bool bSRGB, nsTexture2DResourceHandle& out_hTexture)
{
  nsTexture2DResourceDescriptor td;
  td.m_SamplerDesc.m_AddressU = nsImageAddressMode::Clamp;
  td.m_SamplerDesc.m_AddressV = nsImageAddressMode::Clamp;
  td.m_SamplerDesc.m_AddressW = nsImageAddressMode::Clamp;

  nsUInt32 uiMemory;
  nsHybridArray<nsGALSystemMemoryDescription, 32> initData;
  nsTexture2DResource::FillOutDescriptor(td, &img, bSRGB, img.GetNumMipLevels(), uiMemory, initData);
  nsTextureUtils::ConfigureSampler(nsTextureFilterSetting::HighQuality, td.m_SamplerDesc);

  nsStringBuilder sTexId;
  sTexId.SetFormat("{0}_Tex{1}", GetResourceID(), s_uiDecalAtlasResources);
  ++s_uiDecalAtlasResources;

  out_hTexture = nsResourceManager::CreateResource<nsTexture2DResource>(sTexId, std::move(td));
}

void nsDecalAtlasResource::ReadDecalInfo(nsStreamReader* Stream)
{
  m_Atlas.Deserialize(*Stream).IgnoreResult();
}

void nsDecalAtlasResource::ReportResourceIsMissing()
{
#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  // normal during development, don't care much
  nsLog::Debug("Decal Atlas Resource is missing: '{0}' ('{1}')", GetResourceID(), GetResourceDescription());
#else
  // should probably exist for shipped applications, report this
  nsLog::Warning("Decal Atlas Resource is missing: '{0}' ('{1}')", GetResourceID(), GetResourceDescription());
#endif
}



NS_STATICLINK_FILE(RendererCore, RendererCore_Decals_Implementation_DecalAtlasResource);
