#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/Log.h>
#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Serialization/ApplyNativePropertyChangesContext.h>
#include <Foundation/Serialization/RttiConverter.h>

// clang-format off
WD_BEGIN_STATIC_REFLECTED_ENUM(wdObjectChangeType, 1)
  WD_ENUM_CONSTANTS(wdObjectChangeType::NodeAdded, wdObjectChangeType::NodeRemoved)
  WD_ENUM_CONSTANTS(wdObjectChangeType::PropertySet, wdObjectChangeType::PropertyInserted, wdObjectChangeType::PropertyRemoved)
WD_END_STATIC_REFLECTED_ENUM;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdAbstractObjectNode, wdNoBase, 1, wdRTTIDefaultAllocator<wdAbstractObjectNode>)
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdDiffOperation, wdNoBase, 1, wdRTTIDefaultAllocator<wdDiffOperation>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_ENUM_MEMBER_PROPERTY("Operation", wdObjectChangeType, m_Operation),
    WD_MEMBER_PROPERTY("Node", m_Node),
    WD_MEMBER_PROPERTY("Property", m_sProperty),
    WD_MEMBER_PROPERTY("Index", m_Index),
    WD_MEMBER_PROPERTY("Value", m_Value),
  }
    WD_END_PROPERTIES;
}
WD_END_STATIC_REFLECTED_TYPE;
// clang-format on

wdAbstractObjectGraph::~wdAbstractObjectGraph()
{
  Clear();
}

void wdAbstractObjectGraph::Clear()
{
  for (auto it = m_Nodes.GetIterator(); it.IsValid(); ++it)
  {
    WD_DEFAULT_DELETE(it.Value());
  }
  m_Nodes.Clear();
  m_NodesByName.Clear();
  m_Strings.Clear();
}


wdAbstractObjectNode* wdAbstractObjectGraph::Clone(wdAbstractObjectGraph& ref_cloneTarget, const wdAbstractObjectNode* pRootNode, FilterFunction filter) const
{
  ref_cloneTarget.Clear();

  if (pRootNode == nullptr)
  {
    for (auto it = m_Nodes.GetIterator(); it.IsValid(); ++it)
    {
      if (filter.IsValid())
      {
        ref_cloneTarget.CopyNodeIntoGraph(it.Value(), filter);
      }
      else
      {
        ref_cloneTarget.CopyNodeIntoGraph(it.Value());
      }
    }
    return nullptr;
  }
  else
  {
    WD_ASSERT_DEV(pRootNode->GetOwner() == this, "The given root node must be part of this document");
    wdSet<wdUuid> reachableNodes;
    FindTransitiveHull(pRootNode->GetGuid(), reachableNodes);

    for (const wdUuid& guid : reachableNodes)
    {
      if (auto* pNode = GetNode(guid))
      {
        if (filter.IsValid())
        {
          ref_cloneTarget.CopyNodeIntoGraph(pNode, filter);
        }
        else
        {
          ref_cloneTarget.CopyNodeIntoGraph(pNode);
        }
      }
    }

    return ref_cloneTarget.GetNode(pRootNode->GetGuid());
  }
}

const char* wdAbstractObjectGraph::RegisterString(const char* szString)
{
  auto it = m_Strings.Insert(szString);
  WD_ASSERT_DEV(it.IsValid(), "");
  return it.Key().GetData();
}

wdAbstractObjectNode* wdAbstractObjectGraph::GetNode(const wdUuid& guid)
{
  return m_Nodes.GetValueOrDefault(guid, nullptr);
}

const wdAbstractObjectNode* wdAbstractObjectGraph::GetNode(const wdUuid& guid) const
{
  return const_cast<wdAbstractObjectGraph*>(this)->GetNode(guid);
}

const wdAbstractObjectNode* wdAbstractObjectGraph::GetNodeByName(const char* szName) const
{
  return const_cast<wdAbstractObjectGraph*>(this)->GetNodeByName(szName);
}

wdAbstractObjectNode* wdAbstractObjectGraph::GetNodeByName(const char* szName)
{
  return m_NodesByName.GetValueOrDefault(szName, nullptr);
}

wdAbstractObjectNode* wdAbstractObjectGraph::AddNode(const wdUuid& guid, const char* szType, wdUInt32 uiTypeVersion, const char* szNodeName)
{
  WD_ASSERT_DEV(!m_Nodes.Contains(guid), "object {0} must not yet exist", guid);
  if (!wdStringUtils::IsNullOrEmpty(szNodeName))
  {
    szNodeName = RegisterString(szNodeName);
  }
  else
  {
    szNodeName = nullptr;
  }

  wdAbstractObjectNode* pNode = WD_DEFAULT_NEW(wdAbstractObjectNode);
  pNode->m_Guid = guid;
  pNode->m_pOwner = this;
  pNode->m_szType = RegisterString(szType);
  pNode->m_uiTypeVersion = uiTypeVersion;
  pNode->m_szNodeName = szNodeName;

  m_Nodes[guid] = pNode;

  if (!wdStringUtils::IsNullOrEmpty(szNodeName))
  {
    m_NodesByName[szNodeName] = pNode;
  }

  return pNode;
}

