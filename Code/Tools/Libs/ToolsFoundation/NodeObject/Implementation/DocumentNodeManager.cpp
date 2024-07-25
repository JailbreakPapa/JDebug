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

  bool IsValid() const { return m_Source.IsValid() && m_Target.IsValid(); }
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
// nsDocumentObject_ConnectionBase
////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsDocumentObject_ConnectionBase, 1, nsRTTIDefaultAllocator<nsDocumentObject_ConnectionBase>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Source", m_Source)->AddAttributes(new nsHiddenAttribute()),
    NS_MEMBER_PROPERTY("Target", m_Target)->AddAttributes(new nsHiddenAttribute()),
    NS_MEMBER_PROPERTY("SourcePin", m_SourcePin)->AddAttributes(new nsHiddenAttribute()),
    NS_MEMBER_PROPERTY("TargetPin", m_TargetPin)->AddAttributes(new nsHiddenAttribute()),
  }
  NS_END_PROPERTIES;
}
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

void nsDocumentNodeManager::GetNodeCreationTemplates(nsDynamicArray<nsNodeCreationTemplate>& out_templates) const
{
  nsHybridArray<const nsRTTI*, 32> types;
  GetCreateableTypes(types);

  for (auto pType : types)
  {
    auto& nodeTemplate = out_templates.ExpandAndGetRef();
    nodeTemplate.m_pType = pType;
  }
}

const nsRTTI* nsDocumentNodeManager::GetConnectionType() const
{
  return nsGetStaticRTTI<nsDocumentObject_ConnectionBase>();
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

const nsConnection* nsDocumentNodeManager::GetConnectionIfExists(const nsDocumentObject* pObject) const
{
  NS_ASSERT_DEV(pObject != nullptr, "Invalid input!");
  auto it = m_ObjectToConnection.Find(pObject->GetGuid());
  return it.IsValid() ? it.Value().Borrow() : nullptr;
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

  NS_ASSERT_DEBUG(pObject->GetTypeAccessor().GetValue("Source") == source.GetParent()->GetGuid(), "Property should have been set at this point already");
  NS_ASSERT_DEBUG(pObject->GetTypeAccessor().GetValue("Target") == target.GetParent()->GetGuid(), "Property should have been set at this point already");
  NS_ASSERT_DEBUG(pObject->GetTypeAccessor().GetValue("SourcePin") == source.GetName(), "Property should have been set at this point already");
  NS_ASSERT_DEBUG(pObject->GetTypeAccessor().GetValue("TargetPin") == target.GetName(), "Property should have been set at this point already");

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
  }
}

