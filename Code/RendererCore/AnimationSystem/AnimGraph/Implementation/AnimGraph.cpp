#include <RendererCore/RendererCorePCH.h>

#include <Core/World/GameObject.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraph.h>

nsAnimGraph::nsAnimGraph()
{
  Clear();
}

nsAnimGraph::~nsAnimGraph() = default;

void nsAnimGraph::Clear()
{
  nsMemoryUtils::ZeroFillArray(m_uiInputPinCounts);
  nsMemoryUtils::ZeroFillArray(m_uiPinInstanceDataOffset);
  m_From.Clear();
  m_Nodes.Clear();
  m_bPreparedForUse = true;
  m_InstanceDataAllocator.ClearDescs();

  for (auto& r : m_OutputPinToInputPinMapping)
  {
    r.Clear();
  }
}

nsAnimGraphNode* nsAnimGraph::AddNode(nsUniquePtr<nsAnimGraphNode>&& pNode)
{
  m_bPreparedForUse = false;

  m_Nodes.PushBack(std::move(pNode));
  return m_Nodes.PeekBack().Borrow();
}

void nsAnimGraph::AddConnection(const nsAnimGraphNode* pSrcNode, nsStringView sSrcPinName, nsAnimGraphNode* pDstNode, nsStringView sDstPinName)
{
  // TODO: assert pSrcNode and pDstNode exist

  m_bPreparedForUse = false;
  nsStringView sIdx;

  nsAbstractMemberProperty* pPinPropSrc = (nsAbstractMemberProperty*)pSrcNode->GetDynamicRTTI()->FindPropertyByName(sSrcPinName);

  auto& to = m_From[pSrcNode].m_To.ExpandAndGetRef();
  to.m_sSrcPinName = sSrcPinName;
  to.m_pDstNode = pDstNode;
  to.m_sDstPinName = sDstPinName;
  to.m_pSrcPin = (nsAnimGraphPin*)pPinPropSrc->GetPropertyPointer(pSrcNode);

  if (const char* szIdx = sDstPinName.FindSubString("["))
  {
    sIdx = nsStringView(szIdx + 1, sDstPinName.GetEndPointer() - 1);
    sDstPinName = nsStringView(sDstPinName.GetStartPointer(), szIdx);

    nsAbstractArrayProperty* pPinPropDst = (nsAbstractArrayProperty*)pDstNode->GetDynamicRTTI()->FindPropertyByName(sDstPinName);
    const nsDynamicPinAttribute* pDynPinAttr = pPinPropDst->GetAttributeByType<nsDynamicPinAttribute>();

    const nsTypedMemberProperty<nsUInt8>* pPinSizeProp = (const nsTypedMemberProperty<nsUInt8>*)pDstNode->GetDynamicRTTI()->FindPropertyByName(pDynPinAttr->GetProperty());
    nsUInt8 uiArraySize = pPinSizeProp->GetValue(pDstNode);
    pPinPropDst->SetCount(pDstNode, uiArraySize);

    nsUInt32 uiIdx;
    nsConversionUtils::StringToUInt(sIdx, uiIdx).AssertSuccess();

    to.m_pDstPin = (nsAnimGraphPin*)pPinPropDst->GetValuePointer(pDstNode, uiIdx);
  }
  else
  {
    nsAbstractMemberProperty* pPinPropDst = (nsAbstractMemberProperty*)pDstNode->GetDynamicRTTI()->FindPropertyByName(sDstPinName);

    to.m_pDstPin = (nsAnimGraphPin*)pPinPropDst->GetPropertyPointer(pDstNode);
  }
}

void nsAnimGraph::PreparePinMapping()
{
  nsUInt16 uiOutputPinCounts[nsAnimGraphPin::Type::ENUM_COUNT];
  nsMemoryUtils::ZeroFillArray(uiOutputPinCounts);

  for (const auto& consFrom : m_From)
  {
    for (const ConnectionTo& to : consFrom.Value().m_To)
    {
      uiOutputPinCounts[to.m_pSrcPin->GetPinType()]++;
    }
  }

  for (nsUInt32 i = 0; i < nsAnimGraphPin::ENUM_COUNT; ++i)
  {
    m_OutputPinToInputPinMapping[i].Clear();
    m_OutputPinToInputPinMapping[i].SetCount(uiOutputPinCounts[i]);
  }
}

void nsAnimGraph::AssignInputPinIndices()
{
  nsMemoryUtils::ZeroFillArray(m_uiInputPinCounts);

  for (auto& consFrom : m_From)
  {
    for (ConnectionTo& to : consFrom.Value().m_To)
    {
      // there may be multiple connections to this pin
      // only assign the index the first time we see a connection to this pin
      // otherwise only count up the number of connections

      if (to.m_pDstPin->m_iPinIndex == -1)
      {
        to.m_pDstPin->m_iPinIndex = m_uiInputPinCounts[to.m_pDstPin->GetPinType()]++;
      }

      ++to.m_pDstPin->m_uiNumConnections;
    }
  }
}

