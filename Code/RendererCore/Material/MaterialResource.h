#pragma once

#include <Core/ResourceManager/Resource.h>
#include <Foundation/Containers/HashTable.h>
#include <Foundation/Strings/HashedString.h>
#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderData.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>
#include <RendererCore/Shader/ShaderResource.h>

using nsMaterialResourceHandle = nsTypedResourceHandle<class nsMaterialResource>;
using nsTexture2DResourceHandle = nsTypedResourceHandle<class nsTexture2DResource>;
using nsTextureCubeResourceHandle = nsTypedResourceHandle<class nsTextureCubeResource>;

struct nsMaterialResourceDescriptor
{
  struct Parameter
  {
    nsHashedString m_Name;
    nsVariant m_Value;

    NS_FORCE_INLINE bool operator==(const Parameter& other) const { return m_Name == other.m_Name && m_Value == other.m_Value; }
  };

  struct Texture2DBinding
  {
    nsHashedString m_Name;
    nsTexture2DResourceHandle m_Value;

    NS_FORCE_INLINE bool operator==(const Texture2DBinding& other) const { return m_Name == other.m_Name && m_Value == other.m_Value; }
  };

  struct TextureCubeBinding
  {
    nsHashedString m_Name;
    nsTextureCubeResourceHandle m_Value;

    NS_FORCE_INLINE bool operator==(const TextureCubeBinding& other) const { return m_Name == other.m_Name && m_Value == other.m_Value; }
  };

  void Clear();

  bool operator==(const nsMaterialResourceDescriptor& other) const;
  NS_FORCE_INLINE bool operator!=(const nsMaterialResourceDescriptor& other) const { return !(*this == other); }

  nsMaterialResourceHandle m_hBaseMaterial;
  // nsSurfaceResource is not linked into this project (not true anymore -> could be changed)
  // this is not used for game purposes but rather for automatic collision mesh generation, so we only store the asset ID here
  nsHashedString m_sSurface;
  nsShaderResourceHandle m_hShader;
  nsDynamicArray<nsPermutationVar> m_PermutationVars;
  nsDynamicArray<Parameter> m_Parameters;
  nsDynamicArray<Texture2DBinding> m_Texture2DBindings;
  nsDynamicArray<TextureCubeBinding> m_TextureCubeBindings;
  nsRenderData::Category m_RenderDataCategory;
};

class NS_RENDERERCORE_DLL nsMaterialResource final : public nsResource
{
  NS_ADD_DYNAMIC_REFLECTION(nsMaterialResource, nsResource);
  NS_RESOURCE_DECLARE_COMMON_CODE(nsMaterialResource);
  NS_RESOURCE_DECLARE_CREATEABLE(nsMaterialResource, nsMaterialResourceDescriptor);

public:
  nsMaterialResource();
  ~nsMaterialResource();

  nsHashedString GetPermutationValue(const nsTempHashedString& sName);
  nsHashedString GetSurface() const;

  void SetParameter(const nsHashedString& sName, const nsVariant& value);
  void SetParameter(const char* szName, const nsVariant& value);
  nsVariant GetParameter(const nsTempHashedString& sName);

  void SetTexture2DBinding(const nsHashedString& sName, const nsTexture2DResourceHandle& value);
  void SetTexture2DBinding(const char* szName, const nsTexture2DResourceHandle& value);
  nsTexture2DResourceHandle GetTexture2DBinding(const nsTempHashedString& sName);

  void SetTextureCubeBinding(const nsHashedString& sName, const nsTextureCubeResourceHandle& value);
  void SetTextureCubeBinding(const char* szName, const nsTextureCubeResourceHandle& value);
  nsTextureCubeResourceHandle GetTextureCubeBinding(const nsTempHashedString& sName);

  nsRenderData::Category GetRenderDataCategory();

  /// \brief Copies current desc to original desc so the material is not modified on reset
  void PreserveCurrentDesc();
  virtual void ResetResource() override;

  const nsMaterialResourceDescriptor& GetCurrentDesc() const;

  /// \brief Use these enum values together with GetDefaultMaterialFileName() to get the default file names for these material types.
  enum class DefaultMaterialType
  {
    Fullbright,
    FullbrightAlphaTest,
    Lit,
    LitAlphaTest,
    Sky,
    MissingMaterial
  };

  /// \brief Returns the default material file name for the given type (materials in Data/Base/Materials/BaseMaterials).
  static const char* GetDefaultMaterialFileName(DefaultMaterialType materialType);

private:
  virtual nsResourceLoadDesc UnloadData(Unload WhatToUnload) override;
  virtual nsResourceLoadDesc UpdateContent(nsStreamReader* Stream) override;
  virtual void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

private:
  nsMaterialResourceDescriptor m_mOriginalDesc; // stores the state at loading, such that SetParameter etc. calls can be reset later
  nsMaterialResourceDescriptor m_mDesc;

  friend class nsRenderContext;
  NS_MAKE_SUBSYSTEM_STARTUP_FRIEND(RendererCore, MaterialResource);

  nsEvent<const nsMaterialResource*, nsMutex> m_ModifiedEvent;
  void OnBaseMaterialModified(const nsMaterialResource* pModifiedMaterial);
  void OnResourceEvent(const nsResourceEvent& resourceEvent);

  void AddPermutationVar(nsStringView sName, nsStringView sValue);

  nsAtomicInteger32 m_iLastModified;
  nsAtomicInteger32 m_iLastConstantsModified;
  nsInt32 m_iLastUpdated;
  nsInt32 m_iLastConstantsUpdated;

  bool IsModified();
  bool AreConstantsModified();

  void UpdateConstantBuffer(nsShaderPermutationResource* pShaderPermutation);

  nsConstantBufferStorageHandle m_hConstantBufferStorage;

  struct CachedValues
  {
    nsShaderResourceHandle m_hShader;
    nsHashTable<nsHashedString, nsHashedString> m_PermutationVars;
    nsHashTable<nsHashedString, nsVariant> m_Parameters;
    nsHashTable<nsHashedString, nsTexture2DResourceHandle> m_Texture2DBindings;
    nsHashTable<nsHashedString, nsTextureCubeResourceHandle> m_TextureCubeBindings;
    nsRenderData::Category m_RenderDataCategory;

    void Reset();
  };

  nsUInt32 m_uiCacheIndex;
  CachedValues* m_pCachedValues;

  CachedValues* GetOrUpdateCachedValues();
  static CachedValues* AllocateCache(nsUInt32& inout_uiCacheIndex);
  static void DeallocateCache(nsUInt32 uiCacheIndex);

  nsMutex m_UpdateCacheMutex;
  static nsDeque<nsMaterialResource::CachedValues> s_CachedValues;

  static void ClearCache();
};
