#include <RendererCore/RendererCorePCH.h>

#include <Foundation/IO/ChunkStream.h>
#include <Foundation/Utilities/AssetFileHeader.h>
#include <RendererCore/BakedProbes/ProbeTreeSectorResource.h>

nsProbeTreeSectorResourceDescriptor::nsProbeTreeSectorResourceDescriptor() = default;
nsProbeTreeSectorResourceDescriptor::~nsProbeTreeSectorResourceDescriptor() = default;
nsProbeTreeSectorResourceDescriptor& nsProbeTreeSectorResourceDescriptor::operator=(nsProbeTreeSectorResourceDescriptor&& other) = default;

void nsProbeTreeSectorResourceDescriptor::Clear()
{
  m_ProbePositions.Clear();
  m_SkyVisibility.Clear();
}

nsUInt64 nsProbeTreeSectorResourceDescriptor::GetHeapMemoryUsage() const
{
  nsUInt64 uiMemUsage = 0;
  uiMemUsage += m_ProbePositions.GetHeapMemoryUsage();
  uiMemUsage += m_SkyVisibility.GetHeapMemoryUsage();
  return uiMemUsage;
}

static nsTypeVersion s_ProbeTreeResourceDescriptorVersion = 1;
nsResult nsProbeTreeSectorResourceDescriptor::Serialize(nsStreamWriter& inout_stream) const
{
  inout_stream.WriteVersion(s_ProbeTreeResourceDescriptorVersion);

  inout_stream << m_vGridOrigin;
  inout_stream << m_vProbeSpacing;
  inout_stream << m_vProbeCount;

  NS_SUCCEED_OR_RETURN(inout_stream.WriteArray(m_ProbePositions));
  NS_SUCCEED_OR_RETURN(inout_stream.WriteArray(m_SkyVisibility));

  return NS_SUCCESS;
}

nsResult nsProbeTreeSectorResourceDescriptor::Deserialize(nsStreamReader& inout_stream)
{
  Clear();

  const nsTypeVersion version = inout_stream.ReadVersion(s_ProbeTreeResourceDescriptorVersion);
  NS_IGNORE_UNUSED(version);

  inout_stream >> m_vGridOrigin;
  inout_stream >> m_vProbeSpacing;
  inout_stream >> m_vProbeCount;

  NS_SUCCEED_OR_RETURN(inout_stream.ReadArray(m_ProbePositions));
  NS_SUCCEED_OR_RETURN(inout_stream.ReadArray(m_SkyVisibility));

  return NS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsProbeTreeSectorResource, 1, nsRTTIDefaultAllocator<nsProbeTreeSectorResource>);
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_RESOURCE_IMPLEMENT_COMMON_CODE(nsProbeTreeSectorResource);
// clang-format on

nsProbeTreeSectorResource::nsProbeTreeSectorResource()
  : nsResource(DoUpdate::OnAnyThread, 1)
{
}

nsProbeTreeSectorResource::~nsProbeTreeSectorResource() = default;

nsResourceLoadDesc nsProbeTreeSectorResource::UnloadData(Unload WhatToUnload)
{
  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;
  res.m_State = nsResourceState::Unloaded;

  m_Desc.Clear();

  return res;
}

nsResourceLoadDesc nsProbeTreeSectorResource::UpdateContent(nsStreamReader* Stream)
{
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
    nsString sAbsFilePath;
    (*Stream) >> sAbsFilePath;
  }

  nsAssetFileHeader AssetHash;
  AssetHash.Read(*Stream).IgnoreResult();

  nsProbeTreeSectorResourceDescriptor descriptor;
  if (descriptor.Deserialize(*Stream).Failed())
  {
    res.m_State = nsResourceState::LoadedResourceMissing;
    return res;
  }

  return CreateResource(std::move(descriptor));
}

void nsProbeTreeSectorResource::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
{
  out_NewMemoryUsage.m_uiMemoryCPU = sizeof(nsProbeTreeSectorResource);
  out_NewMemoryUsage.m_uiMemoryCPU += m_Desc.GetHeapMemoryUsage();
  out_NewMemoryUsage.m_uiMemoryGPU = 0;
}

nsResourceLoadDesc nsProbeTreeSectorResource::CreateResource(nsProbeTreeSectorResourceDescriptor&& descriptor)
{
  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;
  res.m_State = nsResourceState::Loaded;

  m_Desc = std::move(descriptor);

  return res;
}


NS_STATICLINK_FILE(RendererCore, RendererCore_BakedProbes_Implementation_ProbeTreeSectorResource);
