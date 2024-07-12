#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/Log.h>
#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Serialization/ApplyNativePropertyChangesContext.h>
#include <Foundation/Serialization/RttiConverter.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_ENUM(nsObjectChangeType, 1)
  NS_ENUM_CONSTANTS(nsObjectChangeType::NodeAdded, nsObjectChangeType::NodeRemoved)
  NS_ENUM_CONSTANTS(nsObjectChangeType::PropertySet, nsObjectChangeType::PropertyInserted, nsObjectChangeType::PropertyRemoved)
NS_END_STATIC_REFLECTED_ENUM;

NS_BEGIN_STATIC_REFLECTED_TYPE(nsAbstractObjectNode, nsNoBase, 1, nsRTTIDefaultAllocator<nsAbstractObjectNode>)
NS_END_STATIC_REFLECTED_TYPE;

NS_BEGIN_STATIC_REFLECTED_TYPE(nsDiffOperation, nsNoBase, 1, nsRTTIDefaultAllocator<nsDiffOperation>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ENUM_MEMBER_PROPERTY("Operation", nsObjectChangeType, m_Operation),
    NS_MEMBER_PROPERTY("Node", m_Node),
    NS_MEMBER_PROPERTY("Property", m_sProperty),
    NS_MEMBER_PROPERTY("Index", m_Index),
    NS_MEMBER_PROPERTY("Value", m_Value),
  }
    NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

nsAbstractObjectGraph::~nsAbstractObjectGraph()
{
  Clear();
}

void nsAbstractObjectGraph::Clear()
{
  for (auto it = m_Nodes.GetIterator(); it.IsValid(); ++it)
  {
    NS_DEFAULT_DELETE(it.Value());
  }
  m_Nodes.Clear();
  m_NodesByName.Clear();
  m_Strings.Clear();
}


nsAbstractObjectNode* nsAbstractObjectGraph::Clone(nsAbstractObjectGraph& ref_cloneTarget, const nsAbstractObjectNode* pRootNode, FilterFunction filter) const
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
    NS_ASSERT_DEV(pRootNode->GetOwner() == this, "The given root node must be part of this document");
    nsSet<nsUuid> reachableNodes;
    FindTransitiveHull(pRootNode->GetGuid(), reachableNodes);

    for (const nsUuid& guid : reachableNodes)
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

nsStringView nsAbstractObjectGraph::RegisterString(nsStringView sString)
{
  auto it = m_Strings.Insert(sString);
  NS_ASSERT_DEV(it.IsValid(), "");
  return it.Key();
}

nsAbstractObjectNode* nsAbstractObjectGraph::GetNode(const nsUuid& guid)
{
  return m_Nodes.GetValueOrDefault(guid, nullptr);
}

const nsAbstractObjectNode* nsAbstractObjectGraph::GetNode(const nsUuid& guid) const
{
  return const_cast<nsAbstractObjectGraph*>(this)->GetNode(guid);
}

const nsAbstractObjectNode* nsAbstractObjectGraph::GetNodeByName(nsStringView sName) const
{
  return const_cast<nsAbstractObjectGraph*>(this)->GetNodeByName(sName);
}

nsAbstractObjectNode* nsAbstractObjectGraph::GetNodeByName(nsStringView sName)
{
  return m_NodesByName.GetValueOrDefault(sName, nullptr);
}

nsAbstractObjectNode* nsAbstractObjectGraph::AddNode(const nsUuid& guid, nsStringView sType, nsUInt32 uiTypeVersion, nsStringView sNodeName)
{
  NS_ASSERT_DEV(!m_Nodes.Contains(guid), "object {0} must not yet exist", guid);
  if (!sNodeName.IsEmpty())
  {
    sNodeName = RegisterString(sNodeName);
  }
  else
  {
    sNodeName = {};
  }

  nsAbstractObjectNode* pNode = NS_DEFAULT_NEW(nsAbstractObjectNode);
  pNode->m_Guid = guid;
  pNode->m_pOwner = this;
  pNode->m_sType = RegisterString(sType);
  pNode->m_uiTypeVersion = uiTypeVersion;
  pNode->m_sNodeName = sNodeName;

  m_Nodes[guid] = pNode;

  if (!sNodeName.IsEmpty())
  {
    m_NodesByName[sNodeName] = pNode;
  }

  return pNode;
}

