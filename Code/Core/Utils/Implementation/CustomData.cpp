#include <Core/CorePCH.h>

#include <Core/Utils/CustomData.h>
#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Serialization/BinarySerializer.h>
#include <Foundation/Serialization/ReflectionSerializer.h>
#include <Foundation/Serialization/RttiConverter.h>
#include <Foundation/Utilities/AssetFileHeader.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsCustomData, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

void nsCustomData::Load(nsAbstractObjectGraph& ref_graph, nsRttiConverterContext& ref_context, const nsAbstractObjectNode* pRootNode)
{
  nsRttiConverterReader convRead(&ref_graph, &ref_context);
  convRead.ApplyPropertiesToObject(pRootNode, GetDynamicRTTI(), this);
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsCustomDataResourceBase, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsCustomDataResourceBase::nsCustomDataResourceBase()
  : nsResource(DoUpdate::OnAnyThread, 1)
{
}

nsCustomDataResourceBase::~nsCustomDataResourceBase() = default;

nsResourceLoadDesc nsCustomDataResourceBase::UnloadData(Unload WhatToUnload)
{
  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;
  res.m_State = nsResourceState::Unloaded;
  return res;
}

nsResourceLoadDesc nsCustomDataResourceBase::UpdateContent_Internal(nsStreamReader* Stream, const nsRTTI& rtti)
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
  AssetHash.Read(*Stream).IgnoreResult();

  nsAbstractObjectGraph graph;
  nsRttiConverterContext context;

  nsAbstractGraphBinarySerializer::Read(*Stream, &graph);

  const nsAbstractObjectNode* pRootNode = graph.GetNodeByName("root");

  if (pRootNode != nullptr && pRootNode->GetType() != rtti.GetTypeName())
  {
    nsLog::Error("Expected nsCustomData type '{}' but resource is of type '{}' ('{}')", rtti.GetTypeName(), pRootNode->GetType(), GetResourceIdOrDescription());

    // make sure we create a default-initialized object and don't deserialize data that happens to match
    pRootNode = nullptr;
  }

  CreateAndLoadData(graph, context, pRootNode);

  res.m_State = nsResourceState::Loaded;
  return res;
}

NS_STATICLINK_FILE(Core, Core_Utils_Implementation_CustomData);
