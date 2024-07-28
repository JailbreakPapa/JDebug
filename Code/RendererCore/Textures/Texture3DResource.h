#pragma once

#include <RendererCore/RendererCoreDLL.h>

#include <Foundation/IO/MemoryStream.h>

#include <Core/ResourceManager/Resource.h>
#include <Core/ResourceManager/ResourceTypeLoader.h>

#include <RendererFoundation/Descriptors/Descriptors.h>
#include <RendererFoundation/RendererFoundationDLL.h>

#include <RendererCore/Pipeline/Declarations.h>
#include <RendererCore/RenderContext/Implementation/RenderContextStructs.h>

class nsImage;

using nsTexture3DResourceHandle = nsTypedResourceHandle<class nsTexture3DResource>;

/// \brief Use this descriptor in calls to nsResourceManager::CreateResource<nsTexture3DResource> to create textures from data in memory.
struct NS_RENDERERCORE_DLL nsTexture3DResourceDescriptor
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

class NS_RENDERERCORE_DLL nsTexture3DResource : public nsResource
{
  NS_ADD_DYNAMIC_REFLECTION(nsTexture3DResource, nsResource);

  NS_RESOURCE_DECLARE_COMMON_CODE(nsTexture3DResource);
  NS_RESOURCE_DECLARE_CREATEABLE(nsTexture3DResource, nsTexture3DResourceDescriptor);

public:
  nsTexture3DResource();

  NS_ALWAYS_INLINE nsGALResourceFormat::Enum GetFormat() const { return m_Format; }
  NS_ALWAYS_INLINE nsUInt32 GetWidth() const { return m_uiWidth; }
  NS_ALWAYS_INLINE nsUInt32 GetHeight() const { return m_uiHeight; }
  NS_ALWAYS_INLINE nsUInt32 GetDepth() const { return m_uiDepth; }
  NS_ALWAYS_INLINE nsGALTextureType::Enum GetType() const { return m_Type; }

  static void FillOutDescriptor(nsTexture3DResourceDescriptor& ref_td, const nsImage* pImage, bool bSRGB, nsUInt32 uiNumMipLevels,
    nsUInt32& out_uiMemoryUsed, nsHybridArray<nsGALSystemMemoryDescription, 32>& ref_initData);

  const nsGALTextureHandle& GetGALTexture() const { return m_hGALTexture[m_uiLoadedTextures - 1]; }
  const nsGALSamplerStateHandle& GetGALSamplerState() const { return m_hSamplerState; }

protected:
  virtual nsResourceLoadDesc UnloadData(Unload WhatToUnload) override;
  virtual nsResourceLoadDesc UpdateContent(nsStreamReader* Stream) override;
  virtual void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

  nsTexture3DResource(DoUpdate ResourceUpdateThread);

  nsUInt8 m_uiLoadedTextures = 0;
  nsGALTextureHandle m_hGALTexture[2];
  nsUInt32 m_uiMemoryGPU[2] = {0, 0};

  nsGALTextureType::Enum m_Type = nsGALTextureType::Invalid;
  nsGALResourceFormat::Enum m_Format = nsGALResourceFormat::Invalid;
  nsUInt32 m_uiWidth = 0;
  nsUInt32 m_uiHeight = 0;
  nsUInt32 m_uiDepth = 0;

  nsGALSamplerStateHandle m_hSamplerState;
};
