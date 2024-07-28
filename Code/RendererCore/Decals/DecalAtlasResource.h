#pragma once

#include <Core/ResourceManager/Resource.h>
#include <Core/ResourceManager/ResourceTypeLoader.h>
#include <Foundation/Math/Rect.h>
#include <RendererCore/RendererCoreDLL.h>
#include <Texture/Utils/TextureAtlasDesc.h>

using nsDecalAtlasResourceHandle = nsTypedResourceHandle<class nsDecalAtlasResource>;
using nsTexture2DResourceHandle = nsTypedResourceHandle<class nsTexture2DResource>;

class nsImage;

struct nsDecalAtlasResourceDescriptor
{
};

class NS_RENDERERCORE_DLL nsDecalAtlasResource : public nsResource
{
  NS_ADD_DYNAMIC_REFLECTION(nsDecalAtlasResource, nsResource);
  NS_RESOURCE_DECLARE_COMMON_CODE(nsDecalAtlasResource);
  NS_RESOURCE_DECLARE_CREATEABLE(nsDecalAtlasResource, nsDecalAtlasResourceDescriptor);

public:
  nsDecalAtlasResource();

  /// \brief Returns the one global decal atlas resource
  static nsDecalAtlasResourceHandle GetDecalAtlasResource();

  const nsTexture2DResourceHandle& GetBaseColorTexture() const { return m_hBaseColor; }
  const nsTexture2DResourceHandle& GetNormalTexture() const { return m_hNormal; }
  const nsTexture2DResourceHandle& GetORMTexture() const { return m_hORM; }
  const nsVec2U32& GetBaseColorTextureSize() const { return m_vBaseColorSize; }
  const nsVec2U32& GetNormalTextureSize() const { return m_vNormalSize; }
  const nsVec2U32& GetORMTextureSize() const { return m_vORMSize; }
  const nsTextureAtlasRuntimeDesc& GetAtlas() const { return m_Atlas; }

private:
  virtual nsResourceLoadDesc UnloadData(Unload WhatToUnload) override;
  virtual nsResourceLoadDesc UpdateContent(nsStreamReader* Stream) override;
  virtual void ReportResourceIsMissing() override;

  void ReadDecalInfo(nsStreamReader* Stream);

  virtual void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

  void CreateLayerTexture(const nsImage& img, bool bSRGB, nsTexture2DResourceHandle& out_hTexture);

  nsTextureAtlasRuntimeDesc m_Atlas;
  static nsUInt32 s_uiDecalAtlasResources;
  nsTexture2DResourceHandle m_hBaseColor;
  nsTexture2DResourceHandle m_hNormal;
  nsTexture2DResourceHandle m_hORM;
  nsVec2U32 m_vBaseColorSize;
  nsVec2U32 m_vNormalSize;
  nsVec2U32 m_vORMSize;
};