void wdAbstractObjectGraph::RemoveNode(const wdUuid& guid)
{
  auto it = m_Nodes.Find(guid);

  if (it.IsValid())
  {
    wdAbstractObjectNode* pNode = it.Value();
    if (pNode->m_szNodeName != nullptr)
      m_NodesByName.Remove(pNode->m_szNodeName);

    m_Nodes.Remove(guid);
    WD_DEFAULT_DELETE(pNode);
  }
}

void wdAbstractObjectNode::AddProperty(const char* szName, const wdVariant& value)
{
  auto& prop = m_Properties.ExpandAndGetRef();
  prop.m_szPropertyName = m_pOwner->RegisterString(szName);
  prop.m_Value = value;
}

void wdAbstractObjectNode::ChangeProperty(const char* szName, const wdVariant& value)
{
  for (wdUInt32 i = 0; i < m_Properties.GetCount(); ++i)
  {
    if (wdStringUtils::IsEqual(m_Properties[i].m_szPropertyName, szName))
    {
      m_Properties[i].m_Value = value;
      return;
    }
  }

  WD_REPORT_FAILURE("Property '{0}' is unknown", szName);
}

void wdAbstractObjectNode::RenameProperty(const char* szOldName, const char* szNewName)
{
  for (wdUInt32 i = 0; i < m_Properties.GetCount(); ++i)
  {
    if (wdStringUtils::IsEqual(m_Properties[i].m_szPropertyName, szOldName))
    {
      m_Properties[i].m_szPropertyName = m_pOwner->RegisterString(szNewName);
      return;
    }
  }
}

void wdAbstractObjectNode::ClearProperties()
{
  m_Properties.Clear();
}

wdResult wdAbstractObjectNode::InlineProperty(const char* szName)
{
  for (wdUInt32 i = 0; i < m_Properties.GetCount(); ++i)
  {
    Property& prop = m_Properties[i];
    if (wdStringUtils::IsEqual(prop.m_szPropertyName, szName))
    {
      if (!prop.m_Value.IsA<wdUuid>())
        return WD_FAILURE;

      wdUuid guid = prop.m_Value.Get<wdUuid>();
      wdAbstractObjectNode* pNode = m_pOwner->GetNode(guid);
      if (!pNode)
        return WD_FAILURE;

      class InlineContext : public wdRttiConverterContext
      {
      public:
        void RegisterObject(const wdUuid& guid, const wdRTTI* pRtti, void* pObject) override
        {
          m_SubTree.PushBack(guid);
        }
        wdHybridArray<wdUuid, 1> m_SubTree;
      };

      InlineContext context;
      wdRttiConverterReader reader(m_pOwner, &context);
      void* pObject = reader.CreateObjectFromNode(pNode);
      if (!pObject)
        return WD_FAILURE;

      prop.m_Value.MoveTypedObject(pObject, wdRTTI::FindTypeByName(pNode->GetType()));

      // Delete old objects.
      for (wdUuid& uuid : context.m_SubTree)
      {
        m_pOwner->RemoveNode(uuid);
      }
      return WD_SUCCESS;
    }
  }
  return WD_FAILURE;
}

void wdAbstractObjectNode::RemoveProperty(const char* szName)
{
  for (wdUInt32 i = 0; i < m_Properties.GetCount(); ++i)
  {
    if (wdStringUtils::IsEqual(m_Properties[i].m_szPropertyName, szName))
    {
      m_Properties.RemoveAtAndSwap(i);
      return;
    }
  }
}

void wdAbstractObjectNode::SetType(const char* szType)
{
  m_szType = m_pOwner->RegisterString(szType);
}

const wdAbstractObjectNode::Property* wdAbstractObjectNode::FindProperty(const char* szName) const
{
  for (wdUInt32 i = 0; i < m_Properties.GetCount(); ++i)
  {
    if (wdStringUtils::IsEqual(m_Properties[i].m_szPropertyName, szName))
    {
      return &m_Properties[i];
    }
  }

  return nullptr;
}

wdAbstractObjectNode::Property* wdAbstractObjectNode::FindProperty(const char* szName)
{
  for (wdUInt32 i = 0; i < m_Properties.GetCount(); ++i)
  {
    if (wdStringUtils::IsEqual(m_Properties[i].m_szPropertyName, szName))
    {
      return &m_Properties[i];
    }
  }

  return nullptr;
}

void wdAbstractObjectGraph::ReMapNodeGuids(const wdUuid& seedGuid, bool bRemapInverse /*= false*/)
{
  wdHybridArray<wdAbstractObjectNode*, 16> nodes;
  nodes.Reserve(m_Nodes.GetCount());
  wdHashTable<wdUuid, wdUuid> guidMap;
  guidMap.Reserve(m_Nodes.GetCount());

  for (auto it = m_Nodes.GetIterator(); it.IsValid(); ++it)
  {
    wdUuid newGuid = it.Key();

    if (bRemapInverse)
      newGuid.RevertCombinationWithSeed(seedGuid);
    else
      newGuid.CombineWithSeed(seedGuid);

    guidMap[it.Key()] = newGuid;

    nodes.PushBack(it.Value());
  }

  m_Nodes.Clear();

  // go through all nodes to remap guids
  for (auto* pNode : nodes)
  {
    pNode->m_Guid = guidMap[pNode->m_Guid];

    // check every property
    for (auto& prop : pNode->m_Properties)
    {
      RemapVariant(prop.m_Value, guidMap);
    }
    m_Nodes[pNode->m_Guid] = pNode;
  }
}


