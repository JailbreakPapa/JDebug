#include <RendererCore/RendererCorePCH.h>

#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/OSFile.h>
#include <RendererCore/Shader/ShaderPermutationResource.h>
#include <RendererCore/ShaderCompiler/ShaderCompiler.h>
#include <RendererCore/ShaderCompiler/ShaderManager.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererFoundation/Shader/Shader.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsShaderPermutationResource, 1, nsRTTIDefaultAllocator<nsShaderPermutationResource>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_RESOURCE_IMPLEMENT_COMMON_CODE(nsShaderPermutationResource);
// clang-format on

static nsShaderPermutationResourceLoader g_PermutationResourceLoader;

nsShaderPermutationResource::nsShaderPermutationResource()
  : nsResource(DoUpdate::OnAnyThread, 1)
{
  m_bShaderPermutationValid = false;

  for (nsUInt32 stage = 0; stage < nsGALShaderStage::ENUM_COUNT; ++stage)
  {
    m_ByteCodes[stage] = nullptr;
  }
}

nsResourceLoadDesc nsShaderPermutationResource::UnloadData(Unload WhatToUnload)
{
  m_bShaderPermutationValid = false;

  auto pDevice = nsGALDevice::GetDefaultDevice();

  if (!m_hShader.IsInvalidated())
  {
    pDevice->DestroyShader(m_hShader);
    m_hShader.Invalidate();
  }

  if (!m_hBlendState.IsInvalidated())
  {
    pDevice->DestroyBlendState(m_hBlendState);
    m_hBlendState.Invalidate();
  }

  if (!m_hDepthStencilState.IsInvalidated())
  {
    pDevice->DestroyDepthStencilState(m_hDepthStencilState);
    m_hDepthStencilState.Invalidate();
  }

  if (!m_hRasterizerState.IsInvalidated())
  {
    pDevice->DestroyRasterizerState(m_hRasterizerState);
    m_hRasterizerState.Invalidate();
  }


  nsResourceLoadDesc res;
  res.m_State = nsResourceState::Unloaded;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;

  return res;
}

nsResourceLoadDesc nsShaderPermutationResource::UpdateContent(nsStreamReader* Stream)
{
  nsUInt32 uiGPUMem = 0;
  ModifyMemoryUsage().m_uiMemoryGPU = 0;

  m_bShaderPermutationValid = false;

  nsResourceLoadDesc res;
  res.m_State = nsResourceState::Loaded;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;

  if (Stream == nullptr)
  {
    nsLog::Error("Shader Permutation '{0}': Data is not available", GetResourceID());
    return res;
  }

  nsShaderPermutationBinary PermutationBinary;

  bool bOldVersion = false;
  if (PermutationBinary.Read(*Stream, bOldVersion).Failed())
  {
    nsLog::Error("Shader Permutation '{0}': Could not read shader permutation binary", GetResourceID());
    return res;
  }

  auto pDevice = nsGALDevice::GetDefaultDevice();

  // get the shader render state object
  {
    m_hBlendState = pDevice->CreateBlendState(PermutationBinary.m_StateDescriptor.m_BlendDesc);
    m_hDepthStencilState = pDevice->CreateDepthStencilState(PermutationBinary.m_StateDescriptor.m_DepthStencilDesc);
    m_hRasterizerState = pDevice->CreateRasterizerState(PermutationBinary.m_StateDescriptor.m_RasterizerDesc);
  }

  nsGALShaderCreationDescription ShaderDesc;

  // iterate over all shader stages, add them to the descriptor
  for (nsUInt32 stage = nsGALShaderStage::VertexShader; stage < nsGALShaderStage::ENUM_COUNT; ++stage)
  {
    const nsUInt32 uiStageHash = PermutationBinary.m_uiShaderStageHashes[stage];

    if (uiStageHash == 0) // not used
      continue;

    nsShaderStageBinary* pStageBin = nsShaderStageBinary::LoadStageBinary((nsGALShaderStage::Enum)stage, uiStageHash, nsShaderManager::GetActivePlatform());

    if (pStageBin == nullptr)
    {
      nsLog::Error("Shader Permutation '{0}': Stage '{1}' could not be loaded", GetResourceID(), nsGALShaderStage::Names[stage]);
      return res;
    }

    // store not only the hash but also the pointer to the stage binary
    // since it contains other useful information (resource bindings), that we need for shader binding
    m_ByteCodes[stage] = pStageBin->GetByteCode();

    NS_ASSERT_DEV(pStageBin->m_pGALByteCode->m_Stage == stage, "Invalid shader stage! Expected stage '{0}', but loaded data is for stage '{1}'", nsGALShaderStage::Names[stage], nsGALShaderStage::Names[pStageBin->m_pGALByteCode->m_Stage]);

    ShaderDesc.m_ByteCodes[stage] = pStageBin->m_pGALByteCode;

    uiGPUMem += pStageBin->m_pGALByteCode->m_ByteCode.GetCount();
  }

  m_hShader = pDevice->CreateShader(ShaderDesc);

  if (m_hShader.IsInvalidated())
  {
    nsLog::Error("Shader Permutation '{0}': Shader program creation failed", GetResourceID());
    return res;
  }

  pDevice->GetShader(m_hShader)->SetDebugName(GetResourceID());

  m_PermutationVars = PermutationBinary.m_PermutationVars;

  m_bShaderPermutationValid = true;

  ModifyMemoryUsage().m_uiMemoryGPU = uiGPUMem;

  return res;
}

