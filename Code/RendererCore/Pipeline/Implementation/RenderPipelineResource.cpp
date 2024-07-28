#include <RendererCore/RendererCorePCH.h>

#include <Foundation/Utilities/AssetFileHeader.h>
#include <RendererCore/Pipeline/Implementation/RenderPipelineResourceLoader.h>
#include <RendererCore/Pipeline/Passes/SimpleRenderPass.h>
#include <RendererCore/Pipeline/Passes/SourcePass.h>
#include <RendererCore/Pipeline/Passes/TargetPass.h>
#include <RendererCore/Pipeline/RenderPipeline.h>
#include <RendererCore/Pipeline/RenderPipelineResource.h>

void nsRenderPipelineResourceDescriptor::CreateFromRenderPipeline(const nsRenderPipeline* pPipeline)
{
  nsRenderPipelineResourceLoader::CreateRenderPipelineResourceDescriptor(pPipeline, *this);
}

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsRenderPipelineResource, 1, nsRTTIDefaultAllocator<nsRenderPipelineResource>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_RESOURCE_IMPLEMENT_COMMON_CODE(nsRenderPipelineResource);
// clang-format on

nsRenderPipelineResource::nsRenderPipelineResource()
  : nsResource(DoUpdate::OnAnyThread, 1)
{
}

nsInternal::NewInstance<nsRenderPipeline> nsRenderPipelineResource::CreateRenderPipeline() const
{
  if (GetLoadingState() != nsResourceState::Loaded)
  {
    nsLog::Error("Can't create render pipeline '{0}', the resource is not loaded!", GetResourceID());
    return nsInternal::NewInstance<nsRenderPipeline>(nullptr, nullptr);
  }

  return nsRenderPipelineResourceLoader::CreateRenderPipeline(m_Desc);
}

// static
nsRenderPipelineResourceHandle nsRenderPipelineResource::CreateMissingPipeline()
{
  nsUniquePtr<nsRenderPipeline> pRenderPipeline = NS_DEFAULT_NEW(nsRenderPipeline);

  nsSourcePass* pColorSourcePass = nullptr;
  {
    nsUniquePtr<nsSourcePass> pPass = NS_DEFAULT_NEW(nsSourcePass, "ColorSource");
    pColorSourcePass = pPass.Borrow();
    pRenderPipeline->AddPass(std::move(pPass));
  }

  nsSimpleRenderPass* pSimplePass = nullptr;
  {
    nsUniquePtr<nsSimpleRenderPass> pPass = NS_DEFAULT_NEW(nsSimpleRenderPass);
    pSimplePass = pPass.Borrow();
    pSimplePass->SetMessage("Render pipeline resource is missing. Ensure that the corresponding asset has been transformed.");
    pRenderPipeline->AddPass(std::move(pPass));
  }

  nsTargetPass* pTargetPass = nullptr;
  {
    nsUniquePtr<nsTargetPass> pPass = NS_DEFAULT_NEW(nsTargetPass);
    pTargetPass = pPass.Borrow();
    pRenderPipeline->AddPass(std::move(pPass));
  }

  NS_VERIFY(pRenderPipeline->Connect(pColorSourcePass, "Output", pSimplePass, "Color"), "Connect failed!");
  NS_VERIFY(pRenderPipeline->Connect(pSimplePass, "Color", pTargetPass, "Color0"), "Connect failed!");

  nsRenderPipelineResourceDescriptor desc;
  nsRenderPipelineResourceLoader::CreateRenderPipelineResourceDescriptor(pRenderPipeline.Borrow(), desc);

  return nsResourceManager::CreateResource<nsRenderPipelineResource>("MissingRenderPipeline", std::move(desc), "MissingRenderPipeline");
}

nsResourceLoadDesc nsRenderPipelineResource::UnloadData(Unload WhatToUnload)
{
  m_Desc.Clear();

  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;
  res.m_State = nsResourceState::Unloaded;

  return res;
}

nsResourceLoadDesc nsRenderPipelineResource::UpdateContent(nsStreamReader* Stream)
{
  m_Desc.Clear();

  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;
  res.m_State = nsResourceState::Loaded;

  if (Stream == nullptr)
  {
    res.m_State = nsResourceState::LoadedResourceMissing;
    return res;
  }

  nsStringBuilder sAbsFilePath;
  (*Stream) >> sAbsFilePath;

  if (sAbsFilePath.HasExtension("nsRenderPipelineBin"))
  {
    nsStringBuilder sTemp, sTemp2;

    nsAssetFileHeader AssetHash;
    AssetHash.Read(*Stream).IgnoreResult();

    nsUInt8 uiVersion = 0;
    (*Stream) >> uiVersion;

    // Version 1 was using old tooling serialization. Code path removed.
    if (uiVersion == 1)
    {
      res.m_State = nsResourceState::LoadedResourceMissing;
      nsLog::Error("Failed to load old nsRenderPipelineResource '{}'. Needs re-transform.", sAbsFilePath);
      return res;
    }
    NS_ASSERT_DEV(uiVersion == 2, "Unknown nsRenderPipelineBin version {0}", uiVersion);

    nsUInt32 uiSize = 0;
    (*Stream) >> uiSize;

    m_Desc.m_SerializedPipeline.SetCountUninitialized(uiSize);
    Stream->ReadBytes(m_Desc.m_SerializedPipeline.GetData(), uiSize);

    NS_ASSERT_DEV(uiSize > 0, "RenderPipeline resourse contains no pipeline data!");
  }
  else
  {
    NS_REPORT_FAILURE("The file '{0}' is unsupported, only '.nsRenderPipelineBin' files can be loaded as nsRenderPipelineResource", sAbsFilePath);
  }

  return res;
}

void nsRenderPipelineResource::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
{
  out_NewMemoryUsage.m_uiMemoryCPU = sizeof(nsRenderPipelineResource) + (nsUInt32)(m_Desc.m_SerializedPipeline.GetCount());

  out_NewMemoryUsage.m_uiMemoryGPU = 0;
}

NS_RESOURCE_IMPLEMENT_CREATEABLE(nsRenderPipelineResource, nsRenderPipelineResourceDescriptor)
{
  m_Desc = descriptor;

  nsResourceLoadDesc res;
  res.m_State = nsResourceState::Loaded;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;

  return res;
}



NS_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_RenderPipelineResource);
