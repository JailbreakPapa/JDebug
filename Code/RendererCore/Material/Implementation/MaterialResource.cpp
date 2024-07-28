#include <RendererCore/RendererCorePCH.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/IO/OpenDdlReader.h>
#include <Foundation/IO/OpenDdlUtils.h>
#include <Foundation/Types/ScopeExit.h>
#include <Foundation/Utilities/AssetFileHeader.h>
#include <RendererCore/Material/MaterialResource.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererCore/RenderWorld/RenderWorld.h>
#include <RendererCore/Shader/ShaderPermutationResource.h>
#include <RendererCore/ShaderCompiler/ShaderManager.h>
#include <RendererCore/Textures/Texture2DResource.h>
#include <RendererCore/Textures/TextureCubeResource.h>
#include <RendererCore/Textures/TextureLoader.h>
#include <Texture/Image/Formats/DdsFileFormat.h>

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
#  include <Foundation/IO/CompressedStreamZstd.h>
#endif

void nsMaterialResourceDescriptor::Clear()
{
  m_hBaseMaterial.Invalidate();
  m_sSurface.Clear();
  m_hShader.Invalidate();
  m_PermutationVars.Clear();
  m_Parameters.Clear();
  m_Texture2DBindings.Clear();
  m_TextureCubeBindings.Clear();
  m_RenderDataCategory = nsInvalidRenderDataCategory;
}