void nsShaderPermutationResource::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
{
  out_NewMemoryUsage.m_uiMemoryCPU = sizeof(nsShaderPermutationResource);
  out_NewMemoryUsage.m_uiMemoryGPU = ModifyMemoryUsage().m_uiMemoryGPU;
}

NS_RESOURCE_IMPLEMENT_CREATEABLE(nsShaderPermutationResource, nsShaderPermutationResourceDescriptor)
{
  nsResourceLoadDesc ret;
  ret.m_State = nsResourceState::Loaded;
  ret.m_uiQualityLevelsDiscardable = 0;
  ret.m_uiQualityLevelsLoadable = 0;

  return ret;
}

nsResourceTypeLoader* nsShaderPermutationResource::GetDefaultResourceTypeLoader() const
{
  return &g_PermutationResourceLoader;
}

struct ShaderPermutationResourceLoadData
{
  ShaderPermutationResourceLoadData()
    : m_Reader(&m_Storage)
  {
  }

  nsContiguousMemoryStreamStorage m_Storage;
  nsMemoryStreamReader m_Reader;
};

nsResult nsShaderPermutationResourceLoader::RunCompiler(const nsResource* pResource, nsShaderPermutationBinary& BinaryInfo, bool bForce)
{
  if (nsShaderManager::IsRuntimeCompilationEnabled())
  {
    if (!bForce)
    {
      // check whether any dependent file has changed, and trigger a recompilation if necessary
      if (BinaryInfo.m_DependencyFile.HasAnyFileChanged())
      {
        bForce = true;
      }
    }

    if (!bForce) // no recompilation necessary
      return NS_SUCCESS;

    nsStringBuilder sPermutationFile = pResource->GetResourceID();

    sPermutationFile.ChangeFileExtension("");
    sPermutationFile.Shrink(nsShaderManager::GetCacheDirectory().GetCharacterCount() + nsShaderManager::GetActivePlatform().GetCharacterCount() + 2, 1);

    sPermutationFile.Shrink(0, 9); // remove underscore and the hash at the end
    sPermutationFile.Append(".nsShader");

    nsArrayPtr<const nsPermutationVar> permutationVars = static_cast<const nsShaderPermutationResource*>(pResource)->GetPermutationVars();

    nsShaderCompiler sc;
    return sc.CompileShaderPermutationForPlatforms(sPermutationFile, permutationVars, nsLog::GetThreadLocalLogSystem(), nsShaderManager::GetActivePlatform());
  }
  else
  {
    if (bForce)
    {
      nsLog::Error("Shader was forced to be compiled, but runtime shader compilation is not available");
      return NS_FAILURE;
    }
  }

  return NS_SUCCESS;
}