void nsAnimGraph::AssignOutputPinIndices()
{
  nsInt16 iPinTypeCount[nsAnimGraphPin::Type::ENUM_COUNT];
  nsMemoryUtils::ZeroFillArray(iPinTypeCount);

  for (auto& consFrom : m_From)
  {
    for (ConnectionTo& to : consFrom.Value().m_To)
    {
      const nsUInt8 pinType = to.m_pSrcPin->GetPinType();

      // there may be multiple connections from this pin
      // only assign the index the first time we see a connection from this pin

      if (to.m_pSrcPin->m_iPinIndex == -1)
      {
        to.m_pSrcPin->m_iPinIndex = iPinTypeCount[pinType]++;
      }

      // store the indices of all the destination pins
      m_OutputPinToInputPinMapping[pinType][to.m_pSrcPin->m_iPinIndex].PushBack(to.m_pDstPin->m_iPinIndex);
    }
  }
}

nsUInt16 nsAnimGraph::ComputeNodePriority(const nsAnimGraphNode* pNode, nsMap<const nsAnimGraphNode*, nsUInt16>& inout_Prios, nsUInt16& inout_uiOutputPrio) const
{
  auto itPrio = inout_Prios.Find(pNode);
  if (itPrio.IsValid())
  {
    // priority already computed -> return it
    return itPrio.Value();
  }

  const auto itConsFrom = m_From.Find(pNode);

  nsUInt16 uiOwnPrio = 0xFFFF;

  if (itConsFrom.IsValid())
  {
    // look at all outgoing priorities and take the smallest dst priority - 1
    for (const ConnectionTo& to : itConsFrom.Value().m_To)
    {
      uiOwnPrio = nsMath::Min<nsUInt16>(uiOwnPrio, ComputeNodePriority(to.m_pDstNode, inout_Prios, inout_uiOutputPrio) - 1);
    }
  }
  else
  {
    // has no outgoing connections at all -> max priority value
    uiOwnPrio = inout_uiOutputPrio;
    inout_uiOutputPrio -= 64;
  }

  NS_ASSERT_DEBUG(uiOwnPrio != 0xFFFF, "");

  inout_Prios[pNode] = uiOwnPrio;
  return uiOwnPrio;
}

void nsAnimGraph::SortNodesByPriority()
{
  // this is important so that we can step all nodes in linear order,
  // and have them generate their output such that it is ready before
  // dependent nodes are stepped

  nsUInt16 uiOutputPrio = 0xFFFE;
  nsMap<const nsAnimGraphNode*, nsUInt16> prios;
  for (const auto& pNode : m_Nodes)
  {
    ComputeNodePriority(pNode.Borrow(), prios, uiOutputPrio);
  }

  m_Nodes.Sort([&](const auto& lhs, const auto& rhs) -> bool
    { return prios[lhs.Borrow()] < prios[rhs.Borrow()]; });
}

void nsAnimGraph::PrepareForUse()
{
  if (m_bPreparedForUse)
    return;

  m_bPreparedForUse = true;

  for (auto& consFrom : m_From)
  {
    for (ConnectionTo& to : consFrom.Value().m_To)
    {
      to.m_pSrcPin->m_iPinIndex = -1;
      to.m_pSrcPin->m_uiNumConnections = 0;
      to.m_pDstPin->m_iPinIndex = -1;
      to.m_pDstPin->m_uiNumConnections = 0;
    }
  }

  SortNodesByPriority();
  PreparePinMapping();
  AssignInputPinIndices();
  AssignOutputPinIndices();

  m_InstanceDataAllocator.ClearDescs();
  for (const auto& pNode : m_Nodes)
  {
    nsInstanceDataDesc desc;
    if (pNode->GetInstanceDataDesc(desc))
    {
      pNode->m_uiInstanceDataOffset = m_InstanceDataAllocator.AddDesc(desc);
    }
  }

  // EXTEND THIS if a new type is introduced
  {
    nsInstanceDataDesc desc;
    desc.m_uiTypeAlignment = NS_ALIGNMENT_OF(nsInt8);
    desc.m_uiTypeSize = sizeof(nsInt8) * m_uiInputPinCounts[nsAnimGraphPin::Type::Trigger];
    m_uiPinInstanceDataOffset[nsAnimGraphPin::Type::Trigger] = m_InstanceDataAllocator.AddDesc(desc);
  }
  {
    nsInstanceDataDesc desc;
    desc.m_uiTypeAlignment = NS_ALIGNMENT_OF(double);
    desc.m_uiTypeSize = sizeof(double) * m_uiInputPinCounts[nsAnimGraphPin::Type::Number];
    m_uiPinInstanceDataOffset[nsAnimGraphPin::Type::Number] = m_InstanceDataAllocator.AddDesc(desc);
  }
  {
    nsInstanceDataDesc desc;
    desc.m_uiTypeAlignment = NS_ALIGNMENT_OF(bool);
    desc.m_uiTypeSize = sizeof(bool) * m_uiInputPinCounts[nsAnimGraphPin::Type::Bool];
    m_uiPinInstanceDataOffset[nsAnimGraphPin::Type::Bool] = m_InstanceDataAllocator.AddDesc(desc);
  }
  {
    nsInstanceDataDesc desc;
    desc.m_uiTypeAlignment = NS_ALIGNMENT_OF(nsUInt16);
    desc.m_uiTypeSize = sizeof(nsUInt16) * m_uiInputPinCounts[nsAnimGraphPin::Type::BoneWeights];
    m_uiPinInstanceDataOffset[nsAnimGraphPin::Type::BoneWeights] = m_InstanceDataAllocator.AddDesc(desc);
  }
  {
    nsInstanceDataDesc desc;
    desc.m_uiTypeAlignment = NS_ALIGNMENT_OF(nsUInt16);
    desc.m_uiTypeSize = sizeof(nsUInt16) * m_uiInputPinCounts[nsAnimGraphPin::Type::ModelPose];
    m_uiPinInstanceDataOffset[nsAnimGraphPin::Type::ModelPose] = m_InstanceDataAllocator.AddDesc(desc);
  }
}