void wdAbstractObjectGraph::ReMapNodeGuidsToMatchGraph(wdAbstractObjectNode* pRoot, const wdAbstractObjectGraph& rhsGraph, const wdAbstractObjectNode* pRhsRoot)
{
  wdHashTable<wdUuid, wdUuid> guidMap;
  WD_ASSERT_DEV(wdStringUtils::IsEqual(pRoot->GetType(), pRhsRoot->GetType()), "Roots must have the same type to be able re-map guids!");

  ReMapNodeGuidsToMatchGraphRecursive(guidMap, pRoot, rhsGraph, pRhsRoot);

  // go through all nodes to remap remaining occurrences of remapped guids
  for (auto it : m_Nodes)
  {
    // check every property
    for (auto& prop : it.Value()->m_Properties)
    {
      RemapVariant(prop.m_Value, guidMap);
    }
    m_Nodes[it.Value()->m_Guid] = it.Value();
  }
}

void wdAbstractObjectGraph::ReMapNodeGuidsToMatchGraphRecursive(wdHashTable<wdUuid, wdUuid>& guidMap, wdAbstractObjectNode* lhs, const wdAbstractObjectGraph& rhsGraph, const wdAbstractObjectNode* rhs)
{
  if (!wdStringUtils::IsEqual(lhs->GetType(), rhs->GetType()))
  {
    // Types differ, remapping ends as this is a removal and add of a new object.
    return;
  }

  if (lhs->GetGuid() != rhs->GetGuid())
  {
    guidMap[lhs->GetGuid()] = rhs->GetGuid();
    m_Nodes.Remove(lhs->GetGuid());
    lhs->m_Guid = rhs->GetGuid();
    m_Nodes.Insert(rhs->GetGuid(), lhs);
  }

  for (wdAbstractObjectNode::Property& prop : lhs->m_Properties)
  {
    if (prop.m_Value.IsA<wdUuid>() && prop.m_Value.Get<wdUuid>().IsValid())
    {
      // if the guid is an owned object in the graph, remap to rhs.
      auto it = m_Nodes.Find(prop.m_Value.Get<wdUuid>());
      if (it.IsValid())
      {
        if (const wdAbstractObjectNode::Property* rhsProp = rhs->FindProperty(prop.m_szPropertyName))
        {
          if (rhsProp->m_Value.IsA<wdUuid>() && rhsProp->m_Value.Get<wdUuid>().IsValid())
          {
            if (const wdAbstractObjectNode* rhsPropNode = rhsGraph.GetNode(rhsProp->m_Value.Get<wdUuid>()))
            {
              ReMapNodeGuidsToMatchGraphRecursive(guidMap, it.Value(), rhsGraph, rhsPropNode);
            }
          }
        }
      }
    }
    // Arrays may be of owner guids and could be remapped.
    else if (prop.m_Value.IsA<wdVariantArray>())
    {
      const wdVariantArray& values = prop.m_Value.Get<wdVariantArray>();
      for (wdUInt32 i = 0; i < values.GetCount(); i++)
      {
        auto& subValue = values[i];
        if (subValue.IsA<wdUuid>() && subValue.Get<wdUuid>().IsValid())
        {
          // if the guid is an owned object in the graph, remap to array element.
          auto it = m_Nodes.Find(subValue.Get<wdUuid>());
          if (it.IsValid())
          {
            if (const wdAbstractObjectNode::Property* rhsProp = rhs->FindProperty(prop.m_szPropertyName))
            {
              if (rhsProp->m_Value.IsA<wdVariantArray>())
              {
                const wdVariantArray& rhsValues = rhsProp->m_Value.Get<wdVariantArray>();
                if (i < rhsValues.GetCount())
                {
                  const auto& rhsElemValue = rhsValues[i];
                  if (rhsElemValue.IsA<wdUuid>() && rhsElemValue.Get<wdUuid>().IsValid())
                  {
                    if (const wdAbstractObjectNode* rhsPropNode = rhsGraph.GetNode(rhsElemValue.Get<wdUuid>()))
                    {
                      ReMapNodeGuidsToMatchGraphRecursive(guidMap, it.Value(), rhsGraph, rhsPropNode);
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
    // Maps may be of owner guids and could be remapped.
    else if (prop.m_Value.IsA<wdVariantDictionary>())
    {
      const wdVariantDictionary& values = prop.m_Value.Get<wdVariantDictionary>();
      for (auto lhsIt = values.GetIterator(); lhsIt.IsValid(); ++lhsIt)
      {
        auto& subValue = lhsIt.Value();
        if (subValue.IsA<wdUuid>() && subValue.Get<wdUuid>().IsValid())
        {
          // if the guid is an owned object in the graph, remap to map element.
          auto it = m_Nodes.Find(subValue.Get<wdUuid>());
          if (it.IsValid())
          {
            if (const wdAbstractObjectNode::Property* rhsProp = rhs->FindProperty(prop.m_szPropertyName))
            {
              if (rhsProp->m_Value.IsA<wdVariantDictionary>())
              {
                const wdVariantDictionary& rhsValues = rhsProp->m_Value.Get<wdVariantDictionary>();
                if (rhsValues.Contains(lhsIt.Key()))
                {
                  const auto& rhsElemValue = *rhsValues.GetValue(lhsIt.Key());
                  if (rhsElemValue.IsA<wdUuid>() && rhsElemValue.Get<wdUuid>().IsValid())
                  {
                    if (const wdAbstractObjectNode* rhsPropNode = rhsGraph.GetNode(rhsElemValue.Get<wdUuid>()))
                    {
                      ReMapNodeGuidsToMatchGraphRecursive(guidMap, it.Value(), rhsGraph, rhsPropNode);
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}


void wdAbstractObjectGraph::FindTransitiveHull(const wdUuid& rootGuid, wdSet<wdUuid>& ref_reachableNodes) const
{
  ref_reachableNodes.Clear();
  wdSet<wdUuid> inProgress;
  inProgress.Insert(rootGuid);

  while (!inProgress.IsEmpty())
  {
    wdUuid current = *inProgress.GetIterator();
    auto it = m_Nodes.Find(current);
    if (it.IsValid())
    {
      const wdAbstractObjectNode* pNode = it.Value();
      for (auto& prop : pNode->m_Properties)
      {
        if (prop.m_Value.IsA<wdUuid>())
        {
          const wdUuid& guid = prop.m_Value.Get<wdUuid>();
          if (!ref_reachableNodes.Contains(guid))
          {
            inProgress.Insert(guid);
          }
        }
        // Arrays may be of uuids
        else if (prop.m_Value.IsA<wdVariantArray>())
        {
          const wdVariantArray& values = prop.m_Value.Get<wdVariantArray>();
          for (auto& subValue : values)
          {
            if (subValue.IsA<wdUuid>())
            {
              const wdUuid& guid = subValue.Get<wdUuid>();
              if (!ref_reachableNodes.Contains(guid))
              {
                inProgress.Insert(guid);
              }
            }
          }
        }
        else if (prop.m_Value.IsA<wdVariantDictionary>())
        {
          const wdVariantDictionary& values = prop.m_Value.Get<wdVariantDictionary>();
          for (auto& subValue : values)
          {
            if (subValue.Value().IsA<wdUuid>())
            {
              const wdUuid& guid = subValue.Value().Get<wdUuid>();
              if (!ref_reachableNodes.Contains(guid))
              {
                inProgress.Insert(guid);
              }
            }
          }
        }
      }
    }
    // Even if 'current' is not in the graph add it anyway to early out if it is found again.
    ref_reachableNodes.Insert(current);
    inProgress.Remove(current);
  }
}

void wdAbstractObjectGraph::PruneGraph(const wdUuid& rootGuid)
{
  wdSet<wdUuid> reachableNodes;
  FindTransitiveHull(rootGuid, reachableNodes);

  // Determine nodes to be removed by subtracting valid ones from all nodes.
  wdSet<wdUuid> removeSet;
  for (auto it = GetAllNodes().GetIterator(); it.IsValid(); ++it)
  {
    removeSet.Insert(it.Key());
  }
  removeSet.Difference(reachableNodes);

  // Remove nodes.
  for (const wdUuid& guid : removeSet)
  {
    RemoveNode(guid);
  }
}

void wdAbstractObjectGraph::ModifyNodeViaNativeCounterpart(wdAbstractObjectNode* pRootNode, wdDelegate<void(void*, const wdRTTI*)> callback)
{
  WD_ASSERT_DEV(pRootNode->GetOwner() == this, "Node must be from this graph.");

  // Clone sub graph
  wdAbstractObjectGraph origGraph;
  wdAbstractObjectNode* pOrigRootNode = nullptr;
  {
    pOrigRootNode = Clone(origGraph, pRootNode);
  }

  // Create native object
  wdRttiConverterContext context;
  wdRttiConverterReader convRead(&origGraph, &context);
  void* pNativeRoot = convRead.CreateObjectFromNode(pOrigRootNode);
  const wdRTTI* pType = wdRTTI::FindTypeByName(pOrigRootNode->GetType());
  WD_SCOPE_EXIT(pType->GetAllocator()->Deallocate(pNativeRoot););

  // Make changes to native object
  if (callback.IsValid())
  {
    callback(pNativeRoot, pType);
  }

  // Create native object graph
  wdAbstractObjectGraph graph;
  wdAbstractObjectNode* pRootNode2 = nullptr;
  {
    // The wdApplyNativePropertyChangesContext takes care of generating guids for native pointers that match those
    // of the object manager.
    wdApplyNativePropertyChangesContext nativeChangesContext(context, origGraph);
    wdRttiConverterWriter rttiConverter(&graph, &nativeChangesContext, true, true);
    nativeChangesContext.RegisterObject(pOrigRootNode->GetGuid(), pType, pNativeRoot);
    pRootNode2 = rttiConverter.AddObjectToGraph(pType, pNativeRoot, "Object");
  }

  // Create diff from native to cloned sub-graph and then apply the diff to the original graph.
  wdDeque<wdAbstractGraphDiffOperation> diffResult;
  graph.CreateDiffWithBaseGraph(origGraph, diffResult);

  ApplyDiff(diffResult);
}

wdAbstractObjectNode* wdAbstractObjectGraph::CopyNodeIntoGraph(const wdAbstractObjectNode* pNode)
{
  auto pNewNode = AddNode(pNode->GetGuid(), pNode->GetType(), pNode->GetTypeVersion(), pNode->GetNodeName());

  for (const auto& props : pNode->GetProperties())
    pNewNode->AddProperty(props.m_szPropertyName, props.m_Value);

  return pNewNode;
}

wdAbstractObjectNode* wdAbstractObjectGraph::CopyNodeIntoGraph(const wdAbstractObjectNode* pNode, FilterFunction& ref_filter)
{
  auto pNewNode = AddNode(pNode->GetGuid(), pNode->GetType(), pNode->GetTypeVersion(), pNode->GetNodeName());

  if (ref_filter.IsValid())
  {
    for (const auto& props : pNode->GetProperties())
    {
      if (!ref_filter(pNode, &props))
        continue;
      pNewNode->AddProperty(props.m_szPropertyName, props.m_Value);
    }
  }
  else
  {
    for (const auto& props : pNode->GetProperties())
      pNewNode->AddProperty(props.m_szPropertyName, props.m_Value);
  }

  return pNewNode;
}

void wdAbstractObjectGraph::CreateDiffWithBaseGraph(const wdAbstractObjectGraph& base, wdDeque<wdAbstractGraphDiffOperation>& out_diffResult) const
{
  out_diffResult.Clear();

  // check whether any nodes have been deleted
  {
    for (auto itNodeBase = base.GetAllNodes().GetIterator(); itNodeBase.IsValid(); ++itNodeBase)
    {
      if (GetNode(itNodeBase.Key()) == nullptr)
      {
        // does not exist in this graph -> has been deleted from base
        wdAbstractGraphDiffOperation op;
        op.m_Node = itNodeBase.Key();
        op.m_Operation = wdAbstractGraphDiffOperation::Op::NodeRemoved;
        op.m_sProperty = itNodeBase.Value()->m_szType;
        op.m_Value = itNodeBase.Value()->m_szNodeName;

        out_diffResult.PushBack(op);
      }
    }
  }

  // check whether any nodes have been added
  {
    for (auto itNodeThis = GetAllNodes().GetIterator(); itNodeThis.IsValid(); ++itNodeThis)
    {
      if (base.GetNode(itNodeThis.Key()) == nullptr)
      {
        // does not exist in base graph -> has been added
        wdAbstractGraphDiffOperation op;
        op.m_Node = itNodeThis.Key();
        op.m_Operation = wdAbstractGraphDiffOperation::Op::NodeAdded;
        op.m_sProperty = itNodeThis.Value()->m_szType;
        op.m_Value = itNodeThis.Value()->m_szNodeName;

        out_diffResult.PushBack(op);

        // set all properties
        for (const auto& prop : itNodeThis.Value()->GetProperties())
        {
          op.m_Operation = wdAbstractGraphDiffOperation::Op::PropertyChanged;
          op.m_sProperty = prop.m_szPropertyName;
          op.m_Value = prop.m_Value;

          out_diffResult.PushBack(op);
        }
      }
    }
  }

  // check whether any properties have been modified
  {
    for (auto itNodeThis = GetAllNodes().GetIterator(); itNodeThis.IsValid(); ++itNodeThis)
    {
      const auto pBaseNode = base.GetNode(itNodeThis.Key());

      if (pBaseNode == nullptr)
        continue;

      for (const wdAbstractObjectNode::Property& prop : itNodeThis.Value()->GetProperties())
      {
        bool bDifferent = true;

        for (const wdAbstractObjectNode::Property& baseProp : pBaseNode->GetProperties())
        {
          if (wdStringUtils::IsEqual(baseProp.m_szPropertyName, prop.m_szPropertyName))
          {
            if (baseProp.m_Value == prop.m_Value)
            {
              bDifferent = false;
              break;
            }

            bDifferent = true;
            break;
          }
        }

        if (bDifferent)
        {
          wdAbstractGraphDiffOperation op;
          op.m_Node = itNodeThis.Key();
          op.m_Operation = wdAbstractGraphDiffOperation::Op::PropertyChanged;
          op.m_sProperty = prop.m_szPropertyName;
          op.m_Value = prop.m_Value;

          out_diffResult.PushBack(op);
        }
      }
    }
  }
}


void wdAbstractObjectGraph::ApplyDiff(wdDeque<wdAbstractGraphDiffOperation>& ref_diff)
{
  for (const auto& op : ref_diff)
  {
    switch (op.m_Operation)
    {
      case wdAbstractGraphDiffOperation::Op::NodeAdded:
      {
        AddNode(op.m_Node, op.m_sProperty, op.m_uiTypeVersion, op.m_Value.Get<wdString>());
      }
      break;

      case wdAbstractGraphDiffOperation::Op::NodeRemoved:
      {
        RemoveNode(op.m_Node);
      }
      break;

      case wdAbstractGraphDiffOperation::Op::PropertyChanged:
      {
        auto* pNode = GetNode(op.m_Node);
        if (pNode)
        {
          auto* pProp = pNode->FindProperty(op.m_sProperty);

          if (!pProp)
            pNode->AddProperty(op.m_sProperty, op.m_Value);
          else
            pProp->m_Value = op.m_Value;
        }
      }
      break;
    }
  }
}


void wdAbstractObjectGraph::MergeDiffs(const wdDeque<wdAbstractGraphDiffOperation>& lhs, const wdDeque<wdAbstractGraphDiffOperation>& rhs, wdDeque<wdAbstractGraphDiffOperation>& ref_out) const
{
  struct Prop
  {
    Prop() {}
    Prop(wdUuid node, wdStringView sProperty)
    {
      m_Node = node;
      m_sProperty = sProperty;
    }
    wdUuid m_Node;
    wdStringView m_sProperty;

    bool operator<(const Prop& rhs) const
    {
      if (m_Node == rhs.m_Node)
        return m_sProperty < rhs.m_sProperty;

      return m_Node < rhs.m_Node;
    }

    bool operator==(const Prop& rhs) const { return m_Node == rhs.m_Node && m_sProperty == rhs.m_sProperty; }
  };

  wdMap<Prop, wdHybridArray<const wdAbstractGraphDiffOperation*, 2>> propChanges;
  wdSet<wdUuid> removed;
  wdMap<wdUuid, wdUInt32> added;
  for (const wdAbstractGraphDiffOperation& op : lhs)
  {
    if (op.m_Operation == wdAbstractGraphDiffOperation::Op::NodeRemoved)
    {
      removed.Insert(op.m_Node);
      ref_out.PushBack(op);
    }
    else if (op.m_Operation == wdAbstractGraphDiffOperation::Op::NodeAdded)
    {
      added[op.m_Node] = ref_out.GetCount();
      ref_out.PushBack(op);
    }
    else if (op.m_Operation == wdAbstractGraphDiffOperation::Op::PropertyChanged)
    {
      auto it = propChanges.FindOrAdd(Prop(op.m_Node, op.m_sProperty));
      it.Value().PushBack(&op);
    }
  }
  for (const wdAbstractGraphDiffOperation& op : rhs)
  {
    if (op.m_Operation == wdAbstractGraphDiffOperation::Op::NodeRemoved)
    {
      if (!removed.Contains(op.m_Node))
        ref_out.PushBack(op);
    }
    else if (op.m_Operation == wdAbstractGraphDiffOperation::Op::NodeAdded)
    {
      if (added.Contains(op.m_Node))
      {
        wdAbstractGraphDiffOperation& leftOp = ref_out[added[op.m_Node]];
        leftOp.m_sProperty = op.m_sProperty; // Take type from rhs.
      }
      else
      {
        ref_out.PushBack(op);
      }
    }
    else if (op.m_Operation == wdAbstractGraphDiffOperation::Op::PropertyChanged)
    {
      auto it = propChanges.FindOrAdd(Prop(op.m_Node, op.m_sProperty));
      it.Value().PushBack(&op);
    }
  }

  for (auto it = propChanges.GetIterator(); it.IsValid(); ++it)
  {
    const Prop& key = it.Key();
    const wdHybridArray<const wdAbstractGraphDiffOperation*, 2>& value = it.Value();

    if (value.GetCount() == 1)
    {
      ref_out.PushBack(*value[0]);
    }
    else
    {
      const wdAbstractGraphDiffOperation& leftProp = *value[0];
      const wdAbstractGraphDiffOperation& rightProp = *value[1];

      if (leftProp.m_Value.GetType() == wdVariantType::VariantArray && rightProp.m_Value.GetType() == wdVariantType::VariantArray)
      {
        const wdVariantArray& leftArray = leftProp.m_Value.Get<wdVariantArray>();
        const wdVariantArray& rightArray = rightProp.m_Value.Get<wdVariantArray>();

        const wdAbstractObjectNode* pNode = GetNode(key.m_Node);
        if (pNode)
        {
          wdStringBuilder sTemp(key.m_sProperty);
          const wdAbstractObjectNode::Property* pProperty = pNode->FindProperty(sTemp);
          if (pProperty && pProperty->m_Value.GetType() == wdVariantType::VariantArray)
          {
            // Do 3-way array merge
            const wdVariantArray& baseArray = pProperty->m_Value.Get<wdVariantArray>();
            wdVariantArray res;
            MergeArrays(baseArray, leftArray, rightArray, res);
            ref_out.PushBack(rightProp);
            ref_out.PeekBack().m_Value = res;
          }
          else
          {
            ref_out.PushBack(rightProp);
          }
        }
        else
        {
          ref_out.PushBack(rightProp);
        }
      }
      else
      {
        ref_out.PushBack(rightProp);
      }
    }
  }
}

void wdAbstractObjectGraph::RemapVariant(wdVariant& value, const wdHashTable<wdUuid, wdUuid>& guidMap)
{
  wdStringBuilder tmp;

  // if the property is a guid, we check if we need to remap it
  if (value.IsA<wdUuid>())
  {
    const wdUuid& guid = value.Get<wdUuid>();

    // if we find the guid in our map, replace it by the new guid
    if (auto* found = guidMap.GetValue(guid))
    {
      value = *found;
    }
  }
  else if (value.IsA<wdString>() && wdConversionUtils::IsStringUuid(value.Get<wdString>()))
  {
    const wdUuid guid = wdConversionUtils::ConvertStringToUuid(value.Get<wdString>());

    // if we find the guid in our map, replace it by the new guid
    if (auto* found = guidMap.GetValue(guid))
    {
      value = wdConversionUtils::ToString(*found, tmp).GetData();
    }
  }
  // Arrays may be of uuids
  else if (value.IsA<wdVariantArray>())
  {
    const wdVariantArray& values = value.Get<wdVariantArray>();
    bool bNeedToRemap = false;
    for (auto& subValue : values)
    {
      if (subValue.IsA<wdUuid>() && guidMap.Contains(subValue.Get<wdUuid>()))
      {
        bNeedToRemap = true;
        break;
      }
      else if (subValue.IsA<wdString>() && wdConversionUtils::IsStringUuid(subValue.Get<wdString>()))
      {
        bNeedToRemap = true;
        break;
      }
      else if (subValue.IsA<wdVariantArray>())
      {
        bNeedToRemap = true;
        break;
      }
    }

    if (bNeedToRemap)
    {
      wdVariantArray newValues = values;
      for (auto& subValue : newValues)
      {
        RemapVariant(subValue, guidMap);
      }
      value = newValues;
    }
  }
  // Maps may be of uuids
  else if (value.IsA<wdVariantDictionary>())
  {
    const wdVariantDictionary& values = value.Get<wdVariantDictionary>();
    bool bNeedToRemap = false;
    for (auto it = values.GetIterator(); it.IsValid(); ++it)
    {
      const wdVariant& subValue = it.Value();

      if (subValue.IsA<wdUuid>() && guidMap.Contains(subValue.Get<wdUuid>()))
      {
        bNeedToRemap = true;
        break;
      }
      else if (subValue.IsA<wdString>() && wdConversionUtils::IsStringUuid(subValue.Get<wdString>()))
      {
        bNeedToRemap = true;
        break;
      }
    }

    if (bNeedToRemap)
    {
      wdVariantDictionary newValues = values;
      for (auto it = newValues.GetIterator(); it.IsValid(); ++it)
      {
        RemapVariant(it.Value(), guidMap);
      }
      value = newValues;
    }
  }
}

void wdAbstractObjectGraph::MergeArrays(const wdDynamicArray<wdVariant>& baseArray, const wdDynamicArray<wdVariant>& leftArray, const wdDynamicArray<wdVariant>& rightArray, wdDynamicArray<wdVariant>& out) const
{
  // Find element type.
  wdVariantType::Enum type = wdVariantType::Invalid;
  if (!baseArray.IsEmpty())
    type = baseArray[0].GetType();
  if (type != wdVariantType::Invalid && !leftArray.IsEmpty())
    type = leftArray[0].GetType();
  if (type != wdVariantType::Invalid && !rightArray.IsEmpty())
    type = rightArray[0].GetType();

  if (type == wdVariantType::Invalid)
    return;

  // For now, assume non-uuid types are arrays, uuids are sets.
  if (type != wdVariantType::Uuid)
  {
    // Any size changes?
    wdUInt32 uiSize = baseArray.GetCount();
    if (leftArray.GetCount() != baseArray.GetCount())
      uiSize = leftArray.GetCount();
    if (rightArray.GetCount() != baseArray.GetCount())
      uiSize = rightArray.GetCount();

    out.SetCount(uiSize);
    for (wdUInt32 i = 0; i < uiSize; i++)
    {
      if (i < baseArray.GetCount())
        out[i] = baseArray[i];
    }

    wdUInt32 uiCountLeft = wdMath::Min(uiSize, leftArray.GetCount());
    for (wdUInt32 i = 0; i < uiCountLeft; i++)
    {
      if (leftArray[i] != baseArray[i])
        out[i] = leftArray[i];
    }

    wdUInt32 uiCountRight = wdMath::Min(uiSize, rightArray.GetCount());
    for (wdUInt32 i = 0; i < uiCountRight; i++)
    {
      if (rightArray[i] != baseArray[i])
        out[i] = rightArray[i];
    }
    return;
  }

  // Move distance is NP-complete so try greedy algorithm
  struct Element
  {
    Element(const wdVariant* pValue = nullptr, wdInt32 iBaseIndex = -1, wdInt32 iLeftIndex = -1, wdInt32 iRightIndex = -1)
      : m_pValue(pValue)
      , m_iBaseIndex(iBaseIndex)
      , m_iLeftIndex(iLeftIndex)
      , m_iRightIndex(iRightIndex)
      , m_fIndex(wdMath::MaxValue<float>())
    {
    }
    bool IsDeleted() const { return m_iBaseIndex != -1 && (m_iLeftIndex == -1 || m_iRightIndex == -1); }
    bool operator<(const Element& rhs) const { return m_fIndex < rhs.m_fIndex; }

    const wdVariant* m_pValue;
    wdInt32 m_iBaseIndex;
    wdInt32 m_iLeftIndex;
    wdInt32 m_iRightIndex;
    float m_fIndex;
  };
  wdDynamicArray<Element> baseOrder;
  baseOrder.Reserve(leftArray.GetCount() + rightArray.GetCount());

  // First, add up all unique elements and their position in each array.
  for (wdInt32 i = 0; i < (wdInt32)baseArray.GetCount(); i++)
  {
    baseOrder.PushBack(Element(&baseArray[i], i));
    baseOrder.PeekBack().m_fIndex = (float)i;
  }

  wdDynamicArray<wdInt32> leftOrder;
  leftOrder.SetCountUninitialized(leftArray.GetCount());
  for (wdInt32 i = 0; i < (wdInt32)leftArray.GetCount(); i++)
  {
    const wdVariant& val = leftArray[i];
    bool bFound = false;
    for (wdInt32 j = 0; j < (wdInt32)baseOrder.GetCount(); j++)
    {
      Element& elem = baseOrder[j];
      if (elem.m_iLeftIndex == -1 && *elem.m_pValue == val)
      {
        elem.m_iLeftIndex = i;
        leftOrder[i] = j;
        bFound = true;
        break;
      }
    }

    if (!bFound)
    {
      // Added element.
      leftOrder[i] = (wdInt32)baseOrder.GetCount();
      baseOrder.PushBack(Element(&leftArray[i], -1, i));
    }
  }

  wdDynamicArray<wdInt32> rightOrder;
  rightOrder.SetCountUninitialized(rightArray.GetCount());
  for (wdInt32 i = 0; i < (wdInt32)rightArray.GetCount(); i++)
  {
    const wdVariant& val = rightArray[i];
    bool bFound = false;
    for (wdInt32 j = 0; j < (wdInt32)baseOrder.GetCount(); j++)
    {
      Element& elem = baseOrder[j];
      if (elem.m_iRightIndex == -1 && *elem.m_pValue == val)
      {
        elem.m_iRightIndex = i;
        rightOrder[i] = j;
        bFound = true;
        break;
      }
    }

    if (!bFound)
    {
      // Added element.
      rightOrder[i] = (wdInt32)baseOrder.GetCount();
      baseOrder.PushBack(Element(&rightArray[i], -1, -1, i));
    }
  }

  // Re-order greedy
  float fLastElement = -0.5f;
  for (wdInt32 i = 0; i < (wdInt32)leftOrder.GetCount(); i++)
  {
    Element& currentElem = baseOrder[leftOrder[i]];
    if (currentElem.IsDeleted())
      continue;

    float fLowestSubsequent = wdMath::MaxValue<float>();
    for (wdInt32 j = i + 1; j < (wdInt32)leftOrder.GetCount(); j++)
    {
      Element& elem = baseOrder[leftOrder[j]];
      if (elem.IsDeleted())
        continue;

      if (elem.m_iBaseIndex < fLowestSubsequent)
      {
        fLowestSubsequent = (float)elem.m_iBaseIndex;
      }
    }

    if (currentElem.m_fIndex >= fLowestSubsequent)
    {
      currentElem.m_fIndex = (fLowestSubsequent + fLastElement) / 2.0f;
    }

    fLastElement = currentElem.m_fIndex;
  }

  fLastElement = -0.5f;
  for (wdInt32 i = 0; i < (wdInt32)rightOrder.GetCount(); i++)
  {
    Element& currentElem = baseOrder[rightOrder[i]];
    if (currentElem.IsDeleted())
      continue;

    float fLowestSubsequent = wdMath::MaxValue<float>();
    for (wdInt32 j = i + 1; j < (wdInt32)rightOrder.GetCount(); j++)
    {
      Element& elem = baseOrder[rightOrder[j]];
      if (elem.IsDeleted())
        continue;

      if (elem.m_iBaseIndex < fLowestSubsequent)
      {
        fLowestSubsequent = (float)elem.m_iBaseIndex;
      }
    }

    if (currentElem.m_fIndex >= fLowestSubsequent)
    {
      currentElem.m_fIndex = (fLowestSubsequent + fLastElement) / 2.0f;
    }

    fLastElement = currentElem.m_fIndex;
  }


  // Sort
  baseOrder.Sort();
  out.Reserve(baseOrder.GetCount());
  for (const Element& elem : baseOrder)
  {
    if (!elem.IsDeleted())
    {
      out.PushBack(*elem.m_pValue);
    }
  }
}

WD_STATICLINK_FILE(Foundation, Foundation_Serialization_Implementation_AbstractObjectGraph);
