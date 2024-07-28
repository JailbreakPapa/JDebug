#pragma once

#include <Core/ResourceManager/Resource.h>
#include <Core/ResourceManager/ResourceTypeLoader.h>
#include <RendererCore/RenderContext/Implementation/RenderContextStructs.h>
#include <RendererCore/RendererCoreDLL.h>
#include <RendererFoundation/RendererFoundationDLL.h>
#include <Texture/Image/Image.h>
#include <Texture/nsTexFormat/nsTexFormat.h>

class NS_RENDERERCORE_DLL nsTextureResourceLoader : public nsResourceTypeLoader
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
    nsImage m_Image;

    bool m_bIsFallback = false;
    nsTexFormat m_TexFormat;
  };

  virtual nsResourceLoadData OpenDataStream(const nsResource* pResource) override;
  virtual void CloseDataStream(const nsResource* pResource, const nsResourceLoadData& loaderData) override;
  virtual bool IsResourceOutdated(const nsResource* pResource) const override;

  static nsResult LoadTexFile(nsStreamReader& inout_stream, LoadedData& ref_data);
  static void WriteTextureLoadStream(nsStreamWriter& inout_stream, const LoadedData& data);
};