nsResult nsAnimGraph::Serialize(nsStreamWriter& inout_stream) const
{
  inout_stream.WriteVersion(10);

  const nsUInt32 uiNumNodes = m_Nodes.GetCount();
  inout_stream << uiNumNodes;

  nsMap<const nsAnimGraphNode*, nsUInt32> nodeToIdx;

  for (nsUInt32 n = 0; n < m_Nodes.GetCount(); ++n)
  {
    const nsAnimGraphNode* pNode = m_Nodes[n].Borrow();

    nodeToIdx[pNode] = n;

    inout_stream << pNode->GetDynamicRTTI()->GetTypeName();
    NS_SUCCEED_OR_RETURN(pNode->SerializeNode(inout_stream));
  }

  inout_stream << m_From.GetCount();
  for (auto itFrom : m_From)
  {
    inout_stream << nodeToIdx[itFrom.Key()];

    const auto& toAll = itFrom.Value().m_To;
    inout_stream << toAll.GetCount();

    for (const auto& to : toAll)
    {
      inout_stream << to.m_sSrcPinName;
      inout_stream << nodeToIdx[to.m_pDstNode];
      inout_stream << to.m_sDstPinName;
    }
  }

  return NS_SUCCESS;
}

nsResult nsAnimGraph::Deserialize(nsStreamReader& inout_stream)
{
  Clear();

  const nsTypeVersion version = inout_stream.ReadVersion(10);

  if (version < 10)
    return NS_FAILURE;

  nsUInt32 uiNumNodes = 0;
  inout_stream >> uiNumNodes;

  nsDynamicArray<nsAnimGraphNode*> idxToNode;
  idxToNode.SetCount(uiNumNodes);

  nsStringBuilder sTypeName;

  for (nsUInt32 n = 0; n < uiNumNodes; ++n)
  {
    inout_stream >> sTypeName;
    nsUniquePtr<nsAnimGraphNode> pNode = nsRTTI::FindTypeByName(sTypeName)->GetAllocator()->Allocate<nsAnimGraphNode>();
    NS_SUCCEED_OR_RETURN(pNode->DeserializeNode(inout_stream));

    idxToNode[n] = AddNode(std::move(pNode));
  }

  nsUInt32 uiNumConnectionsFrom = 0;
  inout_stream >> uiNumConnectionsFrom;

  nsStringBuilder sPinSrc, sPinDst;

  for (nsUInt32 cf = 0; cf < uiNumConnectionsFrom; ++cf)
  {
    nsUInt32 nodeIdx;
    inout_stream >> nodeIdx;
    const nsAnimGraphNode* ptrNodeFrom = idxToNode[nodeIdx];

    nsUInt32 uiNumConnectionsTo = 0;
    inout_stream >> uiNumConnectionsTo;

    for (nsUInt32 ct = 0; ct < uiNumConnectionsTo; ++ct)
    {
      inout_stream >> sPinSrc;

      inout_stream >> nodeIdx;
      nsAnimGraphNode* ptrNodeTo = idxToNode[nodeIdx];

      inout_stream >> sPinDst;

      AddConnection(ptrNodeFrom, sPinSrc, ptrNodeTo, sPinDst);
    }
  }

  m_bPreparedForUse = false;
  return NS_SUCCESS;
}
