#pragma once

#include <RendererCore/Meshes/MeshResourceDescriptor.h>

class NS_RENDERERCORE_DLL nsCpuMeshResource : public nsResource
{
  NS_ADD_DYNAMIC_REFLECTION(nsCpuMeshResource, nsResource);
  NS_RESOURCE_DECLARE_COMMON_CODE(nsCpuMeshResource);
  NS_RESOURCE_DECLARE_CREATEABLE(nsCpuMeshResource, nsMeshResourceDescriptor);

public:
  nsCpuMeshResource();

  const nsMeshResourceDescriptor& GetDescriptor() const { return m_Descriptor; }

private:
  virtual nsResourceLoadDesc UnloadData(Unload WhatToUnload) override;
  virtual nsResourceLoadDesc UpdateContent(nsStreamReader* Stream) override;
  virtual void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

  nsMeshResourceDescriptor m_Descriptor;
};

using nsCpuMeshResourceHandle = nsTypedResourceHandle<class nsCpuMeshResource>;
