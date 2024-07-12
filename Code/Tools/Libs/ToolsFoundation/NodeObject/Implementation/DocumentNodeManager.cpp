/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Serialization/RttiConverter.h>
#include <ToolsFoundation/Command/NodeCommands.h>
#include <ToolsFoundation/CommandHistory/CommandHistory.h>
#include <ToolsFoundation/Document/Document.h>
#include <ToolsFoundation/NodeObject/DocumentNodeManager.h>
#include <ToolsFoundation/Serialization/DocumentObjectConverter.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsPin, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

////////////////////////////////////////////////////////////////////////
// nsDocumentNodeManager Internal
////////////////////////////////////////////////////////////////////////

struct DocumentNodeManager_NodeMetaData
{
  nsVec2 m_Pos = nsVec2::MakeZero();
};
NS_DECLARE_REFLECTABLE_TYPE(NS_NO_LINKAGE, DocumentNodeManager_NodeMetaData);

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(DocumentNodeManager_NodeMetaData, nsNoBase, 1, nsRTTIDefaultAllocator<DocumentNodeManager_NodeMetaData>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Node::Pos", m_Pos),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

struct DocumentNodeManager_ConnectionMetaData
{
  nsUuid m_Source;
  nsUuid m_Target;
  nsString m_SourcePin;
  nsString m_TargetPin;
};
NS_DECLARE_REFLECTABLE_TYPE(NS_NO_LINKAGE, DocumentNodeManager_ConnectionMetaData);

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(DocumentNodeManager_ConnectionMetaData, nsNoBase, 1, nsRTTIDefaultAllocator<DocumentNodeManager_ConnectionMetaData>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Connection::Source", m_Source),
    NS_MEMBER_PROPERTY("Connection::Target", m_Target),
    NS_MEMBER_PROPERTY("Connection::SourcePin", m_SourcePin),    
    NS_MEMBER_PROPERTY("Connection::TargetPin", m_TargetPin),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

class DocumentNodeManager_DefaultConnection : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(DocumentNodeManager_DefaultConnection, nsReflectedClass);
};

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(DocumentNodeManager_DefaultConnection, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

////////////////////////////////////////////////////////////////////////
// nsDocumentNodeManager
////////////////////////////////////////////////////////////////////////

nsDocumentNodeManager::nsDocumentNodeManager()
{
  m_ObjectEvents.AddEventHandler(nsMakeDelegate(&nsDocumentNodeManager::ObjectHandler, this));
  m_StructureEvents.AddEventHandler(nsMakeDelegate(&nsDocumentNodeManager::StructureEventHandler, this));
  m_PropertyEvents.AddEventHandler(nsMakeDelegate(&nsDocumentNodeManager::PropertyEventsHandler, this));
}

nsDocumentNodeManager::~nsDocumentNodeManager()
{
  m_ObjectEvents.RemoveEventHandler(nsMakeDelegate(&nsDocumentNodeManager::ObjectHandler, this));
  m_StructureEvents.RemoveEventHandler(nsMakeDelegate(&nsDocumentNodeManager::StructureEventHandler, this));
  m_PropertyEvents.RemoveEventHandler(nsMakeDelegate(&nsDocumentNodeManager::PropertyEventsHandler, this));
}

const nsRTTI* nsDocumentNodeManager::GetConnectionType() const
{
  return nsGetStaticRTTI<DocumentNodeManager_DefaultConnection>();
}

nsVec2 nsDocumentNodeManager::GetNodePos(const nsDocumentObject* pObject) const
{
  NS_ASSERT_DEV(pObject != nullptr, "Invalid input!");
  auto it = m_ObjectToNode.Find(pObject->GetGuid());
  NS_ASSERT_DEV(it.IsValid(), "Can't get pos of objects that aren't nodes!");
  return it.Value().m_vPos;
}

const nsConnection& nsDocumentNodeManager::GetConnection(const nsDocumentObject* pObject) const
{
  NS_ASSERT_DEV(pObject != nullptr, "Invalid input!");
  auto it = m_ObjectToConnection.Find(pObject->GetGuid());
  NS_ASSERT_DEV(it.IsValid(), "Can't get connection for objects that aren't connections!");
  return *it.Value();
}

const nsPin* nsDocumentNodeManager::GetInputPinByName(const nsDocumentObject* pObject, nsStringView sName) const
{
  NS_ASSERT_DEV(pObject != nullptr, "Invalid input!");
  auto it = m_ObjectToNode.Find(pObject->GetGuid());
  NS_ASSERT_DEV(it.IsValid(), "Can't get input pins of objects that aren't nodes!");
  for (auto& pPin : it.Value().m_Inputs)
  {
    if (pPin->GetName() == sName)
      return pPin.Borrow();
  }
  return nullptr;
}

const nsPin* nsDocumentNodeManager::GetOutputPinByName(const nsDocumentObject* pObject, nsStringView sName) const
{
  NS_ASSERT_DEV(pObject != nullptr, "Invalid input!");
  auto it = m_ObjectToNode.Find(pObject->GetGuid());
  NS_ASSERT_DEV(it.IsValid(), "Can't get input pins of objects that aren't nodes!");
  for (auto& pPin : it.Value().m_Outputs)
  {
    if (pPin->GetName() == sName)
      return pPin.Borrow();
  }
  return nullptr;
}

nsArrayPtr<const nsUniquePtr<const nsPin>> nsDocumentNodeManager::GetInputPins(const nsDocumentObject* pObject) const
{
  NS_ASSERT_DEV(pObject != nullptr, "Invalid input!");
  auto it = m_ObjectToNode.Find(pObject->GetGuid());
  NS_ASSERT_DEV(it.IsValid(), "Can't get input pins of objects that aren't nodes!");
  return nsMakeArrayPtr((nsUniquePtr<const nsPin>*)it.Value().m_Inputs.GetData(), it.Value().m_Inputs.GetCount());
}

nsArrayPtr<const nsUniquePtr<const nsPin>> nsDocumentNodeManager::GetOutputPins(const nsDocumentObject* pObject) const
{
  NS_ASSERT_DEV(pObject != nullptr, "Invalid input!");
  auto it = m_ObjectToNode.Find(pObject->GetGuid());
  NS_ASSERT_DEV(it.IsValid(), "Can't get input pins of objects that aren't nodes!");
  return nsMakeArrayPtr((nsUniquePtr<const nsPin>*)it.Value().m_Outputs.GetData(), it.Value().m_Outputs.GetCount());
}

bool nsDocumentNodeManager::IsNode(const nsDocumentObject* pObject) const
{
  NS_ASSERT_DEV(pObject != nullptr, "Invalid input!");
  if (pObject == nullptr)
    return false;
  if (pObject == GetRootObject())
    return false;

  return InternalIsNode(pObject);
}

bool nsDocumentNodeManager::IsConnection(const nsDocumentObject* pObject) const
{
  NS_ASSERT_DEV(pObject != nullptr, "Invalid input!");
  if (pObject == nullptr)
    return false;
  if (pObject == GetRootObject())
    return false;

  return InternalIsConnection(pObject);
}

bool nsDocumentNodeManager::IsDynamicPinProperty(const nsDocumentObject* pObject, const nsAbstractProperty* pProp) const
{
  if (IsNode(pObject) == false)
    return false;

  if (pProp == nullptr)
    return false;

  return InternalIsDynamicPinProperty(pObject, pProp);
}

nsArrayPtr<const nsConnection* const> nsDocumentNodeManager::GetConnections(const nsPin& pin) const
{
  auto it = m_Connections.Find(&pin);
  if (it.IsValid())
  {
    return it.Value();
  }

  return nsArrayPtr<const nsConnection* const>();
}

bool nsDocumentNodeManager::HasConnections(const nsPin& pin) const
{
  auto it = m_Connections.Find(&pin);
  return it.IsValid() && it.Value().IsEmpty() == false;
}

bool nsDocumentNodeManager::IsConnected(const nsPin& source, const nsPin& target) const
{
  auto it = m_Connections.Find(&source);
  if (it.IsValid())
  {
    for (auto pConnection : it.Value())
    {
      if (&pConnection->GetTargetPin() == &target)
        return true;
    }
  }

  return false;
}

nsStatus nsDocumentNodeManager::CanConnect(const nsRTTI* pObjectType, const nsPin& source, const nsPin& target, CanConnectResult& out_result) const
{
  out_result = CanConnectResult::ConnectNever;

  if (pObjectType == nullptr || pObjectType->IsDerivedFrom(GetConnectionType()) == false)
    return nsStatus("Invalid connection object type");

  if (source.m_Type != nsPin::Type::Output)
    return nsStatus("Source pin is not an output pin.");
  if (target.m_Type != nsPin::Type::Input)
    return nsStatus("Target pin is not an input pin.");

  if (source.m_pParent == target.m_pParent)
    return nsStatus("Nodes cannot be connect with themselves.");

  if (IsConnected(source, target))
    return nsStatus("Pins already connected.");

  return InternalCanConnect(source, target, out_result);
}

nsStatus nsDocumentNodeManager::CanDisconnect(const nsConnection* pConnection) const
{
  if (pConnection == nullptr)
    return nsStatus("Invalid connection");

  return InternalCanDisconnect(pConnection->GetSourcePin(), pConnection->GetTargetPin());
}

nsStatus nsDocumentNodeManager::CanDisconnect(const nsDocumentObject* pObject) const
{
  if (!IsConnection(pObject))
    return nsStatus("Invalid connection object");

  const nsConnection& connection = GetConnection(pObject);
  return InternalCanDisconnect(connection.GetSourcePin(), connection.GetTargetPin());
}

nsStatus nsDocumentNodeManager::CanMoveNode(const nsDocumentObject* pObject, const nsVec2& vPos) const
{
  NS_ASSERT_DEV(pObject != nullptr, "Invalid input!");
  if (!IsNode(pObject))
    return nsStatus("The given object is not a node!");

  return InternalCanMoveNode(pObject, vPos);
}

void nsDocumentNodeManager::Connect(const nsDocumentObject* pObject, const nsPin& source, const nsPin& target)
{
  nsDocumentNodeManager::CanConnectResult res = CanConnectResult::ConnectNever;
  NS_IGNORE_UNUSED(res);
  NS_ASSERT_DEBUG(CanConnect(pObject->GetType(), source, target, res).m_Result.Succeeded(), "Connect: Sanity check failed!");

  auto pConnection = NS_DEFAULT_NEW(nsConnection, source, target, pObject);
  m_ObjectToConnection.Insert(pObject->GetGuid(), pConnection);

  m_Connections[&source].PushBack(pConnection);
  m_Connections[&target].PushBack(pConnection);

  {
    nsDocumentNodeManagerEvent e(nsDocumentNodeManagerEvent::Type::AfterPinsConnected, pObject);
    m_NodeEvents.Broadcast(e);
  }
}

void nsDocumentNodeManager::Disconnect(const nsDocumentObject* pObject)
{
  auto it = m_ObjectToConnection.Find(pObject->GetGuid());
  NS_ASSERT_DEBUG(it.IsValid(), "Sanity check failed!");
  NS_ASSERT_DEBUG(CanDisconnect(pObject).m_Result.Succeeded(), "Disconnect: Sanity check failed!");

  {
    nsDocumentNodeManagerEvent e(nsDocumentNodeManagerEvent::Type::BeforePinsDisonnected, pObject);
    m_NodeEvents.Broadcast(e);
  }

  auto& pConnection = it.Value();
  const nsPin& source = pConnection->GetSourcePin();
  const nsPin& target = pConnection->GetTargetPin();
  m_Connections[&source].RemoveAndCopy(pConnection.Borrow());
  m_Connections[&target].RemoveAndCopy(pConnection.Borrow());

  m_ObjectToConnection.Remove(it);
}

void nsDocumentNodeManager::MoveNode(const nsDocumentObject* pObject, const nsVec2& vPos)
{
  NS_ASSERT_DEBUG(CanMoveNode(pObject, vPos).m_Result.Succeeded(), "MoveNode: Sanity check failed!");

  auto it = m_ObjectToNode.Find(pObject->GetGuid());
  NS_ASSERT_DEBUG(it.IsValid(), "Moveable node does not exist, CanMoveNode impl invalid!");
  it.Value().m_vPos = vPos;

  nsDocumentNodeManagerEvent e(nsDocumentNodeManagerEvent::Type::NodeMoved, pObject);
  m_NodeEvents.Broadcast(e);
}

void nsDocumentNodeManager::AttachMetaDataBeforeSaving(nsAbstractObjectGraph& ref_graph) const
{
  auto pNodeMetaDataType = nsGetStaticRTTI<DocumentNodeManager_NodeMetaData>();
  auto pConnectionMetaDataType = nsGetStaticRTTI<DocumentNodeManager_ConnectionMetaData>();

  nsRttiConverterContext context;
  nsRttiConverterWriter rttiConverter(&ref_graph, &context, true, true);

  for (auto it = ref_graph.GetAllNodes().GetIterator(); it.IsValid(); ++it)
  {
    auto* pAbstractObject = it.Value();
    const nsUuid& guid = pAbstractObject->GetGuid();

    {
      auto it2 = m_ObjectToNode.Find(guid);
      if (it2.IsValid())
      {
        const NodeInternal& node = it2.Value();

        DocumentNodeManager_NodeMetaData nodeMetaData;
        nodeMetaData.m_Pos = node.m_vPos;
        rttiConverter.AddProperties(pAbstractObject, pNodeMetaDataType, &nodeMetaData);
      }
    }

    {
      auto it2 = m_ObjectToConnection.Find(guid);
      if (it2.IsValid())
      {
        const nsConnection& connection = *it2.Value();
        const nsPin& sourcePin = connection.GetSourcePin();
        const nsPin& targetPin = connection.GetTargetPin();

        DocumentNodeManager_ConnectionMetaData connectionMetaData;
        connectionMetaData.m_Source = sourcePin.GetParent()->GetGuid();
        connectionMetaData.m_Target = targetPin.GetParent()->GetGuid();
        connectionMetaData.m_SourcePin = sourcePin.GetName();
        connectionMetaData.m_TargetPin = targetPin.GetName();
        rttiConverter.AddProperties(pAbstractObject, pConnectionMetaDataType, &connectionMetaData);
      }
    }
  }
}

void nsDocumentNodeManager::RestoreMetaDataAfterLoading(const nsAbstractObjectGraph& graph, bool bUndoable)
{
  nsCommandHistory* history = GetDocument()->GetCommandHistory();

  auto pNodeMetaDataType = nsGetStaticRTTI<DocumentNodeManager_NodeMetaData>();
  auto pConnectionMetaDataType = nsGetStaticRTTI<DocumentNodeManager_ConnectionMetaData>();

  nsRttiConverterContext context;
  nsRttiConverterReader rttiConverter(&graph, &context);

  for (auto it : graph.GetAllNodes())
  {
    auto pAbstractObject = it.Value();
    nsDocumentObject* pObject = GetObject(pAbstractObject->GetGuid());
    if (pObject == nullptr)
      continue;

    if (IsNode(pObject))
    {
      DocumentNodeManager_NodeMetaData nodeMetaData;
      rttiConverter.ApplyPropertiesToObject(pAbstractObject, pNodeMetaDataType, &nodeMetaData);

      if (CanMoveNode(pObject, nodeMetaData.m_Pos).m_Result.Succeeded())
      {
        if (bUndoable)
        {
          nsMoveNodeCommand move;
          move.m_Object = pObject->GetGuid();
          move.m_NewPos = nodeMetaData.m_Pos;
          history->AddCommand(move).LogFailure();
        }
        else
        {
          MoveNode(pObject, nodeMetaData.m_Pos);
        }
      }

      // Backwards compatibility to old file format
      if (auto pOldConnections = pAbstractObject->FindProperty("Node::Connections"))
      {
        NS_ASSERT_DEV(bUndoable == false, "Undo not supported for old file format");
        RestoreOldMetaDataAfterLoading(graph, *pOldConnections, pObject);
      }
    }
    else if (IsConnection(pObject))
    {
      DocumentNodeManager_ConnectionMetaData connectionMetaData;
      rttiConverter.ApplyPropertiesToObject(pAbstractObject, pConnectionMetaDataType, &connectionMetaData);

      nsDocumentObject* pSource = GetObject(connectionMetaData.m_Source);
      nsDocumentObject* pTarget = GetObject(connectionMetaData.m_Target);
      if (pSource == nullptr || pTarget == nullptr)
      {
        RemoveObject(pObject);
        DestroyObject(pObject);
        continue;
      }

      const nsPin* pSourcePin = GetOutputPinByName(pSource, connectionMetaData.m_SourcePin);
      if (pSourcePin == nullptr)
      {
        nsLog::Error("Unknown output pin '{}' on '{}'. The connection has been removed.", connectionMetaData.m_SourcePin, pSource->GetType()->GetTypeName());
        RemoveObject(pObject);
        DestroyObject(pObject);
        continue;
      }

      const nsPin* pTargetPin = GetInputPinByName(pTarget, connectionMetaData.m_TargetPin);
      if (pTargetPin == nullptr)
      {
        nsLog::Error("Unknown input pin '{}' on '{}'. The connection has been removed.", connectionMetaData.m_TargetPin, pTarget->GetType()->GetTypeName());
        RemoveObject(pObject);
        DestroyObject(pObject);
        continue;
      }

      nsDocumentNodeManager::CanConnectResult res;
      if (CanConnect(pObject->GetType(), *pSourcePin, *pTargetPin, res).m_Result.Failed())
      {
        RemoveObject(pObject);
        DestroyObject(pObject);
        continue;
      }

      if (bUndoable)
      {
        nsConnectNodePinsCommand cmd;
        cmd.m_ConnectionObject = pObject->GetGuid();
        cmd.m_ObjectSource = connectionMetaData.m_Source;
        cmd.m_ObjectTarget = connectionMetaData.m_Target;
        cmd.m_sSourcePin = connectionMetaData.m_SourcePin;
        cmd.m_sTargetPin = connectionMetaData.m_TargetPin;
        history->AddCommand(cmd).LogFailure();
      }
      else
      {
        Connect(pObject, *pSourcePin, *pTargetPin);
      }
    }
  }
}

void nsDocumentNodeManager::GetMetaDataHash(const nsDocumentObject* pObject, nsUInt64& inout_uiHash) const
{
  if (IsNode(pObject))
  {
    // The node position is not hashed here since the hash is only used for asset transform
    // and for that the node position is irrelevant.
  }
  else if (IsConnection(pObject))
  {
    const nsConnection& connection = GetConnection(pObject);
    const nsPin& sourcePin = connection.GetSourcePin();
    const nsPin& targetPin = connection.GetTargetPin();

    inout_uiHash = nsHashingUtils::xxHash64(&sourcePin.GetParent()->GetGuid(), sizeof(nsUuid), inout_uiHash);
    inout_uiHash = nsHashingUtils::xxHash64(&targetPin.GetParent()->GetGuid(), sizeof(nsUuid), inout_uiHash);
    inout_uiHash = nsHashingUtils::xxHash64String(sourcePin.GetName(), inout_uiHash);
    inout_uiHash = nsHashingUtils::xxHash64String(targetPin.GetName(), inout_uiHash);
  }
}

bool nsDocumentNodeManager::CopySelectedObjects(nsAbstractObjectGraph& out_objectGraph) const
{
  const auto& selection = GetDocument()->GetSelectionManager()->GetSelection();

  if (selection.IsEmpty())
    return false;

  nsDocumentObjectConverterWriter writer(&out_objectGraph, this);

  nsHashSet<const nsDocumentObject*> copiedNodes;
  for (const nsDocumentObject* pObject : selection)
  {
    // Only add nodes here, connections are then collected below to ensure
    // that we always include only valid connections within the copied subgraph no matter if they are selected or not.
    if (IsNode(pObject))
    {
      // objects are required to be named root but this is not enforced or obvious by the interface.
      writer.AddObjectToGraph(pObject, "root");
      copiedNodes.Insert(pObject);
    }
  }

  nsHashSet<const nsDocumentObject*> copiedConnections;
  for (const nsDocumentObject* pNodeObject : selection)
  {
    if (IsNode(pNodeObject) == false)
      continue;

    auto outputs = GetOutputPins(pNodeObject);
    for (auto& pSourcePin : outputs)
    {
      auto connections = GetConnections(*pSourcePin);
      for (const nsConnection* pConnection : connections)
      {
        const nsDocumentObject* pConnectionObject = pConnection->GetParent();

        NS_ASSERT_DEV(pSourcePin == &pConnection->GetSourcePin(), "");
        if (copiedConnections.Contains(pConnectionObject) == false && copiedNodes.Contains(pConnection->GetTargetPin().GetParent()))
        {
          writer.AddObjectToGraph(pConnectionObject, "root");
          copiedConnections.Insert(pConnectionObject);
        }
      }
    }
  }

  AttachMetaDataBeforeSaving(out_objectGraph);

  return true;
}

bool nsDocumentNodeManager::PasteObjects(const nsArrayPtr<nsDocument::PasteInfo>& info, const nsAbstractObjectGraph& objectGraph, const nsVec2& vPickedPosition, bool bAllowPickedPosition)
{
  bool bAddedAll = true;
  nsDeque<const nsDocumentObject*> AddedObjects;

  for (const auto& pi : info)
  {
    // only add nodes that are allowed to be added
    if (CanAdd(pi.m_pObject->GetTypeAccessor().GetType(), nullptr, "Children", pi.m_Index).m_Result.Succeeded())
    {
      AddedObjects.PushBack(pi.m_pObject);
      AddObject(pi.m_pObject, nullptr, "Children", pi.m_Index);
    }
    else
    {
      bAddedAll = false;
    }
  }

  RestoreMetaDataAfterLoading(objectGraph, true);

  if (!AddedObjects.IsEmpty() && bAllowPickedPosition)
  {
    nsCommandHistory* history = GetDocument()->GetCommandHistory();

    nsVec2 vAvgPos(0);
    nsUInt32 nodeCount = 0;
    for (const nsDocumentObject* pObject : AddedObjects)
    {
      if (IsNode(pObject))
      {
        vAvgPos += GetNodePos(pObject);
        ++nodeCount;
      }
    }

    vAvgPos /= (float)nodeCount;
    const nsVec2 vMoveNode = -vAvgPos + vPickedPosition;

    for (const nsDocumentObject* pObject : AddedObjects)
    {
      if (IsNode(pObject))
      {
        nsMoveNodeCommand move;
        move.m_Object = pObject->GetGuid();
        move.m_NewPos = GetNodePos(pObject) + vMoveNode;
        history->AddCommand(move).LogFailure();
      }
    }

    if (!bAddedAll)
    {
      nsLog::Info("[EditorStatus]Not all nodes were allowed to be added to the document");
    }
  }

  GetDocument()->GetSelectionManager()->SetSelection(AddedObjects);
  return true;
}

bool nsDocumentNodeManager::CanReachNode(const nsDocumentObject* pSource, const nsDocumentObject* pTarget, nsSet<const nsDocumentObject*>& Visited) const
{
  if (pSource == pTarget)
    return true;

  if (Visited.Contains(pSource))
    return false;

  Visited.Insert(pSource);

  auto outputs = GetOutputPins(pSource);
  for (auto& pSourcePin : outputs)
  {
    auto connections = GetConnections(*pSourcePin);
    for (const nsConnection* pConnection : connections)
    {
      if (CanReachNode(pConnection->GetTargetPin().GetParent(), pTarget, Visited))
        return true;
    }
  }

  return false;
}


bool nsDocumentNodeManager::WouldConnectionCreateCircle(const nsPin& source, const nsPin& target) const
{
  const nsDocumentObject* pSourceNode = source.GetParent();
  const nsDocumentObject* pTargetNode = target.GetParent();
  nsSet<const nsDocumentObject*> Visited;

  return CanReachNode(pTargetNode, pSourceNode, Visited);
}

void nsDocumentNodeManager::GetDynamicPinNames(const nsDocumentObject* pObject, nsStringView sPropertyName, nsStringView sPinName, nsDynamicArray<nsString>& out_Names) const
{
  out_Names.Clear();

  const nsAbstractProperty* pProp = pObject->GetType()->FindPropertyByName(sPropertyName);
  if (pProp == nullptr)
  {
    nsLog::Warning("Property '{0}' not found in type '{1}'", sPropertyName, pObject->GetType()->GetTypeName());
    return;
  }

  nsStringBuilder sTemp;
  nsVariant value = pObject->GetTypeAccessor().GetValue(sPropertyName);

  if (pProp->GetCategory() == nsPropertyCategory::Member)
  {
    if (value.CanConvertTo<nsUInt32>())
    {
      nsUInt32 uiCount = value.ConvertTo<nsUInt32>();
      for (nsUInt32 i = 0; i < uiCount; ++i)
      {
        sTemp.Format("{}[{}]", sPinName, i);
        out_Names.PushBack(sTemp);
      }
    }
  }
  else if (pProp->GetCategory() == nsPropertyCategory::Array)
  {
    auto pArrayProp = static_cast<const nsAbstractArrayProperty*>(pProp);

    auto& a = value.Get<nsVariantArray>();
    const nsUInt32 uiCount = a.GetCount();

    auto variantType = pArrayProp->GetSpecificType()->GetVariantType();
    if (variantType >= nsVariantType::Int8 && variantType <= nsVariantType::UInt64)
    {
      for (nsUInt32 i = 0; i < uiCount; ++i)
      {
        sTemp.Format("{}", a[i]);
        out_Names.PushBack(sTemp);
      }
    }
    else if (variantType == nsVariantType::String || variantType == nsVariantType::HashedString)
    {
      for (nsUInt32 i = 0; i < uiCount; ++i)
      {
        out_Names.PushBack(a[i].ConvertTo<nsString>());
      }
    }
    else
    {
      for (nsUInt32 i = 0; i < uiCount; ++i)
      {
        sTemp.Format("{}[{}]", sPinName, i);
        out_Names.PushBack(sTemp);
      }
    }
  }
}

bool nsDocumentNodeManager::TryRecreatePins(const nsDocumentObject* pObject)
{
  if (!IsNode(pObject))
    return false;

  auto& nodeInternal = m_ObjectToNode[pObject->GetGuid()];

  for (auto& pPin : nodeInternal.m_Inputs)
  {
    if (HasConnections(*pPin))
      return false;
  }

  for (auto& pPin : nodeInternal.m_Outputs)
  {
    if (HasConnections(*pPin))
      return false;
  }

  {
    nsDocumentNodeManagerEvent e(nsDocumentNodeManagerEvent::Type::BeforePinsChanged, pObject);
    m_NodeEvents.Broadcast(e);
  }

  nodeInternal.m_Inputs.Clear();
  nodeInternal.m_Outputs.Clear();
  InternalCreatePins(pObject, nodeInternal);

  {
    nsDocumentNodeManagerEvent e(nsDocumentNodeManagerEvent::Type::AfterPinsChanged, pObject);
    m_NodeEvents.Broadcast(e);
  }

  return true;
}

bool nsDocumentNodeManager::InternalIsNode(const nsDocumentObject* pObject) const
{
  return true;
}

bool nsDocumentNodeManager::InternalIsConnection(const nsDocumentObject* pObject) const
{
  auto pType = pObject->GetTypeAccessor().GetType();
  return pType->IsDerivedFrom(GetConnectionType());
}

nsStatus nsDocumentNodeManager::InternalCanConnect(const nsPin& source, const nsPin& target, CanConnectResult& out_Result) const
{
  out_Result = CanConnectResult::ConnectNtoN;
  return nsStatus(NS_SUCCESS);
}

void nsDocumentNodeManager::ObjectHandler(const nsDocumentObjectEvent& e)
{
  switch (e.m_EventType)
  {
    case nsDocumentObjectEvent::Type::AfterObjectCreated:
    {
      if (IsNode(e.m_pObject))
      {
        NS_ASSERT_DEBUG(!m_ObjectToNode.Contains(e.m_pObject->GetGuid()), "Sanity check failed!");
        m_ObjectToNode[e.m_pObject->GetGuid()] = NodeInternal();
      }
      else if (IsConnection(e.m_pObject))
      {
        // Nothing to do here: Map entries are created in Connect method.
      }
    }
    break;
    case nsDocumentObjectEvent::Type::BeforeObjectDestroyed:
    {
      if (IsNode(e.m_pObject))
      {
        auto it = m_ObjectToNode.Find(e.m_pObject->GetGuid());
        NS_ASSERT_DEBUG(it.IsValid(), "Sanity check failed!");

        m_ObjectToNode.Remove(it);
      }
      else if (IsConnection(e.m_pObject))
      {
        // Nothing to do here: Map entries are removed in Disconnect method.
      }
    }
    break;
    default:
      NS_ASSERT_NOT_IMPLEMENTED
  }
}

void nsDocumentNodeManager::StructureEventHandler(const nsDocumentObjectStructureEvent& e)
{
  switch (e.m_EventType)
  {
    case nsDocumentObjectStructureEvent::Type::BeforeObjectAdded:
    {
      if (IsNode(e.m_pObject))
      {
        auto& nodeInternal = m_ObjectToNode[e.m_pObject->GetGuid()];
        if (nodeInternal.m_Inputs.IsEmpty() && nodeInternal.m_Outputs.IsEmpty())
        {
          InternalCreatePins(e.m_pObject, nodeInternal);
          // TODO: Sanity check pins (duplicate names etc).
        }

        nsDocumentNodeManagerEvent e2(nsDocumentNodeManagerEvent::Type::BeforeNodeAdded, e.m_pObject);
        m_NodeEvents.Broadcast(e2);
      }
    }
    break;
    case nsDocumentObjectStructureEvent::Type::AfterObjectAdded:
    {
      if (IsNode(e.m_pObject))
      {
        nsDocumentNodeManagerEvent e2(nsDocumentNodeManagerEvent::Type::AfterNodeAdded, e.m_pObject);
        m_NodeEvents.Broadcast(e2);
      }
    }
    break;
    case nsDocumentObjectStructureEvent::Type::BeforeObjectRemoved:
    {
      if (IsNode(e.m_pObject))
      {
        nsDocumentNodeManagerEvent e2(nsDocumentNodeManagerEvent::Type::BeforeNodeRemoved, e.m_pObject);
        m_NodeEvents.Broadcast(e2);
      }
    }
    break;
    case nsDocumentObjectStructureEvent::Type::AfterObjectRemoved:
    {
      if (IsNode(e.m_pObject))
      {
        nsDocumentNodeManagerEvent e2(nsDocumentNodeManagerEvent::Type::AfterNodeRemoved, e.m_pObject);
        m_NodeEvents.Broadcast(e2);
      }
    }
    break;

    default:
      break;
  }
}

void nsDocumentNodeManager::PropertyEventsHandler(const nsDocumentObjectPropertyEvent& e)
{
  if (e.m_pObject == nullptr)
    return;

  const nsAbstractProperty* pProp = e.m_pObject->GetType()->FindPropertyByName(e.m_sProperty);
  if (pProp == nullptr)
    return;

  if (IsDynamicPinProperty(e.m_pObject, pProp))
  {
    TryRecreatePins(e.m_pObject);
  }
}

void nsDocumentNodeManager::RestoreOldMetaDataAfterLoading(const nsAbstractObjectGraph& graph, const nsAbstractObjectNode::Property& connectionsProperty, const nsDocumentObject* pSourceObject)
{
  if (connectionsProperty.m_Value.IsA<nsVariantArray>() == false)
    return;

  const nsVariantArray& array = connectionsProperty.m_Value.Get<nsVariantArray>();
  for (const nsVariant& var : array)
  {
    if (var.IsA<nsUuid>() == false)
      continue;

    auto pOldConnectionAbstractObject = graph.GetNode(var.Get<nsUuid>());
    auto pTargetProperty = pOldConnectionAbstractObject->FindProperty("Target");
    if (pTargetProperty == nullptr || pTargetProperty->m_Value.IsA<nsUuid>() == false)
      continue;

    nsDocumentObject* pTargetObject = GetObject(pTargetProperty->m_Value.Get<nsUuid>());
    if (pTargetObject == nullptr)
      continue;

    auto pSourcePinProperty = pOldConnectionAbstractObject->FindProperty("SourcePin");
    if (pSourcePinProperty == nullptr || pSourcePinProperty->m_Value.IsA<nsString>() == false)
      continue;

    auto pTargetPinProperty = pOldConnectionAbstractObject->FindProperty("TargetPin");
    if (pTargetPinProperty == nullptr || pTargetPinProperty->m_Value.IsA<nsString>() == false)
      continue;

    const nsPin* pSourcePin = GetOutputPinByName(pSourceObject, pSourcePinProperty->m_Value.Get<nsString>());
    const nsPin* pTargetPin = GetInputPinByName(pTargetObject, pTargetPinProperty->m_Value.Get<nsString>());
    if (pSourcePin == nullptr || pTargetPin == nullptr)
      continue;

    const nsRTTI* pConnectionType = GetConnectionType();
    nsDocumentNodeManager::CanConnectResult res;
    if (CanConnect(pConnectionType, *pSourcePin, *pTargetPin, res).m_Result.Succeeded())
    {
      nsDocumentObject* pConnectionObject = CreateObject(pConnectionType, nsUuid::MakeUuid());

      AddObject(pConnectionObject, nullptr, "", -1);

      Connect(pConnectionObject, *pSourcePin, *pTargetPin);
    }
  }
}