void nsAbstractObjectGraph::RemoveNode(const nsUuid& guid)
{
  auto it = m_Nodes.Find(guid);

  if (it.IsValid())
  {
    nsAbstractObjectNode* pNode = it.Value();
    if (!pNode->m_sNodeName.IsEmpty())
      m_NodesByName.Remove(pNode->m_sNodeName);

    m_Nodes.Remove(guid);
    NS_DEFAULT_DELETE(pNode);
  }
}

void nsAbstractObjectNode::AddProperty(nsStringView sName, const nsVariant& value)
{
  auto& prop = m_Properties.ExpandAndGetRef();
  prop.m_sPropertyName = m_pOwner->RegisterString(sName);
  prop.m_Value = value;
}

void nsAbstractObjectNode::ChangeProperty(nsStringView sName, const nsVariant& value)
{
  for (nsUInt32 i = 0; i < m_Properties.GetCount(); ++i)
  {
    if (m_Properties[i].m_sPropertyName == sName)
    {
      m_Properties[i].m_Value = value;
      return;
    }
  }

  NS_REPORT_FAILURE("Property '{0}' is unknown", sName);
}

void nsAbstractObjectNode::RenameProperty(nsStringView sOldName, nsStringView sNewName)
{
  for (nsUInt32 i = 0; i < m_Properties.GetCount(); ++i)
  {
    if (m_Properties[i].m_sPropertyName == sOldName)
    {
      m_Properties[i].m_sPropertyName = m_pOwner->RegisterString(sNewName);
      return;
    }
  }
}

void nsAbstractObjectNode::ClearProperties()
{
  m_Properties.Clear();
}

nsResult nsAbstractObjectNode::InlineProperty(nsStringView sName)
{
  for (nsUInt32 i = 0; i < m_Properties.GetCount(); ++i)
  {
    Property& prop = m_Properties[i];
    if (prop.m_sPropertyName == sName)
    {
      if (!prop.m_Value.IsA<nsUuid>())
        return NS_FAILURE;

      nsUuid guid = prop.m_Value.Get<nsUuid>();
      nsAbstractObjectNode* pNode = m_pOwner->GetNode(guid);
      if (!pNode)
        return NS_FAILURE;

      class InlineContext : public nsRttiConverterContext
      {
      public:
        void RegisterObject(const nsUuid& guid, const nsRTTI* pRtti, void* pObject) override
        {
          m_SubTree.PushBack(guid);
        }
        nsHybridArray<nsUuid, 1> m_SubTree;
      };

      InlineContext context;
      nsRttiConverterReader reader(m_pOwner, &context);
      void* pObject = reader.CreateObjectFromNode(pNode);
      if (!pObject)
        return NS_FAILURE;

      prop.m_Value.MoveTypedObject(pObject, nsRTTI::FindTypeByName(pNode->GetType()));

      // Delete old objects.
      for (nsUuid& uuid : context.m_SubTree)
      {
        m_pOwner->RemoveNode(uuid);
      }
      return NS_SUCCESS;
    }
  }
  return NS_FAILURE;
}

void nsAbstractObjectNode::RemoveProperty(nsStringView sName)
{
  for (nsUInt32 i = 0; i < m_Properties.GetCount(); ++i)
  {
    if (m_Properties[i].m_sPropertyName == sName)
    {
      m_Properties.RemoveAtAndSwap(i);
      return;
    }
  }
}

void nsAbstractObjectNode::SetType(nsStringView sType)
{
  m_sType = m_pOwner->RegisterString(sType);
}

const nsAbstractObjectNode::Property* nsAbstractObjectNode::FindProperty(nsStringView sName) const
{
  for (nsUInt32 i = 0; i < m_Properties.GetCount(); ++i)
  {
    if (m_Properties[i].m_sPropertyName == sName)
    {
      return &m_Properties[i];
    }
  }

  return nullptr;
}

nsAbstractObjectNode::Property* nsAbstractObjectNode::FindProperty(nsStringView sName)
{
  for (nsUInt32 i = 0; i < m_Properties.GetCount(); ++i)
  {
    if (m_Properties[i].m_sPropertyName == sName)
    {
      return &m_Properties[i];
    }
  }

  return nullptr;
}

