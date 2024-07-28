#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Shader/ShaderResource.h>
#include <RendererCore/ShaderCompiler/ShaderParser.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsShaderResource, 1, nsRTTIDefaultAllocator<nsShaderResource>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_RESOURCE_IMPLEMENT_COMMON_CODE(nsShaderResource);
// clang-format on

nsShaderResource::nsShaderResource()
  : nsResource(DoUpdate::OnAnyThread, 1)
{
  m_bShaderResourceIsValid = false;
}

nsResourceLoadDesc nsShaderResource::UnloadData(Unload WhatToUnload)
{
  m_bShaderResourceIsValid = false;
  m_PermutationVarsUsed.Clear();

  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;
  res.m_State = nsResourceState::Unloaded;

  return res;
}

nsResourceLoadDesc nsShaderResource::UpdateContent(nsStreamReader* stream)
{
  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;

  m_bShaderResourceIsValid = false;

  if (stream == nullptr)
  {
    res.m_State = nsResourceState::LoadedResourceMissing;
    return res;
  }

  // skip the absolute file path data that the standard file reader writes into the stream
  {
    nsStringBuilder sAbsFilePath;
    (*stream) >> sAbsFilePath;
  }

  nsHybridArray<nsPermutationVar, 16> fixedPermVars; // ignored here
  nsShaderParser::ParsePermutationSection(*stream, m_PermutationVarsUsed, fixedPermVars);

  res.m_State = nsResourceState::Loaded;
  m_bShaderResourceIsValid = true;

  return res;
}

void nsShaderResource::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
{
  out_NewMemoryUsage.m_uiMemoryCPU = sizeof(nsShaderResource) + (nsUInt32)m_PermutationVarsUsed.GetHeapMemoryUsage();
  out_NewMemoryUsage.m_uiMemoryGPU = 0;
}

NS_RESOURCE_IMPLEMENT_CREATEABLE(nsShaderResource, nsShaderResourceDescriptor)
{
  nsResourceLoadDesc ret;
  ret.m_State = nsResourceState::Loaded;
  ret.m_uiQualityLevelsDiscardable = 0;
  ret.m_uiQualityLevelsLoadable = 0;

  m_bShaderResourceIsValid = false;

  return ret;
}

NS_STATICLINK_FILE(RendererCore, RendererCore_Shader_Implementation_ShaderResource);
