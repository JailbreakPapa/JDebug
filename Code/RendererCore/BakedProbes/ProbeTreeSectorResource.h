#pragma once

#include <Core/Graphics/AmbientCubeBasis.h>
#include <Core/ResourceManager/Resource.h>
#include <RendererCore/BakedProbes/BakingUtils.h>

using nsProbeTreeSectorResourceHandle = nsTypedResourceHandle<class nsProbeTreeSectorResource>;

struct NS_RENDERERCORE_DLL nsProbeTreeSectorResourceDescriptor
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsProbeTreeSectorResourceDescriptor);

  nsProbeTreeSectorResourceDescriptor();
  ~nsProbeTreeSectorResourceDescriptor();
  nsProbeTreeSectorResourceDescriptor& operator=(nsProbeTreeSectorResourceDescriptor&& other);

  nsVec3 m_vGridOrigin;
  nsVec3 m_vProbeSpacing;
  nsVec3U32 m_vProbeCount;

  nsDynamicArray<nsVec3> m_ProbePositions;
  nsDynamicArray<nsCompressedSkyVisibility> m_SkyVisibility;

  void Clear();
  nsUInt64 GetHeapMemoryUsage() const;

  nsResult Serialize(nsStreamWriter& inout_stream) const;
  nsResult Deserialize(nsStreamReader& inout_stream);
};

class NS_RENDERERCORE_DLL nsProbeTreeSectorResource : public nsResource
{
  NS_ADD_DYNAMIC_REFLECTION(nsProbeTreeSectorResource, nsResource);
  NS_RESOURCE_DECLARE_COMMON_CODE(nsProbeTreeSectorResource);
  NS_RESOURCE_DECLARE_CREATEABLE(nsProbeTreeSectorResource, nsProbeTreeSectorResourceDescriptor);

public:
  nsProbeTreeSectorResource();
  ~nsProbeTreeSectorResource();

  const nsVec3& GetGridOrigin() const { return m_Desc.m_vGridOrigin; }
  const nsVec3& GetProbeSpacing() const { return m_Desc.m_vProbeSpacing; }
  const nsVec3U32& GetProbeCount() const { return m_Desc.m_vProbeCount; }

  nsArrayPtr<const nsVec3> GetProbePositions() const { return m_Desc.m_ProbePositions; }
  nsArrayPtr<const nsCompressedSkyVisibility> GetSkyVisibility() const { return m_Desc.m_SkyVisibility; }

private:
  virtual nsResourceLoadDesc UnloadData(Unload WhatToUnload) override;
  virtual nsResourceLoadDesc UpdateContent(nsStreamReader* Stream) override;
  virtual void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

private:
  nsProbeTreeSectorResourceDescriptor m_Desc;
};
