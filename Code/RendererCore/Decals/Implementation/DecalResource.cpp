#include <RendererCore/RendererCorePCH.h>

#include <Foundation/Configuration/Startup.h>
#include <RendererCore/Decals/DecalResource.h>

static nsDecalResourceLoader s_DecalResourceLoader;

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(RendererCore, DecalResource)

  BEGIN_SUBSYSTEM_DEPENDENCIES
  "Foundation",
  "Core",
  "TextureResource"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    nsResourceManager::SetResourceTypeLoader<nsDecalResource>(&s_DecalResourceLoader);

    nsDecalResourceDescriptor desc;
    nsDecalResourceHandle hFallback = nsResourceManager::CreateResource<nsDecalResource>("Fallback Decal", std::move(desc), "Empty Decal for loading and missing decals");

    nsResourceManager::SetResourceTypeLoadingFallback<nsDecalResource>(hFallback);
    nsResourceManager::SetResourceTypeMissingFallback<nsDecalResource>(hFallback);
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    nsResourceManager::SetResourceTypeLoader<nsDecalResource>(nullptr);

    nsResourceManager::SetResourceTypeLoadingFallback<nsDecalResource>(nsDecalResourceHandle());
    nsResourceManager::SetResourceTypeMissingFallback<nsDecalResource>(nsDecalResourceHandle());
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
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsDecalResource, 1, nsRTTIDefaultAllocator<nsDecalResource>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_RESOURCE_IMPLEMENT_COMMON_CODE(nsDecalResource);
// clang-format on

nsDecalResource::nsDecalResource()
  : nsResource(DoUpdate::OnAnyThread, 1)
{
}

nsResourceLoadDesc nsDecalResource::UnloadData(Unload WhatToUnload)
{
  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;
  res.m_State = nsResourceState::Unloaded;

  return res;
}

nsResourceLoadDesc nsDecalResource::UpdateContent(nsStreamReader* Stream)
{
  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;
  res.m_State = nsResourceState::Loaded;

  return res;
}

void nsDecalResource::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
{
  out_NewMemoryUsage.m_uiMemoryCPU = sizeof(nsDecalResource);
  out_NewMemoryUsage.m_uiMemoryGPU = 0;
}

NS_RESOURCE_IMPLEMENT_CREATEABLE(nsDecalResource, nsDecalResourceDescriptor)
{
  nsResourceLoadDesc ret;
  ret.m_uiQualityLevelsDiscardable = 0;
  ret.m_uiQualityLevelsLoadable = 0;
  ret.m_State = nsResourceState::Loaded;

  return ret;
}

//////////////////////////////////////////////////////////////////////////

nsResourceLoadData nsDecalResourceLoader::OpenDataStream(const nsResource* pResource)
{
  // nothing to load, decals are solely identified by their id (name)
  // the rest of the information is in the decal atlas resource

  nsResourceLoadData res;
  return res;
}

void nsDecalResourceLoader::CloseDataStream(const nsResource* pResource, const nsResourceLoadData& loaderData)
{
  // nothing to do
}

bool nsDecalResourceLoader::IsResourceOutdated(const nsResource* pResource) const
{
  // decals are never outdated
  return false;
}



NS_STATICLINK_FILE(RendererCore, RendererCore_Decals_Implementation_DecalResource);
