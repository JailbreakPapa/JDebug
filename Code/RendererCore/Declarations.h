#pragma once

#include <Core/ResourceManager/ResourceHandle.h>
#include <Foundation/Strings/HashedString.h>
#include <RendererCore/RendererCoreDLL.h>

class nsShaderStageBinary;
struct nsVertexDeclarationInfo;

using nsTexture2DResourceHandle = nsTypedResourceHandle<class nsTexture2DResource>;
using nsRenderToTexture2DResourceHandle = nsTypedResourceHandle<class nsRenderToTexture2DResource>;
using nsTextureCubeResourceHandle = nsTypedResourceHandle<class nsTextureCubeResource>;
using nsMeshBufferResourceHandle = nsTypedResourceHandle<class nsMeshBufferResource>;
using nsDynamicMeshBufferResourceHandle = nsTypedResourceHandle<class nsDynamicMeshBufferResource>;
using nsMeshResourceHandle = nsTypedResourceHandle<class nsMeshResource>;
using nsMaterialResourceHandle = nsTypedResourceHandle<class nsMaterialResource>;
using nsShaderResourceHandle = nsTypedResourceHandle<class nsShaderResource>;
using nsShaderPermutationResourceHandle = nsTypedResourceHandle<class nsShaderPermutationResource>;
using nsRenderPipelineResourceHandle = nsTypedResourceHandle<class nsRenderPipelineResource>;
using nsDecalResourceHandle = nsTypedResourceHandle<class nsDecalResource>;
using nsDecalAtlasResourceHandle = nsTypedResourceHandle<class nsDecalAtlasResource>;

struct NS_RENDERERCORE_DLL nsPermutationVar
{
  NS_DECLARE_MEM_RELOCATABLE_TYPE();

  nsHashedString m_sName;
  nsHashedString m_sValue;

  NS_ALWAYS_INLINE bool operator==(const nsPermutationVar& other) const { return m_sName == other.m_sName && m_sValue == other.m_sValue; }
};