void nsAbstractObjectGraph::ReMapNodeGuids(const nsUuid& seedGuid, bool bRemapInverse /*= false*/)
{
  nsHybridArray<nsAbstractObjectNode*, 16> nodes;
  nodes.Reserve(m_Nodes.GetCount());
  nsHashTable<nsUuid, nsUuid> guidMap;
  guidMap.Reserve(m_Nodes.GetCount());

  for (auto it = m_Nodes.GetIterator(); it.IsValid(); ++it)
  {
    nsUuid newGuid = it.Key();

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


void nsAbstractObjectGraph::ReMapNodeGuidsToMatchGraph(nsAbstractObjectNode* pRoot, const nsAbstractObjectGraph& rhsGraph, const nsAbstractObjectNode* pRhsRoot)
{
  nsHashTable<nsUuid, nsUuid> guidMap;
  NS_ASSERT_DEV(pRoot->GetType() == pRhsRoot->GetType(), "Roots must have the same type to be able re-map guids!");

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

void nsAbstractObjectGraph::ReMapNodeGuidsToMatchGraphRecursive(nsHashTable<nsUuid, nsUuid>& guidMap, nsAbstractObjectNode* lhs, const nsAbstractObjectGraph& rhsGraph, const nsAbstractObjectNode* rhs)
{
  if (lhs->GetType() != rhs->GetType())
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

  for (nsAbstractObjectNode::Property& prop : lhs->m_Properties)
  {
    if (prop.m_Value.IsA<nsUuid>() && prop.m_Value.Get<nsUuid>().IsValid())
    {
      // if the guid is an owned object in the graph, remap to rhs.
      auto it = m_Nodes.Find(prop.m_Value.Get<nsUuid>());
      if (it.IsValid())
      {
        if (const nsAbstractObjectNode::Property* rhsProp = rhs->FindProperty(prop.m_sPropertyName))
        {
          if (rhsProp->m_Value.IsA<nsUuid>() && rhsProp->m_Value.Get<nsUuid>().IsValid())
          {
            if (const nsAbstractObjectNode* rhsPropNode = rhsGraph.GetNode(rhsProp->m_Value.Get<nsUuid>()))
            {
              ReMapNodeGuidsToMatchGraphRecursive(guidMap, it.Value(), rhsGraph, rhsPropNode);
            }
          }
        }
      }
    }
    // Arrays may be of owner guids and could be remapped.
    else if (prop.m_Value.IsA<nsVariantArray>())
    {
      const nsVariantArray& values = prop.m_Value.Get<nsVariantArray>();
      for (nsUInt32 i = 0; i < values.GetCount(); i++)
      {
        auto& subValue = values[i];
        if (subValue.IsA<nsUuid>() && subValue.Get<nsUuid>().IsValid())
        {
          // if the guid is an owned object in the graph, remap to array element.
          auto it = m_Nodes.Find(subValue.Get<nsUuid>());
          if (it.IsValid())
          {
            if (const nsAbstractObjectNode::Property* rhsProp = rhs->FindProperty(prop.m_sPropertyName))
            {
              if (rhsProp->m_Value.IsA<nsVariantArray>())
              {
                const nsVariantArray& rhsValues = rhsProp->m_Value.Get<nsVariantArray>();
                if (i < rhsValues.GetCount())
                {
                  const auto& rhsElemValue = rhsValues[i];
                  if (rhsElemValue.IsA<nsUuid>() && rhsElemValue.Get<nsUuid>().IsValid())
                  {
                    if (const nsAbstractObjectNode* rhsPropNode = rhsGraph.GetNode(rhsElemValue.Get<nsUuid>()))
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
    else if (prop.m_Value.IsA<nsVariantDictionary>())
    {
      const nsVariantDictionary& values = prop.m_Value.Get<nsVariantDictionary>();
      for (auto lhsIt = values.GetIterator(); lhsIt.IsValid(); ++lhsIt)
      {
        auto& subValue = lhsIt.Value();
        if (subValue.IsA<nsUuid>() && subValue.Get<nsUuid>().IsValid())
        {
          // if the guid is an owned object in the graph, remap to map element.
          auto it = m_Nodes.Find(subValue.Get<nsUuid>());
          if (it.IsValid())
          {
            if (const nsAbstractObjectNode::Property* rhsProp = rhs->FindProperty(prop.m_sPropertyName))
            {
              if (rhsProp->m_Value.IsA<nsVariantDictionary>())
              {
                const nsVariantDictionary& rhsValues = rhsProp->m_Value.Get<nsVariantDictionary>();
                if (rhsValues.Contains(lhsIt.Key()))
                {
                  const auto& rhsElemValue = *rhsValues.GetValue(lhsIt.Key());
                  if (rhsElemValue.IsA<nsUuid>() && rhsElemValue.Get<nsUuid>().IsValid())
                  {
                    if (const nsAbstractObjectNode* rhsPropNode = rhsGraph.GetNode(rhsElemValue.Get<nsUuid>()))
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


void nsAbstractObjectGraph::FindTransitiveHull(const nsUuid& rootGuid, nsSet<nsUuid>& ref_reachableNodes) const
{
  ref_reachableNodes.Clear();
  nsSet<nsUuid> inProgress;
  inProgress.Insert(rootGuid);

  while (!inProgress.IsEmpty())
  {
    nsUuid current = *inProgress.GetIterator();
    auto it = m_Nodes.Find(current);
    if (it.IsValid())
    {
      const nsAbstractObjectNode* pNode = it.Value();
      for (auto& prop : pNode->m_Properties)
      {
        if (prop.m_Value.IsA<nsUuid>())
        {
          const nsUuid& guid = prop.m_Value.Get<nsUuid>();
          if (!ref_reachableNodes.Contains(guid))
          {
            inProgress.Insert(guid);
          }
        }
        // Arrays may be of uuids
        else if (prop.m_Value.IsA<nsVariantArray>())
        {
          const nsVariantArray& values = prop.m_Value.Get<nsVariantArray>();
          for (auto& subValue : values)
          {
            if (subValue.IsA<nsUuid>())
            {
              const nsUuid& guid = subValue.Get<nsUuid>();
              if (!ref_reachableNodes.Contains(guid))
              {
                inProgress.Insert(guid);
              }
            }
          }
        }
        else if (prop.m_Value.IsA<nsVariantDictionary>())
        {
          const nsVariantDictionary& values = prop.m_Value.Get<nsVariantDictionary>();
          for (auto& subValue : values)
          {
            if (subValue.Value().IsA<nsUuid>())
            {
              const nsUuid& guid = subValue.Value().Get<nsUuid>();
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

void nsAbstractObjectGraph::PruneGraph(const nsUuid& rootGuid)
{
  nsSet<nsUuid> reachableNodes;
  FindTransitiveHull(rootGuid, reachableNodes);

  // Determine nodes to be removed by subtracting valid ones from all nodes.
  nsSet<nsUuid> removeSet;
  for (auto it = GetAllNodes().GetIterator(); it.IsValid(); ++it)
  {
    removeSet.Insert(it.Key());
  }
  removeSet.Difference(reachableNodes);

  // Remove nodes.
  for (const nsUuid& guid : removeSet)
  {
    RemoveNode(guid);
  }
}

void nsAbstractObjectGraph::ModifyNodeViaNativeCounterpart(nsAbstractObjectNode* pRootNode, nsDelegate<void(void*, const nsRTTI*)> callback)
{
  NS_ASSERT_DEV(pRootNode->GetOwner() == this, "Node must be from this graph.");

  // Clone sub graph
  nsAbstractObjectGraph origGraph;
  nsAbstractObjectNode* pOrigRootNode = nullptr;
  {
    pOrigRootNode = Clone(origGraph, pRootNode);
  }

  // Create native object
  nsRttiConverterContext context;
  nsRttiConverterReader convRead(&origGraph, &context);
  void* pNativeRoot = convRead.CreateObjectFromNode(pOrigRootNode);
  const nsRTTI* pType = nsRTTI::FindTypeByName(pOrigRootNode->GetType());
  NS_SCOPE_EXIT(pType->GetAllocator()->Deallocate(pNativeRoot););

  // Make changes to native object
  if (callback.IsValid())
  {
    callback(pNativeRoot, pType);
  }

  // Create native object graph
  nsAbstractObjectGraph graph;
  {
    // The nsApplyNativePropertyChangesContext takes care of generating guids for native pointers that match those
    // of the object manager.
    nsApplyNativePropertyChangesContext nativeChangesContext(context, origGraph);
    nsRttiConverterWriter rttiConverter(&graph, &nativeChangesContext, true, true);
    nativeChangesContext.RegisterObject(pOrigRootNode->GetGuid(), pType, pNativeRoot);
    rttiConverter.AddObjectToGraph(pType, pNativeRoot, "Object");
  }

  // Create diff from native to cloned sub-graph and then apply the diff to the original graph.
  nsDeque<nsAbstractGraphDiffOperation> diffResult;
  graph.CreateDiffWithBaseGraph(origGraph, diffResult);

  ApplyDiff(diffResult);
}

nsAbstractObjectNode* nsAbstractObjectGraph::CopyNodeIntoGraph(const nsAbstractObjectNode* pNode)
{
  auto pNewNode = AddNode(pNode->GetGuid(), pNode->GetType(), pNode->GetTypeVersion(), pNode->GetNodeName());

  for (const auto& props : pNode->GetProperties())
  {
    pNewNode->AddProperty(props.m_sPropertyName, props.m_Value);
  }

  return pNewNode;
}

nsAbstractObjectNode* nsAbstractObjectGraph::CopyNodeIntoGraph(const nsAbstractObjectNode* pNode, FilterFunction& ref_filter)
{
  auto pNewNode = AddNode(pNode->GetGuid(), pNode->GetType(), pNode->GetTypeVersion(), pNode->GetNodeName());

  if (ref_filter.IsValid())
  {
    for (const auto& props : pNode->GetProperties())
    {
      if (!ref_filter(pNode, &props))
        continue;

      pNewNode->AddProperty(props.m_sPropertyName, props.m_Value);
    }
  }
  else
  {
    for (const auto& props : pNode->GetProperties())
      pNewNode->AddProperty(props.m_sPropertyName, props.m_Value);
  }

  return pNewNode;
}

void nsAbstractObjectGraph::CreateDiffWithBaseGraph(const nsAbstractObjectGraph& base, nsDeque<nsAbstractGraphDiffOperation>& out_diffResult) const
{
  out_diffResult.Clear();

  // check whether any nodes have been deleted
  {
    for (auto itNodeBase = base.GetAllNodes().GetIterator(); itNodeBase.IsValid(); ++itNodeBase)
    {
      if (GetNode(itNodeBase.Key()) == nullptr)
      {
        // does not exist in this graph -> has been deleted from base
        nsAbstractGraphDiffOperation op;
        op.m_Node = itNodeBase.Key();
        op.m_Operation = nsAbstractGraphDiffOperation::Op::NodeRemoved;
        op.m_sProperty = itNodeBase.Value()->m_sType;
        op.m_Value = itNodeBase.Value()->m_sNodeName;

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
        nsAbstractGraphDiffOperation op;
        op.m_Node = itNodeThis.Key();
        op.m_Operation = nsAbstractGraphDiffOperation::Op::NodeAdded;
        op.m_sProperty = itNodeThis.Value()->m_sType;
        op.m_Value = itNodeThis.Value()->m_sNodeName;

        out_diffResult.PushBack(op);

        // set all properties
        for (const auto& prop : itNodeThis.Value()->GetProperties())
        {
          op.m_Operation = nsAbstractGraphDiffOperation::Op::PropertyChanged;
          op.m_sProperty = prop.m_sPropertyName;
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

      for (const nsAbstractObjectNode::Property& prop : itNodeThis.Value()->GetProperties())
      {
        bool bDifferent = true;

        for (const nsAbstractObjectNode::Property& baseProp : pBaseNode->GetProperties())
        {
          if (baseProp.m_sPropertyName == prop.m_sPropertyName)
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
          nsAbstractGraphDiffOperation op;
          op.m_Node = itNodeThis.Key();
          op.m_Operation = nsAbstractGraphDiffOperation::Op::PropertyChanged;
          op.m_sProperty = prop.m_sPropertyName;
          op.m_Value = prop.m_Value;

          out_diffResult.PushBack(op);
        }
      }
    }
  }
}


void nsAbstractObjectGraph::ApplyDiff(nsDeque<nsAbstractGraphDiffOperation>& ref_diff)
{
  for (const auto& op : ref_diff)
  {
    switch (op.m_Operation)
    {
      case nsAbstractGraphDiffOperation::Op::NodeAdded:
      {
        AddNode(op.m_Node, op.m_sProperty, op.m_uiTypeVersion, op.m_Value.Get<nsString>());
      }
      break;

      case nsAbstractGraphDiffOperation::Op::NodeRemoved:
      {
        RemoveNode(op.m_Node);
      }
      break;

      case nsAbstractGraphDiffOperation::Op::PropertyChanged:
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


void nsAbstractObjectGraph::MergeDiffs(const nsDeque<nsAbstractGraphDiffOperation>& lhs, const nsDeque<nsAbstractGraphDiffOperation>& rhs, nsDeque<nsAbstractGraphDiffOperation>& ref_out) const
{
  struct Prop
  {
    Prop() = default;
    Prop(nsUuid node, nsStringView sProperty)
      : m_Node(node)
      , m_sProperty(sProperty)
    {
    }
    nsUuid m_Node;
    nsStringView m_sProperty;

    bool operator<(const Prop& rhs) const
    {
      if (m_Node == rhs.m_Node)
        return m_sProperty < rhs.m_sProperty;

      return m_Node < rhs.m_Node;
    }

    bool operator==(const Prop& rhs) const { return m_Node == rhs.m_Node && m_sProperty == rhs.m_sProperty; }
  };

  nsMap<Prop, nsHybridArray<const nsAbstractGraphDiffOperation*, 2>> propChanges;
  nsSet<nsUuid> removed;
  nsMap<nsUuid, nsUInt32> added;
  for (const nsAbstractGraphDiffOperation& op : lhs)
  {
    if (op.m_Operation == nsAbstractGraphDiffOperation::Op::NodeRemoved)
    {
      removed.Insert(op.m_Node);
      ref_out.PushBack(op);
    }
    else if (op.m_Operation == nsAbstractGraphDiffOperation::Op::NodeAdded)
    {
      added[op.m_Node] = ref_out.GetCount();
      ref_out.PushBack(op);
    }
    else if (op.m_Operation == nsAbstractGraphDiffOperation::Op::PropertyChanged)
    {
      auto it = propChanges.FindOrAdd(Prop(op.m_Node, op.m_sProperty));
      it.Value().PushBack(&op);
    }
  }
  for (const nsAbstractGraphDiffOperation& op : rhs)
  {
    if (op.m_Operation == nsAbstractGraphDiffOperation::Op::NodeRemoved)
    {
      if (!removed.Contains(op.m_Node))
        ref_out.PushBack(op);
    }
    else if (op.m_Operation == nsAbstractGraphDiffOperation::Op::NodeAdded)
    {
      if (added.Contains(op.m_Node))
      {
        nsAbstractGraphDiffOperation& leftOp = ref_out[added[op.m_Node]];
        leftOp.m_sProperty = op.m_sProperty; // Take type from rhs.
      }
      else
      {
        ref_out.PushBack(op);
      }
    }
    else if (op.m_Operation == nsAbstractGraphDiffOperation::Op::PropertyChanged)
    {
      auto it = propChanges.FindOrAdd(Prop(op.m_Node, op.m_sProperty));
      it.Value().PushBack(&op);
    }
  }

  for (auto it = propChanges.GetIterator(); it.IsValid(); ++it)
  {
    const Prop& key = it.Key();
    const nsHybridArray<const nsAbstractGraphDiffOperation*, 2>& value = it.Value();

    if (value.GetCount() == 1)
    {
      ref_out.PushBack(*value[0]);
    }
    else
    {
      const nsAbstractGraphDiffOperation& leftProp = *value[0];
      const nsAbstractGraphDiffOperation& rightProp = *value[1];

      if (leftProp.m_Value.GetType() == nsVariantType::VariantArray && rightProp.m_Value.GetType() == nsVariantType::VariantArray)
      {
        const nsVariantArray& leftArray = leftProp.m_Value.Get<nsVariantArray>();
        const nsVariantArray& rightArray = rightProp.m_Value.Get<nsVariantArray>();

        const nsAbstractObjectNode* pNode = GetNode(key.m_Node);
        if (pNode)
        {
          nsStringBuilder sTemp(key.m_sProperty);
          const nsAbstractObjectNode::Property* pProperty = pNode->FindProperty(sTemp);
          if (pProperty && pProperty->m_Value.GetType() == nsVariantType::VariantArray)
          {
            // Do 3-way array merge
            const nsVariantArray& baseArray = pProperty->m_Value.Get<nsVariantArray>();
            nsVariantArray res;
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

void nsAbstractObjectGraph::RemapVariant(nsVariant& value, const nsHashTable<nsUuid, nsUuid>& guidMap)
{
  nsStringBuilder tmp;

  // if the property is a guid, we check if we need to remap it
  if (value.IsA<nsUuid>())
  {
    const nsUuid& guid = value.Get<nsUuid>();

    // if we find the guid in our map, replace it by the new guid
    if (auto* found = guidMap.GetValue(guid))
    {
      value = *found;
    }
  }
  else if (value.IsA<nsString>() && nsConversionUtils::IsStringUuid(value.Get<nsString>()))
  {
    const nsUuid guid = nsConversionUtils::ConvertStringToUuid(value.Get<nsString>());

    // if we find the guid in our map, replace it by the new guid
    if (auto* found = guidMap.GetValue(guid))
    {
      value = nsConversionUtils::ToString(*found, tmp).GetData();
    }
  }
  // Arrays may be of uuids
  else if (value.IsA<nsVariantArray>())
  {
    const nsVariantArray& values = value.Get<nsVariantArray>();
    bool bNeedToRemap = false;
    for (auto& subValue : values)
    {
      if (subValue.IsA<nsUuid>() && guidMap.Contains(subValue.Get<nsUuid>()))
      {
        bNeedToRemap = true;
        break;
      }
      else if (subValue.IsA<nsString>() && nsConversionUtils::IsStringUuid(subValue.Get<nsString>()))
      {
        bNeedToRemap = true;
        break;
      }
      else if (subValue.IsA<nsVariantArray>())
      {
        bNeedToRemap = true;
        break;
      }
    }

    if (bNeedToRemap)
    {
      nsVariantArray newValues = values;
      for (auto& subValue : newValues)
      {
        RemapVariant(subValue, guidMap);
      }
      value = newValues;
    }
  }
  // Maps may be of uuids
  else if (value.IsA<nsVariantDictionary>())
  {
    const nsVariantDictionary& values = value.Get<nsVariantDictionary>();
    bool bNeedToRemap = false;
    for (auto it = values.GetIterator(); it.IsValid(); ++it)
    {
      const nsVariant& subValue = it.Value();

      if (subValue.IsA<nsUuid>() && guidMap.Contains(subValue.Get<nsUuid>()))
      {
        bNeedToRemap = true;
        break;
      }
      else if (subValue.IsA<nsString>() && nsConversionUtils::IsStringUuid(subValue.Get<nsString>()))
      {
        bNeedToRemap = true;
        break;
      }
    }

    if (bNeedToRemap)
    {
      nsVariantDictionary newValues = values;
      for (auto it = newValues.GetIterator(); it.IsValid(); ++it)
      {
        RemapVariant(it.Value(), guidMap);
      }
      value = newValues;
    }
  }
}

void nsAbstractObjectGraph::MergeArrays(const nsDynamicArray<nsVariant>& baseArray, const nsDynamicArray<nsVariant>& leftArray, const nsDynamicArray<nsVariant>& rightArray, nsDynamicArray<nsVariant>& out) const
{
  // Find element type.
  nsVariantType::Enum type = nsVariantType::Invalid;
  if (!baseArray.IsEmpty())
    type = baseArray[0].GetType();
  if (type != nsVariantType::Invalid && !leftArray.IsEmpty())
    type = leftArray[0].GetType();
  if (type != nsVariantType::Invalid && !rightArray.IsEmpty())
    type = rightArray[0].GetType();

  if (type == nsVariantType::Invalid)
    return;

  // For now, assume non-uuid types are arrays, uuids are sets.
  if (type != nsVariantType::Uuid)
  {
    // Any size changes?
    nsUInt32 uiSize = baseArray.GetCount();
    if (leftArray.GetCount() != baseArray.GetCount())
      uiSize = leftArray.GetCount();
    if (rightArray.GetCount() != baseArray.GetCount())
      uiSize = rightArray.GetCount();

    out.SetCount(uiSize);
    for (nsUInt32 i = 0; i < uiSize; i++)
    {
      if (i < baseArray.GetCount())
        out[i] = baseArray[i];
    }

    nsUInt32 uiCountLeft = nsMath::Min(uiSize, leftArray.GetCount());
    for (nsUInt32 i = 0; i < uiCountLeft; i++)
    {
      if (leftArray[i] != baseArray[i])
        out[i] = leftArray[i];
    }

    nsUInt32 uiCountRight = nsMath::Min(uiSize, rightArray.GetCount());
    for (nsUInt32 i = 0; i < uiCountRight; i++)
    {
      if (rightArray[i] != baseArray[i])
        out[i] = rightArray[i];
    }
    return;
  }

  // Move distance is NP-complete so try greedy algorithm
  struct Element
  {
    Element(const nsVariant* pValue = nullptr, nsInt32 iBaseIndex = -1, nsInt32 iLeftIndex = -1, nsInt32 iRightIndex = -1)
      : m_pValue(pValue)
      , m_iBaseIndex(iBaseIndex)
      , m_iLeftIndex(iLeftIndex)
      , m_iRightIndex(iRightIndex)
      , m_fIndex(nsMath::MaxValue<float>())
    {
    }
    bool IsDeleted() const { return m_iBaseIndex != -1 && (m_iLeftIndex == -1 || m_iRightIndex == -1); }
    bool operator<(const Element& rhs) const { return m_fIndex < rhs.m_fIndex; }

    const nsVariant* m_pValue;
    nsInt32 m_iBaseIndex;
    nsInt32 m_iLeftIndex;
    nsInt32 m_iRightIndex;
    float m_fIndex;
  };
  nsDynamicArray<Element> baseOrder;
  baseOrder.Reserve(leftArray.GetCount() + rightArray.GetCount());

  // First, add up all unique elements and their position in each array.
  for (nsInt32 i = 0; i < (nsInt32)baseArray.GetCount(); i++)
  {
    baseOrder.PushBack(Element(&baseArray[i], i));
    baseOrder.PeekBack().m_fIndex = (float)i;
  }

  nsDynamicArray<nsInt32> leftOrder;
  leftOrder.SetCountUninitialized(leftArray.GetCount());
  for (nsInt32 i = 0; i < (nsInt32)leftArray.GetCount(); i++)
  {
    const nsVariant& val = leftArray[i];
    bool bFound = false;
    for (nsInt32 j = 0; j < (nsInt32)baseOrder.GetCount(); j++)
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
      leftOrder[i] = (nsInt32)baseOrder.GetCount();
      baseOrder.PushBack(Element(&leftArray[i], -1, i));
    }
  }

  nsDynamicArray<nsInt32> rightOrder;
  rightOrder.SetCountUninitialized(rightArray.GetCount());
  for (nsInt32 i = 0; i < (nsInt32)rightArray.GetCount(); i++)
  {
    const nsVariant& val = rightArray[i];
    bool bFound = false;
    for (nsInt32 j = 0; j < (nsInt32)baseOrder.GetCount(); j++)
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
      rightOrder[i] = (nsInt32)baseOrder.GetCount();
      baseOrder.PushBack(Element(&rightArray[i], -1, -1, i));
    }
  }

  // Re-order greedy
  float fLastElement = -0.5f;
  for (nsInt32 i = 0; i < (nsInt32)leftOrder.GetCount(); i++)
  {
    Element& currentElem = baseOrder[leftOrder[i]];
    if (currentElem.IsDeleted())
      continue;

    float fLowestSubsequent = nsMath::MaxValue<float>();
    for (nsInt32 j = i + 1; j < (nsInt32)leftOrder.GetCount(); j++)
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
  for (nsInt32 i = 0; i < (nsInt32)rightOrder.GetCount(); i++)
  {
    Element& currentElem = baseOrder[rightOrder[i]];
    if (currentElem.IsDeleted())
      continue;

    float fLowestSubsequent = nsMath::MaxValue<float>();
    for (nsInt32 j = i + 1; j < (nsInt32)rightOrder.GetCount(); j++)
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

NS_STATICLINK_FILE(Foundation, Foundation_Serialization_Implementation_AbstractObjectGraph);
