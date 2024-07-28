#pragma once

#include <Core/ResourceManager/Resource.h>
#include <Core/ResourceManager/ResourceTypeLoader.h>
#include <RendererCore/RendererCoreDLL.h>

using nsDecalResourceHandle = nsTypedResourceHandle<class nsDecalResource>;

struct nsDecalResourceDescriptor
{
};

class NS_RENDERERCORE_DLL nsDecalResource : public nsResource
{
  NS_ADD_DYNAMIC_REFLECTION(nsDecalResource, nsResource);
  NS_RESOURCE_DECLARE_COMMON_CODE(nsDecalResource);
  NS_RESOURCE_DECLARE_CREATEABLE(nsDecalResource, nsDecalResourceDescriptor);

public:
  nsDecalResource();

private:
  virtual nsResourceLoadDesc UnloadData(Unload WhatToUnload) override;
  virtual nsResourceLoadDesc UpdateContent(nsStreamReader* Stream) override;
  virtual void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;
};

class NS_RENDERERCORE_DLL nsDecalResourceLoader : public nsResourceTypeLoader
{
public:
  struct LoadedData
  {
    LoadedData()
      : m_Reader(&m_Storage)
    {
    }

    nsContiguousMemoryStreamStorage m_Storage;
    nsMemoryStreamReader m_Reader;
  };

  virtual nsResourceLoadData OpenDataStream(const nsResource* pResource) override;
  virtual void CloseDataStream(const nsResource* pResource, const nsResourceLoadData& loaderData) override;
  virtual bool IsResourceOutdated(const nsResource* pResource) const override;
};