bool nsMaterialResourceDescriptor::operator==(const nsMaterialResourceDescriptor& other) const
{
  return m_hBaseMaterial == other.m_hBaseMaterial &&
         m_hShader == other.m_hShader &&
         m_PermutationVars == other.m_PermutationVars &&
         m_Parameters == other.m_Parameters &&
         m_Texture2DBindings == other.m_Texture2DBindings &&
         m_TextureCubeBindings == other.m_TextureCubeBindings &&
         m_RenderDataCategory == other.m_RenderDataCategory;
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMaterialResource, 1, nsRTTIDefaultAllocator<nsMaterialResource>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_RESOURCE_IMPLEMENT_COMMON_CODE(nsMaterialResource);
// clang-format on

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(RendererCore, MaterialResource)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "Foundation",
    "Core"
  END_SUBSYSTEM_DEPENDENCIES

  ON_HIGHLEVELSYSTEMS_SHUTDOWN
  {
    nsMaterialResource::ClearCache();
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

nsDeque<nsMaterialResource::CachedValues> nsMaterialResource::s_CachedValues;

nsMaterialResource::nsMaterialResource()
  : nsResource(DoUpdate::OnAnyThread, 1)
{
  m_iLastUpdated = 0;
  m_iLastConstantsUpdated = 0;
  m_uiCacheIndex = nsInvalidIndex;
  m_pCachedValues = nullptr;

  nsResourceManager::GetResourceEvents().AddEventHandler(nsMakeDelegate(&nsMaterialResource::OnResourceEvent, this));
}

nsMaterialResource::~nsMaterialResource()
{
  nsResourceManager::GetResourceEvents().RemoveEventHandler(nsMakeDelegate(&nsMaterialResource::OnResourceEvent, this));
}

nsHashedString nsMaterialResource::GetPermutationValue(const nsTempHashedString& sName)
{
  auto pCachedValues = GetOrUpdateCachedValues();

  nsHashedString sResult;
  pCachedValues->m_PermutationVars.TryGetValue(sName, sResult);

  return sResult;
}

nsHashedString nsMaterialResource::GetSurface() const
{
  if (!m_mDesc.m_sSurface.IsEmpty())
    return m_mDesc.m_sSurface;

  if (m_mDesc.m_hBaseMaterial.IsValid())
  {
    nsResourceLock<nsMaterialResource> pBaseMaterial(m_mDesc.m_hBaseMaterial, nsResourceAcquireMode::BlockTillLoaded);
    return pBaseMaterial->GetSurface();
  }

  return nsHashedString();
}

void nsMaterialResource::SetParameter(const nsHashedString& sName, const nsVariant& value)
{
  nsUInt32 uiIndex = nsInvalidIndex;
  for (nsUInt32 i = 0; i < m_mDesc.m_Parameters.GetCount(); ++i)
  {
    if (m_mDesc.m_Parameters[i].m_Name == sName)
    {
      uiIndex = i;
      break;
    }
  }

  if (value.IsValid())
  {
    if (uiIndex != nsInvalidIndex)
    {
      if (m_mDesc.m_Parameters[uiIndex].m_Value == value)
      {
        return;
      }

      m_mDesc.m_Parameters[uiIndex].m_Value = value;
    }
    else
    {
      auto& param = m_mDesc.m_Parameters.ExpandAndGetRef();
      param.m_Name = sName;
      param.m_Value = value;
    }
  }
  else
  {
    if (uiIndex == nsInvalidIndex)
    {
      return;
    }

    m_mDesc.m_Parameters.RemoveAtAndSwap(uiIndex);
  }

  m_iLastModified.Increment();
  m_iLastConstantsModified.Increment();

  m_ModifiedEvent.Broadcast(this);
}

void nsMaterialResource::SetParameter(const char* szName, const nsVariant& value)
{
  nsTempHashedString sName(szName);

  nsUInt32 uiIndex = nsInvalidIndex;
  for (nsUInt32 i = 0; i < m_mDesc.m_Parameters.GetCount(); ++i)
  {
    if (m_mDesc.m_Parameters[i].m_Name == sName)
    {
      uiIndex = i;
      break;
    }
  }

  if (value.IsValid())
  {
    if (uiIndex != nsInvalidIndex)
    {
      if (m_mDesc.m_Parameters[uiIndex].m_Value == value)
      {
        return;
      }

      m_mDesc.m_Parameters[uiIndex].m_Value = value;
    }
    else
    {
      auto& param = m_mDesc.m_Parameters.ExpandAndGetRef();
      param.m_Name.Assign(szName);
      param.m_Value = value;
    }
  }
  else
  {
    if (uiIndex == nsInvalidIndex)
    {
      return;
    }

    m_mDesc.m_Parameters.RemoveAtAndSwap(uiIndex);
  }

  m_iLastModified.Increment();
  m_iLastConstantsModified.Increment();

  m_ModifiedEvent.Broadcast(this);
}

nsVariant nsMaterialResource::GetParameter(const nsTempHashedString& sName)
{
  auto pCachedValues = GetOrUpdateCachedValues();

  nsVariant value;
  pCachedValues->m_Parameters.TryGetValue(sName, value);

  return value;
}

void nsMaterialResource::SetTexture2DBinding(const nsHashedString& sName, const nsTexture2DResourceHandle& value)
{
  nsUInt32 uiIndex = nsInvalidIndex;
  for (nsUInt32 i = 0; i < m_mDesc.m_Texture2DBindings.GetCount(); ++i)
  {
    if (m_mDesc.m_Texture2DBindings[i].m_Name == sName)
    {
      uiIndex = i;
      break;
    }
  }

  if (value.IsValid())
  {
    if (uiIndex != nsInvalidIndex)
    {
      m_mDesc.m_Texture2DBindings[uiIndex].m_Value = value;
    }
    else
    {
      auto& binding = m_mDesc.m_Texture2DBindings.ExpandAndGetRef();
      binding.m_Name = sName;
      binding.m_Value = value;
    }
  }
  else
  {
    if (uiIndex != nsInvalidIndex)
    {
      m_mDesc.m_Texture2DBindings.RemoveAtAndSwap(uiIndex);
    }
  }

  m_iLastModified.Increment();

  m_ModifiedEvent.Broadcast(this);
}

void nsMaterialResource::SetTexture2DBinding(const char* szName, const nsTexture2DResourceHandle& value)
{
  nsTempHashedString sName(szName);

  nsUInt32 uiIndex = nsInvalidIndex;
  for (nsUInt32 i = 0; i < m_mDesc.m_Texture2DBindings.GetCount(); ++i)
  {
    if (m_mDesc.m_Texture2DBindings[i].m_Name == sName)
    {
      uiIndex = i;
      break;
    }
  }

  if (value.IsValid())
  {
    if (uiIndex != nsInvalidIndex)
    {
      m_mDesc.m_Texture2DBindings[uiIndex].m_Value = value;
    }
    else
    {
      auto& binding = m_mDesc.m_Texture2DBindings.ExpandAndGetRef();
      binding.m_Name.Assign(szName);
      binding.m_Value = value;
    }
  }
  else
  {
    if (uiIndex != nsInvalidIndex)
    {
      m_mDesc.m_Texture2DBindings.RemoveAtAndSwap(uiIndex);
    }
  }

  m_iLastModified.Increment();

  m_ModifiedEvent.Broadcast(this);
}

nsTexture2DResourceHandle nsMaterialResource::GetTexture2DBinding(const nsTempHashedString& sName)
{
  auto pCachedValues = GetOrUpdateCachedValues();

  // Use pointer to prevent ref counting
  nsTexture2DResourceHandle* pBinding;
  if (pCachedValues->m_Texture2DBindings.TryGetValue(sName, pBinding))
  {
    return *pBinding;
  }

  return nsTexture2DResourceHandle();
}


void nsMaterialResource::SetTextureCubeBinding(const nsHashedString& sName, const nsTextureCubeResourceHandle& value)
{
  nsUInt32 uiIndex = nsInvalidIndex;
  for (nsUInt32 i = 0; i < m_mDesc.m_TextureCubeBindings.GetCount(); ++i)
  {
    if (m_mDesc.m_TextureCubeBindings[i].m_Name == sName)
    {
      uiIndex = i;
      break;
    }
  }

  if (value.IsValid())
  {
    if (uiIndex != nsInvalidIndex)
    {
      m_mDesc.m_TextureCubeBindings[uiIndex].m_Value = value;
    }
    else
    {
      auto& binding = m_mDesc.m_TextureCubeBindings.ExpandAndGetRef();
      binding.m_Name = sName;
      binding.m_Value = value;
    }
  }
  else
  {
    if (uiIndex != nsInvalidIndex)
    {
      m_mDesc.m_TextureCubeBindings.RemoveAtAndSwap(uiIndex);
    }
  }

  m_iLastModified.Increment();

  m_ModifiedEvent.Broadcast(this);
}

void nsMaterialResource::SetTextureCubeBinding(const char* szName, const nsTextureCubeResourceHandle& value)
{
  nsTempHashedString sName(szName);

  nsUInt32 uiIndex = nsInvalidIndex;
  for (nsUInt32 i = 0; i < m_mDesc.m_TextureCubeBindings.GetCount(); ++i)
  {
    if (m_mDesc.m_TextureCubeBindings[i].m_Name == sName)
    {
      uiIndex = i;
      break;
    }
  }

  if (value.IsValid())
  {
    if (uiIndex != nsInvalidIndex)
    {
      m_mDesc.m_TextureCubeBindings[uiIndex].m_Value = value;
    }
    else
    {
      auto& binding = m_mDesc.m_TextureCubeBindings.ExpandAndGetRef();
      binding.m_Name.Assign(szName);
      binding.m_Value = value;
    }
  }
  else
  {
    if (uiIndex != nsInvalidIndex)
    {
      m_mDesc.m_TextureCubeBindings.RemoveAtAndSwap(uiIndex);
    }
  }

  m_iLastModified.Increment();

  m_ModifiedEvent.Broadcast(this);
}

nsTextureCubeResourceHandle nsMaterialResource::GetTextureCubeBinding(const nsTempHashedString& sName)
{
  auto pCachedValues = GetOrUpdateCachedValues();

  // Use pointer to prevent ref counting
  nsTextureCubeResourceHandle* pBinding;
  if (pCachedValues->m_TextureCubeBindings.TryGetValue(sName, pBinding))
  {
    return *pBinding;
  }

  return nsTextureCubeResourceHandle();
}

nsRenderData::Category nsMaterialResource::GetRenderDataCategory()
{
  auto pCachedValues = GetOrUpdateCachedValues();
  return pCachedValues->m_RenderDataCategory;
}

void nsMaterialResource::PreserveCurrentDesc()
{
  m_mOriginalDesc = m_mDesc;
}

void nsMaterialResource::ResetResource()
{
  if (m_mDesc != m_mOriginalDesc)
  {
    m_mDesc = m_mOriginalDesc;

    m_iLastModified.Increment();
    m_iLastConstantsModified.Increment();

    m_ModifiedEvent.Broadcast(this);
  }
}

const char* nsMaterialResource::GetDefaultMaterialFileName(DefaultMaterialType materialType)
{
  switch (materialType)
  {
    case DefaultMaterialType::Fullbright:
      return "Base/Materials/BaseMaterials/Fullbright.nsMaterialAsset";
    case DefaultMaterialType::FullbrightAlphaTest:
      return "Base/Materials/BaseMaterials/FullbrightAlphaTest.nsMaterialAsset";
    case DefaultMaterialType::Lit:
      return "Base/Materials/BaseMaterials/Lit.nsMaterialAsset";
    case DefaultMaterialType::LitAlphaTest:
      return "Base/Materials/BaseMaterials/LitAlphaTest.nsMaterialAsset";
    case DefaultMaterialType::Sky:
      return "Base/Materials/BaseMaterials/Sky.nsMaterialAsset";
    case DefaultMaterialType::MissingMaterial:
      return "Base/Materials/Common/MissingMaterial.nsMaterialAsset";
    default:
      NS_ASSERT_NOT_IMPLEMENTED;
      return "";
  }
}

nsResourceLoadDesc nsMaterialResource::UnloadData(Unload WhatToUnload)
{
  if (m_mDesc.m_hBaseMaterial.IsValid())
  {
    nsResourceLock<nsMaterialResource> pBaseMaterial(m_mDesc.m_hBaseMaterial, nsResourceAcquireMode::PointerOnly);

    auto d = nsMakeDelegate(&nsMaterialResource::OnBaseMaterialModified, this);
    if (pBaseMaterial->m_ModifiedEvent.HasEventHandler(d))
    {
      pBaseMaterial->m_ModifiedEvent.RemoveEventHandler(d);
    }
  }

  m_mDesc.Clear();
  m_mOriginalDesc.Clear();

  if (!m_hConstantBufferStorage.IsInvalidated())
  {
    nsRenderContext::DeleteConstantBufferStorage(m_hConstantBufferStorage);
    m_hConstantBufferStorage.Invalidate();
  }

  DeallocateCache(m_uiCacheIndex);
  m_uiCacheIndex = nsInvalidIndex;
  m_pCachedValues = nullptr;

  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;
  res.m_State = nsResourceState::Unloaded;

  return res;
}

nsResourceLoadDesc nsMaterialResource::UpdateContent(nsStreamReader* pOuterStream)
{
  m_mDesc.Clear();
  m_mOriginalDesc.Clear();

  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;
  res.m_State = nsResourceState::Loaded;

  if (pOuterStream == nullptr)
  {
    res.m_State = nsResourceState::LoadedResourceMissing;
    return res;
  }

  nsStringBuilder sAbsFilePath;
  (*pOuterStream) >> sAbsFilePath;

  if (sAbsFilePath.HasExtension("nsMaterialBin"))
  {
    nsStringBuilder sTemp, sTemp2;

    nsAssetFileHeader AssetHash;
    AssetHash.Read(*pOuterStream).IgnoreResult();

    nsUInt8 uiVersion = 0;
    (*pOuterStream) >> uiVersion;
    NS_ASSERT_DEV(uiVersion >= 4 && uiVersion <= 7, "Unknown nsMaterialBin version {0}", uiVersion);

    nsUInt8 uiCompressionMode = 0;
    if (uiVersion >= 6)
    {
      *pOuterStream >> uiCompressionMode;
    }

    nsStreamReader* pInnerStream = pOuterStream;

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
    nsCompressedStreamReaderZstd decompressorZstd;
#endif

    switch (uiCompressionMode)
    {
      case 0:
        break;

      case 1:
#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
        decompressorZstd.SetInputStream(pOuterStream);
        pInnerStream = &decompressorZstd;
        break;
#else
        nsLog::Error("Material resource is compressed with zstandard, but support for this compressor is not compiled in.");
        res.m_State = nsResourceState::LoadedResourceMissing;
        return res;
#endif

      default:
        nsLog::Error("Material resource is compressed with an unknown algorithm.");
        res.m_State = nsResourceState::LoadedResourceMissing;
        return res;
    }

    nsStreamReader& s = *pInnerStream;

    // Base material
    {
      s >> sTemp;

      if (!sTemp.IsEmpty())
        m_mDesc.m_hBaseMaterial = nsResourceManager::LoadResource<nsMaterialResource>(sTemp);
    }

    // Surface
    {
      s >> sTemp;
      m_mDesc.m_sSurface.Assign(sTemp.GetView());
    }

    // Shader
    {
      s >> sTemp;

      if (!sTemp.IsEmpty())
        m_mDesc.m_hShader = nsResourceManager::LoadResource<nsShaderResource>(sTemp);
    }

    // Permutation Variables
    {
      nsUInt16 uiPermVars;
      s >> uiPermVars;

      m_mDesc.m_PermutationVars.Reserve(uiPermVars);

      for (nsUInt16 i = 0; i < uiPermVars; ++i)
      {
        s >> sTemp;
        s >> sTemp2;

        if (!sTemp.IsEmpty() && !sTemp2.IsEmpty())
        {
          AddPermutationVar(sTemp, sTemp2);
        }
      }
    }

    // 2D Textures
    {
      nsUInt16 uiTextures = 0;
      s >> uiTextures;

      m_mDesc.m_Texture2DBindings.Reserve(uiTextures);

      for (nsUInt16 i = 0; i < uiTextures; ++i)
      {
        s >> sTemp;
        s >> sTemp2;

        if (!sTemp.IsEmpty() && !sTemp2.IsEmpty())
        {
          nsMaterialResourceDescriptor::Texture2DBinding& tc = m_mDesc.m_Texture2DBindings.ExpandAndGetRef();
          tc.m_Name.Assign(sTemp.GetData());
          tc.m_Value = nsResourceManager::LoadResource<nsTexture2DResource>(sTemp2);
        }
      }
    }

    // Cube Textures
    {
      nsUInt16 uiTextures = 0;
      s >> uiTextures;

      m_mDesc.m_TextureCubeBindings.Reserve(uiTextures);

      for (nsUInt16 i = 0; i < uiTextures; ++i)
      {
        s >> sTemp;
        s >> sTemp2;

        if (!sTemp.IsEmpty() && !sTemp2.IsEmpty())
        {
          nsMaterialResourceDescriptor::TextureCubeBinding& tc = m_mDesc.m_TextureCubeBindings.ExpandAndGetRef();
          tc.m_Name.Assign(sTemp.GetData());
          tc.m_Value = nsResourceManager::LoadResource<nsTextureCubeResource>(sTemp2);
        }
      }
    }

    // Shader constants
    {
      nsUInt16 uiConstants = 0;
      s >> uiConstants;

      m_mDesc.m_Parameters.Reserve(uiConstants);

      nsVariant vTemp;

      for (nsUInt16 i = 0; i < uiConstants; ++i)
      {
        s >> sTemp;
        s >> vTemp;

        if (!sTemp.IsEmpty() && vTemp.IsValid())
        {
          nsMaterialResourceDescriptor::Parameter& tc = m_mDesc.m_Parameters.ExpandAndGetRef();
          tc.m_Name.Assign(sTemp.GetData());
          tc.m_Value = vTemp;
        }
      }
    }

    // Render data category
    if (uiVersion >= 7)
    {
      nsStringBuilder sRenderDataCategoryName;
      s >> sRenderDataCategoryName;

      nsTempHashedString sCategoryNameHashed(sRenderDataCategoryName.GetView());
      if (sCategoryNameHashed != nsTempHashedString("<Invalid>"))
      {
        m_mDesc.m_RenderDataCategory = nsRenderData::FindCategory(sCategoryNameHashed);
        if (m_mDesc.m_RenderDataCategory == nsInvalidRenderDataCategory)
        {
          nsLog::Error("Material '{}' uses an invalid render data category '{}'", GetResourceIdOrDescription(), sRenderDataCategoryName);
        }
      }
    }

    if (uiVersion >= 5)
    {
      nsStreamReader& s = *pInnerStream;

      nsStringBuilder sResourceName;
      s >> sResourceName;

      nsTextureResourceLoader::LoadedData embedded;

      while (!sResourceName.IsEmpty())
      {
        nsUInt32 dataSize = 0;
        s >> dataSize;

        nsTextureResourceLoader::LoadTexFile(s, embedded).IgnoreResult();
        embedded.m_bIsFallback = true;

        nsDefaultMemoryStreamStorage storage;
        nsMemoryStreamWriter loadStreamWriter(&storage);
        nsTextureResourceLoader::WriteTextureLoadStream(loadStreamWriter, embedded);

        nsMemoryStreamReader loadStreamReader(&storage);

        nsTexture2DResourceHandle hTexture = nsResourceManager::LoadResource<nsTexture2DResource>(sResourceName);
        nsResourceManager::SetResourceLowResData(hTexture, &loadStreamReader);

        s >> sResourceName;
      }
    }
  }

  if (sAbsFilePath.HasExtension("nsMaterial"))
  {
    nsOpenDdlReader reader;

    if (reader.ParseDocument(*pOuterStream, 0, nsLog::GetThreadLocalLogSystem()).Failed())
    {
      res.m_State = nsResourceState::LoadedResourceMissing;
      return res;
    }

    const nsOpenDdlReaderElement* pRoot = reader.GetRootElement();

    // Read the base material
    if (const nsOpenDdlReaderElement* pBase = pRoot->FindChildOfType(nsOpenDdlPrimitiveType::String, "BaseMaterial"))
    {
      m_mDesc.m_hBaseMaterial = nsResourceManager::LoadResource<nsMaterialResource>(pBase->GetPrimitivesString()[0]);
    }

    // Read the shader
    if (const nsOpenDdlReaderElement* pShader = pRoot->FindChildOfType(nsOpenDdlPrimitiveType::String, "Shader"))
    {
      m_mDesc.m_hShader = nsResourceManager::LoadResource<nsShaderResource>(pShader->GetPrimitivesString()[0]);
    }

    // Read the render data category
    if (const nsOpenDdlReaderElement* pRenderDataCategory = pRoot->FindChildOfType(nsOpenDdlPrimitiveType::String, "RenderDataCategory"))
    {
      m_mDesc.m_RenderDataCategory = nsRenderData::FindCategory(nsTempHashedString(pRenderDataCategory->GetPrimitivesString()[0]));
    }

    for (const nsOpenDdlReaderElement* pChild = pRoot->GetFirstChild(); pChild != nullptr; pChild = pChild->GetSibling())
    {
      // Read the shader permutation variables
      if (pChild->IsCustomType("Permutation"))
      {
        const nsOpenDdlReaderElement* pName = pChild->FindChildOfType(nsOpenDdlPrimitiveType::String, "Variable");
        const nsOpenDdlReaderElement* pValue = pChild->FindChildOfType(nsOpenDdlPrimitiveType::String, "Value");

        if (pName && pValue)
        {
          AddPermutationVar(pName->GetPrimitivesString()[0], pValue->GetPrimitivesString()[0]);
        }
      }

      // Read the shader constants
      if (pChild->IsCustomType("Constant"))
      {
        const nsOpenDdlReaderElement* pName = pChild->FindChildOfType(nsOpenDdlPrimitiveType::String, "Variable");
        const nsOpenDdlReaderElement* pValue = pChild->FindChild("Value");

        nsVariant value;
        if (pName && pValue && nsOpenDdlUtils::ConvertToVariant(pValue, value).Succeeded())
        {
          nsMaterialResourceDescriptor::Parameter& sc = m_mDesc.m_Parameters.ExpandAndGetRef();
          sc.m_Name.Assign(pName->GetPrimitivesString()[0]);
          sc.m_Value = value;
        }
      }

      // Read the texture references
      if (pChild->IsCustomType("Texture2D"))
      {
        const nsOpenDdlReaderElement* pName = pChild->FindChildOfType(nsOpenDdlPrimitiveType::String, "Variable");
        const nsOpenDdlReaderElement* pValue = pChild->FindChildOfType(nsOpenDdlPrimitiveType::String, "Value");

        if (pName && pValue)
        {
          nsMaterialResourceDescriptor::Texture2DBinding& tc = m_mDesc.m_Texture2DBindings.ExpandAndGetRef();
          tc.m_Name.Assign(pName->GetPrimitivesString()[0]);
          tc.m_Value = nsResourceManager::LoadResource<nsTexture2DResource>(pValue->GetPrimitivesString()[0]);
        }
      }

      // Read the texture references
      if (pChild->IsCustomType("TextureCube"))
      {
        const nsOpenDdlReaderElement* pName = pChild->FindChildOfType(nsOpenDdlPrimitiveType::String, "Variable");
        const nsOpenDdlReaderElement* pValue = pChild->FindChildOfType(nsOpenDdlPrimitiveType::String, "Value");

        if (pName && pValue)
        {
          nsMaterialResourceDescriptor::TextureCubeBinding& tc = m_mDesc.m_TextureCubeBindings.ExpandAndGetRef();
          tc.m_Name.Assign(pName->GetPrimitivesString()[0]);
          tc.m_Value = nsResourceManager::LoadResource<nsTextureCubeResource>(pValue->GetPrimitivesString()[0]);
        }
      }
    }
  }

  if (m_mDesc.m_hBaseMaterial.IsValid())
  {
    // Block till the base material has been fully loaded to ensure that all parameters have their final value once this material is loaded.
    nsResourceLock<nsMaterialResource> pBaseMaterial(m_mDesc.m_hBaseMaterial, nsResourceAcquireMode::BlockTillLoaded);

    if (!pBaseMaterial->m_ModifiedEvent.HasEventHandler(nsMakeDelegate(&nsMaterialResource::OnBaseMaterialModified, this)))
    {
      pBaseMaterial->m_ModifiedEvent.AddEventHandler(nsMakeDelegate(&nsMaterialResource::OnBaseMaterialModified, this));
    }
  }

  m_mOriginalDesc = m_mDesc;

  m_iLastModified.Increment();
  m_iLastConstantsModified.Increment();

  m_ModifiedEvent.Broadcast(this);

  return res;
}

void nsMaterialResource::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
{
  out_NewMemoryUsage.m_uiMemoryCPU =
    sizeof(nsMaterialResource) + (nsUInt32)(m_mDesc.m_PermutationVars.GetHeapMemoryUsage() + m_mDesc.m_Parameters.GetHeapMemoryUsage() + m_mDesc.m_Texture2DBindings.GetHeapMemoryUsage() + m_mDesc.m_TextureCubeBindings.GetHeapMemoryUsage() + m_mOriginalDesc.m_PermutationVars.GetHeapMemoryUsage() +
                                            m_mOriginalDesc.m_Parameters.GetHeapMemoryUsage() + m_mOriginalDesc.m_Texture2DBindings.GetHeapMemoryUsage() + m_mOriginalDesc.m_TextureCubeBindings.GetHeapMemoryUsage());

  out_NewMemoryUsage.m_uiMemoryGPU = 0;
}

NS_RESOURCE_IMPLEMENT_CREATEABLE(nsMaterialResource, nsMaterialResourceDescriptor)
{
  m_mDesc = descriptor;
  m_mOriginalDesc = descriptor;

  nsResourceLoadDesc res;
  res.m_State = nsResourceState::Loaded;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;

  if (m_mDesc.m_hBaseMaterial.IsValid())
  {
    // Can't block here for the base material since this would result in a deadlock
    nsResourceLock<nsMaterialResource> pBaseMaterial(m_mDesc.m_hBaseMaterial, nsResourceAcquireMode::PointerOnly);
    pBaseMaterial->m_ModifiedEvent.AddEventHandler(nsMakeDelegate(&nsMaterialResource::OnBaseMaterialModified, this));
  }

  m_iLastModified.Increment();
  m_iLastConstantsModified.Increment();

  return res;
}

void nsMaterialResource::OnBaseMaterialModified(const nsMaterialResource* pModifiedMaterial)
{
  NS_ASSERT_DEV(m_mDesc.m_hBaseMaterial == pModifiedMaterial, "Implementation error");

  m_iLastModified.Increment();
  m_iLastConstantsModified.Increment();

  m_ModifiedEvent.Broadcast(this);
}

void nsMaterialResource::OnResourceEvent(const nsResourceEvent& resourceEvent)
{
  if (resourceEvent.m_Type != nsResourceEvent::Type::ResourceContentUpdated)
    return;

  if (m_pCachedValues != nullptr && m_pCachedValues->m_hShader == resourceEvent.m_pResource)
  {
    m_iLastConstantsModified.Increment();
  }
}

void nsMaterialResource::AddPermutationVar(nsStringView sName, nsStringView sValue)
{
  nsHashedString sNameHashed;
  sNameHashed.Assign(sName);
  nsHashedString sValueHashed;
  sValueHashed.Assign(sValue);

  if (nsShaderManager::IsPermutationValueAllowed(sNameHashed, sValueHashed))
  {
    nsPermutationVar& pv = m_mDesc.m_PermutationVars.ExpandAndGetRef();
    pv.m_sName = sNameHashed;
    pv.m_sValue = sValueHashed;
  }
}

bool nsMaterialResource::IsModified()
{
  return m_iLastModified != m_iLastUpdated;
}

bool nsMaterialResource::AreConstantsModified()
{
  return m_iLastConstantsModified != m_iLastConstantsUpdated;
}

void nsMaterialResource::UpdateConstantBuffer(nsShaderPermutationResource* pShaderPermutation)
{
  if (pShaderPermutation == nullptr)
    return;

  const nsGALShader* pShader = nsGALDevice::GetDefaultDevice()->GetShader(pShaderPermutation->GetGALShader());
  if (pShader == nullptr)
    return;

  nsTempHashedString sConstantBufferName("nsMaterialConstants");

  const nsShaderResourceBinding* pBinding = pShader->GetShaderResourceBinding(sConstantBufferName);
  const nsShaderConstantBufferLayout* pLayout = pBinding != nullptr ? pBinding->m_pLayout : nullptr;
  if (pLayout == nullptr)
    return;

  auto pCachedValues = GetOrUpdateCachedValues();

  m_iLastConstantsUpdated = m_iLastConstantsModified;

  if (m_hConstantBufferStorage.IsInvalidated())
  {
    m_hConstantBufferStorage = nsRenderContext::CreateConstantBufferStorage(pLayout->m_uiTotalSize);
  }

  nsConstantBufferStorageBase* pStorage = nullptr;
  if (nsRenderContext::TryGetConstantBufferStorage(m_hConstantBufferStorage, pStorage))
  {
    nsArrayPtr<nsUInt8> data = pStorage->GetRawDataForWriting();
    if (data.GetCount() != pLayout->m_uiTotalSize)
    {
      nsRenderContext::DeleteConstantBufferStorage(m_hConstantBufferStorage);
      m_hConstantBufferStorage = nsRenderContext::CreateConstantBufferStorage(pLayout->m_uiTotalSize);

      NS_VERIFY(nsRenderContext::TryGetConstantBufferStorage(m_hConstantBufferStorage, pStorage), "");
    }

    for (auto& constant : pLayout->m_Constants)
    {
      if (constant.m_uiOffset + nsShaderConstant::s_TypeSize[constant.m_Type.GetValue()] <= data.GetCount())
      {
        nsUInt8* pDest = &data[constant.m_uiOffset];

        nsVariant* pValue = nullptr;
        pCachedValues->m_Parameters.TryGetValue(constant.m_sName, pValue);

        constant.CopyDataFormVariant(pDest, pValue);
      }
    }
  }
}

nsMaterialResource::CachedValues* nsMaterialResource::GetOrUpdateCachedValues()
{
  if (!IsModified())
  {
    NS_ASSERT_DEV(m_pCachedValues != nullptr, "");
    return m_pCachedValues;
  }

  nsHybridArray<nsMaterialResource*, 16> materialHierarchy;
  nsMaterialResource* pCurrentMaterial = this;

  while (true)
  {
    materialHierarchy.PushBack(pCurrentMaterial);

    const nsMaterialResourceHandle& hBaseMaterial = pCurrentMaterial->m_mDesc.m_hBaseMaterial;
    if (!hBaseMaterial.IsValid())
      break;

    // Ensure that the base material is loaded at this point.
    // For loaded materials this will always be the case but is still necessary for runtime created materials.
    pCurrentMaterial = nsResourceManager::BeginAcquireResource(hBaseMaterial, nsResourceAcquireMode::BlockTillLoaded);
  }

  NS_SCOPE_EXIT(for (nsUInt32 i = materialHierarchy.GetCount(); i-- > 1;) {
    nsMaterialResource* pMaterial = materialHierarchy[i];
    nsResourceManager::EndAcquireResource(pMaterial);

    materialHierarchy[i] = nullptr;
  });

  NS_LOCK(m_UpdateCacheMutex);

  if (!IsModified())
  {
    NS_ASSERT_DEV(m_pCachedValues != nullptr, "");
    return m_pCachedValues;
  }

  m_pCachedValues = AllocateCache(m_uiCacheIndex);

  // set state of parent material first
  for (nsUInt32 i = materialHierarchy.GetCount(); i-- > 0;)
  {
    nsMaterialResource* pMaterial = materialHierarchy[i];
    const nsMaterialResourceDescriptor& desc = pMaterial->m_mDesc;

    if (desc.m_hShader.IsValid())
      m_pCachedValues->m_hShader = desc.m_hShader;

    for (const auto& permutationVar : desc.m_PermutationVars)
    {
      m_pCachedValues->m_PermutationVars.Insert(permutationVar.m_sName, permutationVar.m_sValue);
    }

    for (const auto& param : desc.m_Parameters)
    {
      m_pCachedValues->m_Parameters.Insert(param.m_Name, param.m_Value);
    }

    for (const auto& textureBinding : desc.m_Texture2DBindings)
    {
      m_pCachedValues->m_Texture2DBindings.Insert(textureBinding.m_Name, textureBinding.m_Value);
    }

    for (const auto& textureBinding : desc.m_TextureCubeBindings)
    {
      m_pCachedValues->m_TextureCubeBindings.Insert(textureBinding.m_Name, textureBinding.m_Value);
    }

    if (desc.m_RenderDataCategory != nsInvalidRenderDataCategory)
    {
      m_pCachedValues->m_RenderDataCategory = desc.m_RenderDataCategory;
    }
  }

  if (m_pCachedValues->m_RenderDataCategory == nsInvalidRenderDataCategory)
  {
    nsHashedString sBlendModeValue;
    if (m_pCachedValues->m_PermutationVars.TryGetValue("BLEND_MODE", sBlendModeValue))
    {
      if (sBlendModeValue == nsTempHashedString("BLEND_MODE_OPAQUE"))
      {
        m_pCachedValues->m_RenderDataCategory = nsDefaultRenderDataCategories::LitOpaque;
      }
      else if (sBlendModeValue == nsTempHashedString("BLEND_MODE_MASKED"))
      {
        m_pCachedValues->m_RenderDataCategory = nsDefaultRenderDataCategories::LitMasked;
      }
      else
      {
        m_pCachedValues->m_RenderDataCategory = nsDefaultRenderDataCategories::LitTransparent;
      }
    }
    else
    {
      m_pCachedValues->m_RenderDataCategory = nsDefaultRenderDataCategories::LitOpaque;
    }
  }

  m_iLastUpdated = m_iLastModified;
  return m_pCachedValues;
}

namespace
{
  static nsMutex s_MaterialCacheMutex;

  struct FreeCacheEntry
  {
    NS_DECLARE_POD_TYPE();

    nsUInt32 m_uiIndex;
    nsUInt64 m_uiFrame;
  };

  static nsDynamicArray<FreeCacheEntry, nsStaticsAllocatorWrapper> s_FreeMaterialCacheEntries;
} // namespace

void nsMaterialResource::CachedValues::Reset()
{
  m_hShader.Invalidate();
  m_PermutationVars.Clear();
  m_Parameters.Clear();
  m_Texture2DBindings.Clear();
  m_TextureCubeBindings.Clear();
  m_RenderDataCategory = nsInvalidRenderDataCategory;
}

// static
nsMaterialResource::CachedValues* nsMaterialResource::AllocateCache(nsUInt32& inout_uiCacheIndex)
{
  NS_LOCK(s_MaterialCacheMutex);

  nsUInt32 uiOldCacheIndex = inout_uiCacheIndex;

  nsUInt64 uiCurrentFrame = nsRenderWorld::GetFrameCounter();
  if (!s_FreeMaterialCacheEntries.IsEmpty() && s_FreeMaterialCacheEntries[0].m_uiFrame < uiCurrentFrame)
  {
    inout_uiCacheIndex = s_FreeMaterialCacheEntries[0].m_uiIndex;
    s_FreeMaterialCacheEntries.RemoveAtAndCopy(0);
  }
  else
  {
    inout_uiCacheIndex = s_CachedValues.GetCount();
    s_CachedValues.ExpandAndGetRef();
  }

  DeallocateCache(uiOldCacheIndex);

  return &s_CachedValues[inout_uiCacheIndex];
}

// static
void nsMaterialResource::DeallocateCache(nsUInt32 uiCacheIndex)
{
  if (uiCacheIndex != nsInvalidIndex)
  {
    NS_LOCK(s_MaterialCacheMutex);

    if (uiCacheIndex < s_CachedValues.GetCount())
    {
      s_CachedValues[uiCacheIndex].Reset();

      auto& freeEntry = s_FreeMaterialCacheEntries.ExpandAndGetRef();
      freeEntry.m_uiIndex = uiCacheIndex;
      freeEntry.m_uiFrame = nsRenderWorld::GetFrameCounter();
    }
  }
}

// static
void nsMaterialResource::ClearCache()
{
  NS_LOCK(s_MaterialCacheMutex);

  s_CachedValues.Clear();
  s_FreeMaterialCacheEntries.Clear();
}

const nsMaterialResourceDescriptor& nsMaterialResource::GetCurrentDesc() const
{
  return m_mDesc;
}

NS_STATICLINK_FILE(RendererCore, RendererCore_Material_Implementation_MaterialResource);
