#pragma once

#include <Core/ResourceManager/Resource.h>
#include <Core/ResourceManager/ResourceTypeLoader.h>
#include <Foundation/IO/MemoryStream.h>
#include <RendererCore/RenderContext/Implementation/RenderContextStructs.h>
#include <RendererCore/RendererCoreDLL.h>
#include <RendererFoundation/Descriptors/Descriptors.h>
#include <RendererFoundation/RendererFoundationDLL.h>
#include <Texture/Image/Image.h>

using nsTextureCubeResourceHandle = nsTypedResourceHandle<class nsTextureCubeResource>;

/// \brief Use this descriptor in calls to nsResourceManager::CreateResource<nsTextureCubeResource> to create textures from data in memory.
struct nsTextureCubeResourceDescriptor
{
  nsTextureCubeResourceDescriptor()
  {
    m_uiQualityLevelsDiscardable = 0;
    m_uiQualityLevelsLoadable = 0;
  }

  /// Describes the texture format, etc.
  nsGALTextureCreationDescription m_DescGAL;
  nsGALSamplerStateCreationDescription m_SamplerDesc;

  /// How many quality levels can be discarded and reloaded. For created textures this can currently only be 0 or 1.
  nsUInt8 m_uiQualityLevelsDiscardable;

  /// How many additional quality levels can be loaded (typically from file).
  nsUInt8 m_uiQualityLevelsLoadable;

  /// One memory desc per (array * faces * mipmap) (in that order) (array is outer loop, mipmap is inner loop). Can be empty to not
  /// initialize data.
  nsArrayPtr<nsGALSystemMemoryDescription> m_InitialContent;
};

class NS_RENDERERCORE_DLL nsTextureCubeResource : public nsResource
{
  NS_ADD_DYNAMIC_REFLECTION(nsTextureCubeResource, nsResource);
  NS_RESOURCE_DECLARE_COMMON_CODE(nsTextureCubeResource);
  NS_RESOURCE_DECLARE_CREATEABLE(nsTextureCubeResource, nsTextureCubeResourceDescriptor);

public:
  nsTextureCubeResource();

  NS_ALWAYS_INLINE nsGALResourceFormat::Enum GetFormat() const { return m_Format; }
  NS_ALWAYS_INLINE nsUInt32 GetWidthAndHeight() const { return m_uiWidthAndHeight; }

  const nsGALTextureHandle& GetGALTexture() const { return m_hGALTexture[m_uiLoadedTextures - 1]; }
  const nsGALSamplerStateHandle& GetGALSamplerState() const { return m_hSamplerState; }

protected:
  virtual nsResourceLoadDesc UnloadData(Unload WhatToUnload) override;
  virtual nsResourceLoadDesc UpdateContent(nsStreamReader* Stream) override;
  virtual void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

  nsUInt8 m_uiLoadedTextures;
  nsGALTextureHandle m_hGALTexture[2];
  nsUInt32 m_uiMemoryGPU[2];

  nsGALResourceFormat::Enum m_Format;
  nsUInt32 m_uiWidthAndHeight;

  nsGALSamplerStateHandle m_hSamplerState;
};