void nsDocumentNodeManager::RestoreMetaDataAfterLoading(const nsAbstractObjectGraph& graph, bool bUndoable)
{
  nsCommandHistory* history = GetDocument()->GetCommandHistory();

  auto pNodeMetaDataType = nsGetStaticRTTI<DocumentNodeManager_NodeMetaData>();
  auto pConnectionMetaDataType = nsGetStaticRTTI<DocumentNodeManager_ConnectionMetaData>();

  nsRttiConverterContext context;
  nsRttiConverterReader rttiConverter(&graph, &context);

  // Ensure that all nodes have their pins created
  for (auto it : graph.GetAllNodes())
  {
    auto pAbstractObject = it.Value();
    nsDocumentObject* pObject = GetObject(pAbstractObject->GetGuid());
    if (pObject != nullptr && IsNode(pObject))
    {
      auto& nodeInternal = m_ObjectToNode[pObject->GetGuid()];
      if (nodeInternal.m_Inputs.IsEmpty() && nodeInternal.m_Outputs.IsEmpty())
      {
        InternalCreatePins(pObject, nodeInternal);
      }
    }
  }

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

      NS_ASSERT_DEV(pAbstractObject->FindProperty("Node::Connections") == nullptr, "Old file format detected that is not supported anymore. Re-save the document with a previous version of ns. ({})", GetDocument()->GetDocumentPath());
    }
    else if (IsConnection(pObject))
    {
      nsVariant sourceVar = pObject->GetTypeAccessor().GetValue("Source");
      nsVariant targetVar = pObject->GetTypeAccessor().GetValue("Target");
      nsVariant sourcePinVar = pObject->GetTypeAccessor().GetValue("SourcePin");
      nsVariant targetPinVar = pObject->GetTypeAccessor().GetValue("TargetPin");
      NS_ASSERT_DEV(sourceVar.IsA<nsUuid>() && targetVar.IsA<nsUuid>() && sourcePinVar.IsA<nsString>() && targetPinVar.IsA<nsString>(), "Invalid connection object");

      nsUuid source = sourceVar.Get<nsUuid>();
      nsUuid target = targetVar.Get<nsUuid>();
      nsStringView sourcePin = sourcePinVar.Get<nsString>();
      nsStringView targetPin = targetPinVar.Get<nsString>();

      const nsPin* pSourcePin = nullptr;
      const nsPin* pTargetPin = nullptr;
      if (ResolveConnection(source, target, sourcePin, targetPin, pSourcePin, pTargetPin).Failed())
      {
        // Try to restore from metadata
        DocumentNodeManager_ConnectionMetaData connectionMetaData;
        rttiConverter.ApplyPropertiesToObject(pAbstractObject, pConnectionMetaDataType, &connectionMetaData);
        if (connectionMetaData.IsValid())
        {
          pObject->GetTypeAccessor().SetValue("Source", connectionMetaData.m_Source);
          pObject->GetTypeAccessor().SetValue("Target", connectionMetaData.m_Target);
          pObject->GetTypeAccessor().SetValue("SourcePin", connectionMetaData.m_SourcePin);
          pObject->GetTypeAccessor().SetValue("TargetPin", connectionMetaData.m_TargetPin);

          source = connectionMetaData.m_Source;
          target = connectionMetaData.m_Target;
          sourcePin = connectionMetaData.m_SourcePin;
          targetPin = connectionMetaData.m_TargetPin;
        }
      }

      if (ResolveConnection(source, target, sourcePin, targetPin, pSourcePin, pTargetPin).Succeeded())
      {
        if (bUndoable)
        {
          nsConnectNodePinsCommand cmd;
          cmd.m_ConnectionObject = pObject->GetGuid();
          cmd.m_ObjectSource = pSourcePin->GetParent()->GetGuid();
          cmd.m_ObjectTarget = pTargetPin->GetParent()->GetGuid();
          cmd.m_sSourcePin = pSourcePin->GetName();
          cmd.m_sTargetPin = pTargetPin->GetName();
          history->AddCommand(cmd).LogFailure();
        }
        else
        {
          Connect(pObject, *pSourcePin, *pTargetPin);
        }
      }
      else
      {
        RemoveObject(pObject);
        DestroyObject(pObject);
      }
    }
    else
    {
      DocumentNodeManager_ConnectionMetaData connectionMetaData;
      rttiConverter.ApplyPropertiesToObject(pAbstractObject, pConnectionMetaDataType, &connectionMetaData);

      if (connectionMetaData.IsValid() == false)
        continue;

      const nsPin* pSourcePin = nullptr;
      const nsPin* pTargetPin = nullptr;
      if (ResolveConnection(connectionMetaData.m_Source, connectionMetaData.m_Target, connectionMetaData.m_SourcePin, connectionMetaData.m_TargetPin, pSourcePin, pTargetPin).Succeeded())
      {
        nsDocumentObject* pNewConnectionObject = CreateObject(GetConnectionType());
        pNewConnectionObject->GetTypeAccessor().SetValue("Source", connectionMetaData.m_Source);
        pNewConnectionObject->GetTypeAccessor().SetValue("Target", connectionMetaData.m_Target);
        pNewConnectionObject->GetTypeAccessor().SetValue("SourcePin", connectionMetaData.m_SourcePin);
        pNewConnectionObject->GetTypeAccessor().SetValue("TargetPin", connectionMetaData.m_TargetPin);
        AddObject(pNewConnectionObject, nullptr, "", -1);

        NS_ASSERT_DEV(bUndoable == false, "This code path should only be taken by document loading code");
        Connect(pNewConnectionObject, *pSourcePin, *pTargetPin);
      }

      RemoveObject(pObject);
      DestroyObject(pObject);
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

nsResult nsDocumentNodeManager::ResolveConnection(const nsUuid& sourceObject, const nsUuid& targetObject, nsStringView sourcePin, nsStringView targetPin, const nsPin*& out_pSourcePin, const nsPin*& out_pTargetPin) const
{
  const nsDocumentObject* pSource = GetObject(sourceObject);
  const nsDocumentObject* pTarget = GetObject(targetObject);
  if (pSource == nullptr || pTarget == nullptr)
  {
    return NS_FAILURE;
  }

  const nsPin* pSourcePin = GetOutputPinByName(pSource, sourcePin);
  if (pSourcePin == nullptr)
  {
    nsLog::Error("Unknown output pin '{}' on '{}'. The connection has been removed.", sourcePin, pSource->GetType()->GetTypeName());
    return NS_FAILURE;
  }

  const nsPin* pTargetPin = GetInputPinByName(pTarget, targetPin);
  if (pTargetPin == nullptr)
  {
    nsLog::Error("Unknown input pin '{}' on '{}'. The connection has been removed.", targetPin, pTarget->GetType()->GetTypeName());
    return NS_FAILURE;
  }

  out_pSourcePin = pSourcePin;
  out_pTargetPin = pTargetPin;
  return NS_SUCCESS;
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
        sTemp.SetFormat("{}[{}]", sPinName, i);
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
        sTemp.SetFormat("{}", a[i]);
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
    else if (pArrayProp->GetSpecificType()->GetTypeFlags().IsSet(nsTypeFlags::Class))
    {
      for (nsUInt32 i = 0; i < uiCount; ++i)
      {
        auto pInnerObject = GetObject(a[i].Get<nsUuid>());
        if (pInnerObject == nullptr)
          continue;

        nsVariant nameVar = pInnerObject->GetTypeAccessor().GetValue("Name");
        if (nameVar.IsString() || nameVar.IsHashedString())
        {
          out_Names.PushBack(nameVar.ConvertTo<nsString>());
        }
        else
        {
          sTemp.SetFormat("{}[{}]", sPinName, i);
          out_Names.PushBack(sTemp);
        }
      }
    }
    else
    {
      for (nsUInt32 i = 0; i < uiCount; ++i)
      {
        sTemp.SetFormat("{}[{}]", sPinName, i);
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
    {
      nsLog::Error("Can't re-create pins if they are still connected");
      return false;
    }
  }

  for (auto& pPin : nodeInternal.m_Outputs)
  {
    if (HasConnections(*pPin))
    {
      nsLog::Error("Can't re-create pins if they are still connected");
      return false;
    }
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
      else
      {
        HandlePotentialDynamicPinPropertyChanged(e.m_pNewParent, e.m_sParentProperty);
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
      else
      {
        HandlePotentialDynamicPinPropertyChanged(e.m_pPreviousParent, e.m_sParentProperty);
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

  HandlePotentialDynamicPinPropertyChanged(e.m_pObject, e.m_sProperty);

  if (const nsDocumentObject* pParent = e.m_pObject->GetParent())
  {
    HandlePotentialDynamicPinPropertyChanged(pParent, e.m_pObject->GetParentProperty());
  }
}

void nsDocumentNodeManager::HandlePotentialDynamicPinPropertyChanged(const nsDocumentObject* pObject, nsStringView sPropertyName)
{
  if (pObject == nullptr)
    return;

  const nsAbstractProperty* pProp = pObject->GetType()->FindPropertyByName(sPropertyName);
  if (pProp == nullptr)
    return;

  if (IsDynamicPinProperty(pObject, pProp))
  {
    TryRecreatePins(pObject);
  }
}
