#pragma once

#include <Core/ResourceManager/Resource.h>
#include <Core/ResourceManager/ResourceTypeLoader.h>
#include <Foundation/Containers/HashTable.h>
#include <RendererCore/RendererCoreDLL.h>

using nsRenderPipelineResourceHandle = nsTypedResourceHandle<class nsRenderPipelineResource>;
class nsRenderPipeline;

struct nsRenderPipelineResourceDescriptor
{
  void Clear() {}

  void CreateFromRenderPipeline(const nsRenderPipeline* pPipeline);

  nsDynamicArray<nsUInt8> m_SerializedPipeline;
  nsString m_sPath;
};

class NS_RENDERERCORE_DLL nsRenderPipelineResource : public nsResource
{
  NS_ADD_DYNAMIC_REFLECTION(nsRenderPipelineResource, nsResource);
  NS_RESOURCE_DECLARE_COMMON_CODE(nsRenderPipelineResource);
  NS_RESOURCE_DECLARE_CREATEABLE(nsRenderPipelineResource, nsRenderPipelineResourceDescriptor);

public:
  nsRenderPipelineResource();

  NS_ALWAYS_INLINE const nsRenderPipelineResourceDescriptor& GetDescriptor() { return m_Desc; }

  nsInternal::NewInstance<nsRenderPipeline> CreateRenderPipeline() const;

public:
  static nsRenderPipelineResourceHandle CreateMissingPipeline();

private:
  virtual nsResourceLoadDesc UnloadData(Unload WhatToUnload) override;
  virtual nsResourceLoadDesc UpdateContent(nsStreamReader* Stream) override;
  virtual void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

private:
  nsRenderPipelineResourceDescriptor m_Desc;
};
