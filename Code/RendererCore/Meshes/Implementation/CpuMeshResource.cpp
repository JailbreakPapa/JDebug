#include <RendererCore/RendererCorePCH.h>

#include <Foundation/Utilities/AssetFileHeader.h>
#include <RendererCore/Meshes/CpuMeshResource.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsCpuMeshResource, 1, nsRTTIDefaultAllocator<nsCpuMeshResource>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_RESOURCE_IMPLEMENT_COMMON_CODE(nsCpuMeshResource);
// clang-format on

nsCpuMeshResource::nsCpuMeshResource()
  : nsResource(DoUpdate::OnAnyThread, 1)
{
}

nsResourceLoadDesc nsCpuMeshResource::UnloadData(Unload WhatToUnload)
{
  nsResourceLoadDesc res;
  res.m_State = GetLoadingState();
  res.m_uiQualityLevelsDiscardable = GetNumQualityLevelsDiscardable();
  res.m_uiQualityLevelsLoadable = GetNumQualityLevelsLoadable();

  // we currently can only unload the entire mesh
  // if (WhatToUnload == Unload::AllQualityLevels)
  {
    m_Descriptor.Clear();

    res.m_uiQualityLevelsDiscardable = 0;
    res.m_uiQualityLevelsLoadable = 0;
    res.m_State = nsResourceState::Unloaded;
  }

  return res;
}

nsResourceLoadDesc nsCpuMeshResource::UpdateContent(nsStreamReader* Stream)
{
  nsMeshResourceDescriptor desc;
  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;

  if (Stream == nullptr)
  {
    res.m_State = nsResourceState::LoadedResourceMissing;
    return res;
  }

  // skip the absolute file path data that the standard file reader writes into the stream
  {
    nsStringBuilder sAbsFilePath;
    (*Stream) >> sAbsFilePath;
  }

  nsAssetFileHeader AssetHash;
  AssetHash.Read(*Stream).IgnoreResult();

  if (m_Descriptor.Load(*Stream).Failed())
  {
    res.m_State = nsResourceState::LoadedResourceMissing;
    return res;
  }

  res.m_State = nsResourceState::Loaded;
  return res;
}

void nsCpuMeshResource::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
{
  out_NewMemoryUsage.m_uiMemoryCPU = sizeof(nsCpuMeshResource);
  out_NewMemoryUsage.m_uiMemoryGPU = 0;
}

NS_RESOURCE_IMPLEMENT_CREATEABLE(nsCpuMeshResource, nsMeshResourceDescriptor)
{
  m_Descriptor = descriptor;

  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;
  res.m_State = nsResourceState::Loaded;

  return res;
}



NS_STATICLINK_FILE(RendererCore, RendererCore_Meshes_Implementation_CpuMeshResource);
