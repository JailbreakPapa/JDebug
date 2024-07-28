#pragma once

#include <RendererCore/RendererCoreDLL.h>

#include <Core/ResourceManager/Resource.h>
#include <Core/ResourceManager/ResourceTypeLoader.h>
#include <Foundation/IO/MemoryStream.h>
#include <RendererCore/Pipeline/Declarations.h>
#include <RendererCore/RenderContext/Implementation/RenderContextStructs.h>
#include <RendererFoundation/Descriptors/Descriptors.h>
#include <RendererCore/Textures/Virtual/VirtualTextureTypes.h>
#include <RendererFoundation/RendererFoundationDLL.h>

class nsVirtualTextureMTManager;
class nsImage;

using nsVirtualTexture2DResourceHandle = nsTypedResourceHandle<class nsVirtualTexture2DResource>;

/// \brief Use this descriptor in calls to nsResourceManager::CreateResource<nsVirtualTexture2DResource> to create textures from data in memory.
struct NS_RENDERERCORE_DLL nsVirtualTexture2DResourceDescriptor
{
  /// Describes the texture format, etc.
  nsGALTextureCreationDescription m_DescGAL;
  nsGALSamplerStateCreationDescription m_SamplerDesc;

  /// How many quality levels can be discarded and reloaded. For created textures this can currently only be 0 or 1.
  nsUInt8 m_uiQualityLevelsDiscardable = 0;

  /// How many additional quality levels can be loaded (typically from file).
  nsUInt8 m_uiQualityLevelsLoadable = 0;

  /// One memory desc per (array * faces * mipmap) (in that order) (array is outer loop, mipmap is inner loop). Can be empty to not
  /// initialize data.
  nsArrayPtr<nsGALSystemMemoryDescription> m_InitialContent;


};

class NS_RENDERERCORE_DLL nsVirtualTexture2DResource : public nsResource
{
  NS_ADD_DYNAMIC_REFLECTION(nsVirtualTexture2DResource, nsResource);

  NS_RESOURCE_DECLARE_COMMON_CODE(nsVirtualTexture2DResource);
  NS_RESOURCE_DECLARE_CREATEABLE(nsVirtualTexture2DResource, nsVirtualTexture2DResourceDescriptor);

public:
  nsVirtualTexture2DResource();

  NS_ALWAYS_INLINE nsGALResourceFormat::Enum GetFormat() const { return m_Format; }
  NS_ALWAYS_INLINE nsUInt32 GetFullVTWidth() const { return m_uiWidth; }
  NS_ALWAYS_INLINE nsUInt32 GetFullVTHeight() const { return m_uiHeight; }
  NS_ALWAYS_INLINE nsGALTextureType::Enum GetType() const { return m_Type; }
protected:
  virtual nsResourceLoadDesc UnloadData(Unload WhatToUnload) override;
  virtual nsResourceLoadDesc UpdateContent(nsStreamReader* Stream) override;
  virtual void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;
  
  nsVirtualTexture2DResource(DoUpdate ResourceUpdateThread);
  /// NOTE: This data below is releative to the data that is sent to the Resource from PageTable.
  /// i.e. we only load the pages that are visible to the camera.
  
  nsUInt8 m_uiLoadedTextures = 0;
  nsGALTextureHandle m_hGALTexture[2];
  nsUInt32 m_uiMemoryGPU[2] = {0, 0};

  nsGALTextureType::Enum m_Type = nsGALTextureType::Invalid;
  nsGALResourceFormat::Enum m_Format = nsGALResourceFormat::Invalid;
  nsUInt32 m_uiWidth = 0;
  nsUInt32 m_uiHeight = 0;


  nsDeque<nsUInt16> m_vtnewpages;
  nsList<nsUInt16> m_vtoldpages;

  nsMap<nsUInt32, nsVirtualTexturePage> m_vtcachedpages;
#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  /// @brief The access times of the cached pages.
  // NOTE: is timestamp overkill?
  nsMap<nsUInt32, nsTimestamp> m_vtcachedpagesaccesstimes;
#endif
  nsVirtualTextureMTManager* m_pVTManager = nullptr;
  nsGALSamplerStateHandle m_hSamplerState;
};