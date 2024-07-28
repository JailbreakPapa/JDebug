#include <RendererCore/RendererCorePCH.h>

#include <Foundation/IO/MemoryStream.h>
#include <Foundation/IO/Stream.h>
#include <Foundation/Utilities/AssetFileHeader.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraph.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphNode.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphResource.h>
#include <RendererCore/AnimationSystem/AnimationClipResource.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsAnimationClipMapping, 1, nsRTTIDefaultAllocator<nsAnimationClipMapping>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("ClipName", GetClipName, SetClipName)->AddAttributes(new nsDynamicStringEnumAttribute("AnimationClipMappingEnum")),
    NS_ACCESSOR_PROPERTY("Clip", GetClip, SetClip)->AddAttributes(new nsAssetBrowserAttribute("CompatibleAsset_Keyframe_Animation")),
  }
    NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsAnimGraphResource, 1, nsRTTIDefaultAllocator<nsAnimGraphResource>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_RESOURCE_IMPLEMENT_COMMON_CODE(nsAnimGraphResource);
// clang-format on

const char* nsAnimationClipMapping::GetClip() const
{
  if (m_hClip.IsValid())
    return m_hClip.GetResourceID();

  return "";
}

void nsAnimationClipMapping::SetClip(const char* szName)
{
  nsAnimationClipResourceHandle hResource;

  if (!nsStringUtils::IsNullOrEmpty(szName))
  {
    hResource = nsResourceManager::LoadResource<nsAnimationClipResource>(szName);
  }

  m_hClip = hResource;
}

nsAnimGraphResource::nsAnimGraphResource()
  : nsResource(nsResource::DoUpdate::OnAnyThread, 0)
{
}

nsAnimGraphResource::~nsAnimGraphResource() = default;

nsResourceLoadDesc nsAnimGraphResource::UnloadData(Unload WhatToUnload)
{
  nsResourceLoadDesc d;
  d.m_State = nsResourceState::Unloaded;
  d.m_uiQualityLevelsDiscardable = 0;
  d.m_uiQualityLevelsLoadable = 0;
  return d;
}

nsResourceLoadDesc nsAnimGraphResource::UpdateContent(nsStreamReader* Stream)
{
  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;

  if (Stream == nullptr)
  {
    res.m_State = nsResourceState::LoadedResourceMissing;
    return res;
  }

  // skip the absolute file path data that the standard file reader writes into the stream
  {
    nsStringBuilder sAbsFilePath;
    (*Stream) >> sAbsFilePath;
  }

  nsAssetFileHeader AssetHash;
  AssetHash.Read(*Stream).AssertSuccess();

  {
    const auto uiVersion = Stream->ReadVersion(2);
    Stream->ReadArray(m_IncludeGraphs).AssertSuccess();

    if (uiVersion >= 2)
    {
      nsUInt32 uiNum = 0;
      *Stream >> uiNum;

      m_AnimationClipMapping.SetCount(uiNum);
      for (nsUInt32 i = 0; i < uiNum; ++i)
      {
        *Stream >> m_AnimationClipMapping[i].m_sClipName;
        *Stream >> m_AnimationClipMapping[i].m_hClip;
      }
    }
  }

  if (m_AnimGraph.Deserialize(*Stream).Failed())
  {
    res.m_State = nsResourceState::LoadedResourceMissing;
    return res;
  }

  m_AnimGraph.PrepareForUse();

  res.m_State = nsResourceState::Loaded;

  return res;
}

void nsAnimGraphResource::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
{
  out_NewMemoryUsage.m_uiMemoryGPU = 0;
  out_NewMemoryUsage.m_uiMemoryCPU = 0;
}

NS_STATICLINK_FILE(RendererCore, RendererCore_AnimationSystem_AnimGraph_Implementation_AnimGraphResource);