bool nsShaderPermutationResourceLoader::IsResourceOutdated(const nsResource* pResource) const
{
  // don't try to reload a file that cannot be found
  nsStringBuilder sAbs;
  if (nsFileSystem::ResolvePath(pResource->GetResourceID(), &sAbs, nullptr).Failed())
    return false;

#if NS_ENABLED(NS_SUPPORTS_FILE_STATS)
  if (pResource->GetLoadedFileModificationTime().IsValid())
  {
    nsFileStats stat;
    if (nsFileSystem::GetFileStats(pResource->GetResourceID(), stat).Failed())
      return false;

    if (!stat.m_LastModificationTime.Compare(pResource->GetLoadedFileModificationTime(), nsTimestamp::CompareMode::FileTimeEqual))
      return true;
  }

#endif

  nsDependencyFile dep;
  if (dep.ReadDependencyFile(pResource->GetResourceID()).Failed())
    return true;

  return dep.HasAnyFileChanged();
}

nsResourceLoadData nsShaderPermutationResourceLoader::OpenDataStream(const nsResource* pResource)
{
  nsResourceLoadData res;

  nsShaderPermutationBinary permutationBinary;

  bool bNeedsCompilation = true;
  bool bOldVersion = false;

  {
    nsFileReader File;
    if (File.Open(pResource->GetResourceID().GetData()).Failed())
    {
      nsLog::Debug("Shader Permutation '{0}' does not exist, triggering recompile.", pResource->GetResourceID());

      bNeedsCompilation = false;
      if (RunCompiler(pResource, permutationBinary, true).Failed())
        return res;

      // try again
      if (File.Open(pResource->GetResourceID().GetData()).Failed())
      {
        nsLog::Debug("Shader Permutation '{0}' still does not exist after recompile.", pResource->GetResourceID());
        return res;
      }
    }

    res.m_sResourceDescription = File.GetFilePathRelative().GetData();

#if NS_ENABLED(NS_SUPPORTS_FILE_STATS)
    nsFileStats stat;
    if (nsFileSystem::GetFileStats(pResource->GetResourceID(), stat).Succeeded())
    {
      res.m_LoadedFileModificationDate = stat.m_LastModificationTime;
    }
#endif

    if (permutationBinary.Read(File, bOldVersion).Failed())
    {
      nsLog::Error("Shader Permutation '{0}': Could not read shader permutation binary", pResource->GetResourceID());

      bNeedsCompilation = true;
    }

    if (bOldVersion)
    {
      nsLog::Dev("Shader Permutation Binary version is outdated, recompiling shader.");
      bNeedsCompilation = true;
    }
  }

  if (bNeedsCompilation)
  {
    if (RunCompiler(pResource, permutationBinary, false).Failed())
      return res;

    nsFileReader File;

    if (File.Open(pResource->GetResourceID().GetData()).Failed())
    {
      nsLog::Error("Shader Permutation '{0}': Failed to open the file", pResource->GetResourceID());
      return res;
    }

    if (permutationBinary.Read(File, bOldVersion).Failed())
    {
      nsLog::Error("Shader Permutation '{0}': Binary data could not be read", pResource->GetResourceID());
      return res;
    }

    File.Close();
  }



  ShaderPermutationResourceLoadData* pData = NS_DEFAULT_NEW(ShaderPermutationResourceLoadData);

  nsMemoryStreamWriter w(&pData->m_Storage);

  // preload the files that are referenced in the .nsPermutation file
  {
    // write the permutation file info back to the output stream, so that the resource can read it as well
    permutationBinary.Write(w).IgnoreResult();

    for (nsUInt32 stage = nsGALShaderStage::VertexShader; stage < nsGALShaderStage::ENUM_COUNT; ++stage)
    {
      const nsUInt32 uiStageHash = permutationBinary.m_uiShaderStageHashes[stage];

      if (uiStageHash == 0) // not used
        continue;

      // this is where the preloading happens
      nsShaderStageBinary::LoadStageBinary((nsGALShaderStage::Enum)stage, uiStageHash, nsShaderManager::GetActivePlatform());
    }
  }

  res.m_pDataStream = &pData->m_Reader;
  res.m_pCustomLoaderData = pData;

  return res;
}

void nsShaderPermutationResourceLoader::CloseDataStream(const nsResource* pResource, const nsResourceLoadData& loaderData)
{
  ShaderPermutationResourceLoadData* pData = static_cast<ShaderPermutationResourceLoadData*>(loaderData.m_pCustomLoaderData);

  NS_DEFAULT_DELETE(pData);
}



NS_STATICLINK_FILE(RendererCore, RendererCore_Shader_Implementation_ShaderPermutationResource);
