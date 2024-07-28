#pragma once

#include <RendererCore/RendererCoreDLL.h>

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Memory/InstanceDataAllocator.h>
#include <Foundation/Types/UniquePtr.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphNode.h>

class NS_RENDERERCORE_DLL nsAnimGraph
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsAnimGraph);

public:
  nsAnimGraph();
  ~nsAnimGraph();

  void Clear();

  nsAnimGraphNode* AddNode(nsUniquePtr<nsAnimGraphNode>&& pNode);
  void AddConnection(const nsAnimGraphNode* pSrcNode, nsStringView sSrcPinName, nsAnimGraphNode* pDstNode, nsStringView sDstPinName);

  nsResult Serialize(nsStreamWriter& inout_stream) const;
  nsResult Deserialize(nsStreamReader& inout_stream);

  const nsInstanceDataAllocator& GetInstanceDataAlloator() const { return m_InstanceDataAllocator; }
  nsArrayPtr<const nsUniquePtr<nsAnimGraphNode>> GetNodes() const { return m_Nodes; }

  void PrepareForUse();

private:
  friend class nsAnimGraphInstance;

  struct ConnectionTo
  {
    nsString m_sSrcPinName;
    const nsAnimGraphNode* m_pDstNode = nullptr;
    nsString m_sDstPinName;
    nsAnimGraphPin* m_pSrcPin = nullptr;
    nsAnimGraphPin* m_pDstPin = nullptr;
  };

  struct ConnectionsTo
  {
    nsHybridArray<ConnectionTo, 2> m_To;
  };

  void SortNodesByPriority();
  void PreparePinMapping();
  void AssignInputPinIndices();
  void AssignOutputPinIndices();
  nsUInt16 ComputeNodePriority(const nsAnimGraphNode* pNode, nsMap<const nsAnimGraphNode*, nsUInt16>& inout_Prios, nsUInt16& inout_uiOutputPrio) const;

  bool m_bPreparedForUse = true;
  nsUInt32 m_uiInputPinCounts[nsAnimGraphPin::Type::ENUM_COUNT];
  nsUInt32 m_uiPinInstanceDataOffset[nsAnimGraphPin::Type::ENUM_COUNT];
  nsMap<const nsAnimGraphNode*, ConnectionsTo> m_From;

  nsDynamicArray<nsUniquePtr<nsAnimGraphNode>> m_Nodes;
  nsDynamicArray<nsHybridArray<nsUInt16, 1>> m_OutputPinToInputPinMapping[nsAnimGraphPin::ENUM_COUNT];
  nsInstanceDataAllocator m_InstanceDataAllocator;

  friend class nsAnimGraphTriggerOutputPin;
  friend class nsAnimGraphNumberOutputPin;
  friend class nsAnimGraphBoolOutputPin;
  friend class nsAnimGraphBoneWeightsOutputPin;
  friend class nsAnimGraphLocalPoseOutputPin;
  friend class nsAnimGraphModelPoseOutputPin;
};
