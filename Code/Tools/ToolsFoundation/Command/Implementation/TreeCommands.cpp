#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Serialization/DdlSerializer.h>
#include <ToolsFoundation/Command/TreeCommands.h>
#include <ToolsFoundation/Document/PrefabUtils.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>
#include <ToolsFoundation/Reflection/PhantomRttiManager.h>
#include <ToolsFoundation/Serialization/DocumentObjectConverter.h>

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdAddObjectCommand, 1, wdRTTIDefaultAllocator<wdAddObjectCommand>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_ACCESSOR_PROPERTY("Type", GetType, SetType),
    WD_MEMBER_PROPERTY("ParentGuid", m_Parent),
    WD_MEMBER_PROPERTY("ParentProperty", m_sParentProperty),
    WD_MEMBER_PROPERTY("Index", m_Index),
    WD_MEMBER_PROPERTY("NewGuid", m_NewObjectGuid),
  }
  WD_END_PROPERTIES;
}
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdPasteObjectsCommand, 1, wdRTTIDefaultAllocator<wdPasteObjectsCommand>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("ParentGuid", m_Parent),
    WD_MEMBER_PROPERTY("TextGraph", m_sGraphTextFormat),
    WD_MEMBER_PROPERTY("Mime", m_sMimeType),
    WD_MEMBER_PROPERTY("AllowPickedPosition", m_bAllowPickedPosition),
  }
  WD_END_PROPERTIES;
}
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdInstantiatePrefabCommand, 1, wdRTTIDefaultAllocator<wdInstantiatePrefabCommand>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("ParentGuid", m_Parent),
    WD_MEMBER_PROPERTY("CreateFromPrefab", m_CreateFromPrefab),
    WD_MEMBER_PROPERTY("BaseGraph", m_sBasePrefabGraph),
    WD_MEMBER_PROPERTY("ObjectGraph", m_sObjectGraph),
    WD_MEMBER_PROPERTY("RemapGuid", m_RemapGuid),
    WD_MEMBER_PROPERTY("CreatedObjects", m_CreatedRootObject),
    WD_MEMBER_PROPERTY("AllowPickedPos", m_bAllowPickedPosition),
    WD_MEMBER_PROPERTY("Index", m_Index),
  }
  WD_END_PROPERTIES;
}
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdUnlinkPrefabCommand, 1, wdRTTIDefaultAllocator<wdUnlinkPrefabCommand>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Object", m_Object),
  }
  WD_END_PROPERTIES;
}
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdRemoveObjectCommand, 1, wdRTTIDefaultAllocator<wdRemoveObjectCommand>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("ObjectGuid", m_Object),
  }
  WD_END_PROPERTIES;
}
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdMoveObjectCommand, 1, wdRTTIDefaultAllocator<wdMoveObjectCommand>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("ObjectGuid", m_Object),
    WD_MEMBER_PROPERTY("NewParentGuid", m_NewParent),
    WD_MEMBER_PROPERTY("ParentProperty", m_sParentProperty),
    WD_MEMBER_PROPERTY("Index", m_Index),
  }
  WD_END_PROPERTIES;
}
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdSetObjectPropertyCommand, 1, wdRTTIDefaultAllocator<wdSetObjectPropertyCommand>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("ObjectGuid", m_Object),
    WD_MEMBER_PROPERTY("NewValue", m_NewValue),
    WD_MEMBER_PROPERTY("Index", m_Index),
    WD_MEMBER_PROPERTY("Property", m_sProperty),
  }
  WD_END_PROPERTIES;
}
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdResizeAndSetObjectPropertyCommand, 1, wdRTTIDefaultAllocator<wdResizeAndSetObjectPropertyCommand>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("ObjectGuid", m_Object),
    WD_MEMBER_PROPERTY("NewValue", m_NewValue),
    WD_MEMBER_PROPERTY("Index", m_Index),
    WD_MEMBER_PROPERTY("Property", m_sProperty),
  }
  WD_END_PROPERTIES;
}
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdInsertObjectPropertyCommand, 1, wdRTTIDefaultAllocator<wdInsertObjectPropertyCommand>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("ObjectGuid", m_Object),
    WD_MEMBER_PROPERTY("NewValue", m_NewValue),
    WD_MEMBER_PROPERTY("Index", m_Index),
    WD_MEMBER_PROPERTY("Property", m_sProperty),
  }
  WD_END_PROPERTIES;
}
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdRemoveObjectPropertyCommand, 1, wdRTTIDefaultAllocator<wdRemoveObjectPropertyCommand>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("ObjectGuid", m_Object),
    WD_MEMBER_PROPERTY("Index", m_Index),
    WD_MEMBER_PROPERTY("Property", m_sProperty),
  }
  WD_END_PROPERTIES;
}
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdMoveObjectPropertyCommand, 1, wdRTTIDefaultAllocator<wdMoveObjectPropertyCommand>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("ObjectGuid", m_Object),
    WD_MEMBER_PROPERTY("OldIndex", m_OldIndex),
    WD_MEMBER_PROPERTY("NewIndex", m_NewIndex),
    WD_MEMBER_PROPERTY("Property", m_sProperty),
  }
  WD_END_PROPERTIES;
}
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

////////////////////////////////////////////////////////////////////////
// wdAddObjectCommand
////////////////////////////////////////////////////////////////////////

wdAddObjectCommand::wdAddObjectCommand()
  : m_pType(nullptr)
  , m_pObject(nullptr)
{
}

const char* wdAddObjectCommand::GetType() const
{
  if (m_pType == nullptr)
    return "";

  return m_pType->GetTypeName();
}

void wdAddObjectCommand::SetType(const char* szType)
{
  m_pType = wdRTTI::FindTypeByName(szType);
}

wdStatus wdAddObjectCommand::DoInternal(bool bRedo)
{
  wdDocument* pDocument = GetDocument();

  if (!bRedo)
  {
    if (!m_NewObjectGuid.IsValid())
      m_NewObjectGuid.CreateNewUuid();
  }

  wdDocumentObject* pParent = nullptr;
  if (m_Parent.IsValid())
  {
    pParent = pDocument->GetObjectManager()->GetObject(m_Parent);
    if (pParent == nullptr)
      return wdStatus("Add Object: The given parent does not exist!");
  }

  WD_SUCCEED_OR_RETURN(pDocument->GetObjectManager()->CanAdd(m_pType, pParent, m_sParentProperty, m_Index));

  if (!bRedo)
  {
    m_pObject = pDocument->GetObjectManager()->CreateObject(m_pType, m_NewObjectGuid);
  }

  pDocument->GetObjectManager()->AddObject(m_pObject, pParent, m_sParentProperty, m_Index);
  return wdStatus(WD_SUCCESS);
}

wdStatus wdAddObjectCommand::UndoInternal(bool bFireEvents)
{
  WD_ASSERT_DEV(bFireEvents, "This command does not support temporary commands");

  wdDocument* pDocument = GetDocument();
  WD_SUCCEED_OR_RETURN(pDocument->GetObjectManager()->CanRemove(m_pObject));

  pDocument->GetObjectManager()->RemoveObject(m_pObject);
  return wdStatus(WD_SUCCESS);
}

void wdAddObjectCommand::CleanupInternal(CommandState state)
{
  if (state == CommandState::WasUndone)
  {
    GetDocument()->GetObjectManager()->DestroyObject(m_pObject);
    m_pObject = nullptr;
  }
}


////////////////////////////////////////////////////////////////////////
// wdPasteObjectsCommand
////////////////////////////////////////////////////////////////////////

wdPasteObjectsCommand::wdPasteObjectsCommand() {}

wdStatus wdPasteObjectsCommand::DoInternal(bool bRedo)
{
  wdDocument* pDocument = GetDocument();

  wdDocumentObject* pParent = nullptr;
  if (m_Parent.IsValid())
  {
    pParent = pDocument->GetObjectManager()->GetObject(m_Parent);
    if (pParent == nullptr)
      return wdStatus("Paste Objects: The given parent does not exist!");
  }

  if (!bRedo)
  {
    wdAbstractObjectGraph graph;

    {
      // Deserialize
      wdRawMemoryStreamReader memoryReader(m_sGraphTextFormat.GetData(), m_sGraphTextFormat.GetElementCount());
      WD_SUCCEED_OR_RETURN(wdAbstractGraphDdlSerializer::Read(memoryReader, &graph));
    }

    // Remap
    wdUuid seed;
    seed.CreateNewUuid();
    graph.ReMapNodeGuids(seed);

    wdDocumentObjectConverterReader reader(&graph, pDocument->GetObjectManager(), wdDocumentObjectConverterReader::Mode::CreateOnly);

    wdHybridArray<wdAbstractObjectNode*, 16> RootNodes;
    auto& nodes = graph.GetAllNodes();
    for (auto it = nodes.GetIterator(); it.IsValid(); ++it)
    {
      auto* pNode = it.Value();
      if (wdStringUtils::IsEqual(pNode->GetNodeName(), "root"))
      {
        RootNodes.PushBack(pNode);
      }
    }

    RootNodes.Sort([](const wdAbstractObjectNode* a, const wdAbstractObjectNode* b) {
      auto* pOrderA = a->FindProperty("__Order");
      auto* pOrderB = b->FindProperty("__Order");
      if (pOrderA && pOrderB && pOrderA->m_Value.CanConvertTo<wdUInt32>() && pOrderB->m_Value.CanConvertTo<wdUInt32>())
      {
        return pOrderA->m_Value.ConvertTo<wdUInt32>() < pOrderB->m_Value.ConvertTo<wdUInt32>();
      }
      return a < b;
    });

    wdHybridArray<wdDocument::PasteInfo, 16> ToBePasted;
    for (wdAbstractObjectNode* pNode : RootNodes)
    {
      auto* pNewObject = reader.CreateObjectFromNode(pNode);

      if (pNewObject)
      {
        reader.ApplyPropertiesToObject(pNode, pNewObject);

        auto& ref = ToBePasted.ExpandAndGetRef();
        ref.m_pObject = pNewObject;
        ref.m_pParent = pParent;
      }
    }

    if (pDocument->Paste(ToBePasted, graph, m_bAllowPickedPosition, m_sMimeType))
    {
      for (const auto& item : ToBePasted)
      {
        auto& po = m_PastedObjects.ExpandAndGetRef();
        po.m_pObject = item.m_pObject;
        po.m_pParent = item.m_pParent;
        po.m_Index = item.m_pObject->GetPropertyIndex();
        po.m_sParentProperty = item.m_pObject->GetParentProperty();
      }
    }
    else
    {
      for (const auto& item : ToBePasted)
      {
        pDocument->GetObjectManager()->DestroyObject(item.m_pObject);
      }
    }

    if (m_PastedObjects.IsEmpty())
      return wdStatus("Paste Objects: nothing was pasted!");
  }
  else
  {
    // Re-add at recorded place.
    for (auto& po : m_PastedObjects)
    {
      pDocument->GetObjectManager()->AddObject(po.m_pObject, po.m_pParent, po.m_sParentProperty, po.m_Index);
    }
  }
  return wdStatus(WD_SUCCESS);
}

wdStatus wdPasteObjectsCommand::UndoInternal(bool bFireEvents)
{
  WD_ASSERT_DEV(bFireEvents, "This command does not support temporary commands");
  wdDocument* pDocument = GetDocument();

  for (auto& po : m_PastedObjects)
  {
    WD_SUCCEED_OR_RETURN(pDocument->GetObjectManager()->CanRemove(po.m_pObject));

    pDocument->GetObjectManager()->RemoveObject(po.m_pObject);
  }

  return wdStatus(WD_SUCCESS);
}

void wdPasteObjectsCommand::CleanupInternal(CommandState state)
{
  if (state == CommandState::WasUndone)
  {
    for (auto& po : m_PastedObjects)
    {
      GetDocument()->GetObjectManager()->DestroyObject(po.m_pObject);
    }
    m_PastedObjects.Clear();
  }
}

////////////////////////////////////////////////////////////////////////
// wdInstantiatePrefabCommand
////////////////////////////////////////////////////////////////////////

wdInstantiatePrefabCommand::wdInstantiatePrefabCommand()
{
  m_bAllowPickedPosition = true;
}

wdStatus wdInstantiatePrefabCommand::DoInternal(bool bRedo)
{
  wdDocument* pDocument = GetDocument();

  wdDocumentObject* pParent = nullptr;
  if (m_Parent.IsValid())
  {
    pParent = pDocument->GetObjectManager()->GetObject(m_Parent);
    if (pParent == nullptr)
      return wdStatus("Instantiate Prefab: The given parent does not exist!");
  }

  if (!bRedo)
  {
    // TODO: this is hard-coded, it only works for scene documents !
    const wdRTTI* pRootObjectType = wdRTTI::FindTypeByName("wdGameObject");
    const char* szParentProperty = "Children";

    wdDocumentObject* pRootObject = nullptr;
    wdHybridArray<wdDocument::PasteInfo, 16> ToBePasted;
    wdAbstractObjectGraph graph;

    // create root object
    {
      WD_SUCCEED_OR_RETURN(pDocument->GetObjectManager()->CanAdd(pRootObjectType, pParent, szParentProperty, m_Index));

      // use the same GUID for the root object ID as the remap GUID, this way the object ID is deterministic and reproducible
      m_CreatedRootObject = m_RemapGuid;

      pRootObject = pDocument->GetObjectManager()->CreateObject(pRootObjectType, m_CreatedRootObject);

      auto& ref = ToBePasted.ExpandAndGetRef();
      ref.m_pObject = pRootObject;
      ref.m_pParent = pParent;
      ref.m_Index = m_Index;
    }

    // update meta data
    // this is read when Paste is executed, to determine a good node name
    {
      // if prefabs are not allowed in this document, just create this as a regular object, with no link to the prefab template
      if (pDocument->ArePrefabsAllowed())
      {
        auto pMeta = pDocument->m_DocumentObjectMetaData->BeginModifyMetaData(m_CreatedRootObject);
        pMeta->m_CreateFromPrefab = m_CreateFromPrefab;
        pMeta->m_PrefabSeedGuid = m_RemapGuid;
        pMeta->m_sBasePrefab = m_sBasePrefabGraph;
        pDocument->m_DocumentObjectMetaData->EndModifyMetaData(wdDocumentObjectMetaData::PrefabFlag);
      }
      else
      {
        pDocument->ShowDocumentStatus("Nested prefabs are not allowed. Instantiated object will not be linked to prefab template.");
      }
    }

    if (pDocument->Paste(ToBePasted, graph, m_bAllowPickedPosition, "application/wdEditor.wdAbstractGraph"))
    {
      for (const auto& item : ToBePasted)
      {
        auto& po = m_PastedObjects.ExpandAndGetRef();
        po.m_pObject = item.m_pObject;
        po.m_pParent = item.m_pParent;
        po.m_Index = item.m_pObject->GetPropertyIndex();
        po.m_sParentProperty = item.m_pObject->GetParentProperty();
      }
    }
    else
    {
      for (const auto& item : ToBePasted)
      {
        pDocument->GetObjectManager()->DestroyObject(item.m_pObject);
      }

      ToBePasted.Clear();
    }

    if (m_PastedObjects.IsEmpty())
      return wdStatus("Paste Objects: nothing was pasted!");

    if (!m_sObjectGraph.IsEmpty())
      wdPrefabUtils::LoadGraph(graph, m_sObjectGraph);
    else
      wdPrefabUtils::LoadGraph(graph, m_sBasePrefabGraph);

    graph.ReMapNodeGuids(m_RemapGuid);

    // a prefab can have multiple top level nodes
    wdHybridArray<wdAbstractObjectNode*, 4> rootNodes;
    wdPrefabUtils::GetRootNodes(graph, rootNodes);

    for (auto* pPrefabRoot : rootNodes)
    {
      wdDocumentObjectConverterReader reader(&graph, pDocument->GetObjectManager(), wdDocumentObjectConverterReader::Mode::CreateOnly);

      if (auto* pNewObject = reader.CreateObjectFromNode(pPrefabRoot))
      {
        reader.ApplyPropertiesToObject(pPrefabRoot, pNewObject);

        // attach all prefab nodes to the main group node
        pDocument->GetObjectManager()->AddObject(pNewObject, pRootObject, szParentProperty, -1);
      }
    }
  }
  else
  {
    // Re-add at recorded place.
    for (auto& po : m_PastedObjects)
    {
      pDocument->GetObjectManager()->AddObject(po.m_pObject, po.m_pParent, po.m_sParentProperty, po.m_Index);
    }
  }

  return wdStatus(WD_SUCCESS);
}

wdStatus wdInstantiatePrefabCommand::UndoInternal(bool bFireEvents)
{
  WD_ASSERT_DEV(bFireEvents, "This command does not support temporary commands");
  wdDocument* pDocument = GetDocument();

  for (auto& po : m_PastedObjects)
  {
    WD_SUCCEED_OR_RETURN(pDocument->GetObjectManager()->CanRemove(po.m_pObject));

    pDocument->GetObjectManager()->RemoveObject(po.m_pObject);
  }

  return wdStatus(WD_SUCCESS);
}

void wdInstantiatePrefabCommand::CleanupInternal(CommandState state)
{
  if (state == CommandState::WasUndone)
  {
    for (auto& po : m_PastedObjects)
    {
      GetDocument()->GetObjectManager()->DestroyObject(po.m_pObject);
    }
    m_PastedObjects.Clear();
  }
}


//////////////////////////////////////////////////////////////////////////
// wdUnlinkPrefabCommand
//////////////////////////////////////////////////////////////////////////

wdStatus wdUnlinkPrefabCommand::DoInternal(bool bRedo)
{
  wdDocument* pDocument = GetDocument();
  wdDocumentObject* pObject = pDocument->GetObjectManager()->GetObject(m_Object);

  if (pObject == nullptr)
    return wdStatus("Unlink Prefab: The given object does not exist!");

  // store previous values
  if (!bRedo)
  {
    auto pMeta = pDocument->m_DocumentObjectMetaData->BeginReadMetaData(m_Object);
    m_OldCreateFromPrefab = pMeta->m_CreateFromPrefab;
    m_OldRemapGuid = pMeta->m_PrefabSeedGuid;
    m_sOldGraphTextFormat = pMeta->m_sBasePrefab;
    pDocument->m_DocumentObjectMetaData->EndReadMetaData();
  }

  // unlink
  {
    auto pMeta = pDocument->m_DocumentObjectMetaData->BeginModifyMetaData(m_Object);
    pMeta->m_CreateFromPrefab = wdUuid();
    pMeta->m_PrefabSeedGuid = wdUuid();
    pMeta->m_sBasePrefab.Clear();
    pDocument->m_DocumentObjectMetaData->EndModifyMetaData(wdDocumentObjectMetaData::PrefabFlag);
  }

  return wdStatus(WD_SUCCESS);
}

wdStatus wdUnlinkPrefabCommand::UndoInternal(bool bFireEvents)
{
  wdDocument* pDocument = GetDocument();
  wdDocumentObject* pObject = pDocument->GetObjectManager()->GetObject(m_Object);

  if (pObject == nullptr)
    return wdStatus("Unlink Prefab: The given object does not exist!");

  // restore link
  {
    auto pMeta = pDocument->m_DocumentObjectMetaData->BeginModifyMetaData(m_Object);
    pMeta->m_CreateFromPrefab = m_OldCreateFromPrefab;
    pMeta->m_PrefabSeedGuid = m_OldRemapGuid;
    pMeta->m_sBasePrefab = m_sOldGraphTextFormat;
    pDocument->m_DocumentObjectMetaData->EndModifyMetaData(wdDocumentObjectMetaData::PrefabFlag);
  }

  return wdStatus(WD_SUCCESS);
}


////////////////////////////////////////////////////////////////////////
// wdRemoveObjectCommand
////////////////////////////////////////////////////////////////////////

wdRemoveObjectCommand::wdRemoveObjectCommand()
  : m_pParent(nullptr)
  , m_pObject(nullptr)
{
}

wdStatus wdRemoveObjectCommand::DoInternal(bool bRedo)
{
  wdDocument* pDocument = GetDocument();

  if (!bRedo)
  {
    if (m_Object.IsValid())
    {
      m_pObject = pDocument->GetObjectManager()->GetObject(m_Object);
      if (m_pObject == nullptr)
        return wdStatus("Remove Object: The given object does not exist!");
    }
    else
      return wdStatus("Remove Object: The given object does not exist!");

    WD_SUCCEED_OR_RETURN(pDocument->GetObjectManager()->CanRemove(m_pObject));

    m_pParent = const_cast<wdDocumentObject*>(m_pObject->GetParent());
    m_sParentProperty = m_pObject->GetParentProperty();
    const wdIReflectedTypeAccessor& accessor = m_pObject->GetParent()->GetTypeAccessor();
    m_Index = accessor.GetPropertyChildIndex(m_pObject->GetParentProperty(), m_pObject->GetGuid());
  }

  pDocument->GetObjectManager()->RemoveObject(m_pObject);
  return wdStatus(WD_SUCCESS);
}

wdStatus wdRemoveObjectCommand::UndoInternal(bool bFireEvents)
{
  WD_ASSERT_DEV(bFireEvents, "This command does not support temporary commands");

  wdDocument* pDocument = GetDocument();
  WD_SUCCEED_OR_RETURN(pDocument->GetObjectManager()->CanAdd(m_pObject->GetTypeAccessor().GetType(), m_pParent, m_sParentProperty, m_Index));

  pDocument->GetObjectManager()->AddObject(m_pObject, m_pParent, m_sParentProperty, m_Index);
  return wdStatus(WD_SUCCESS);
}

void wdRemoveObjectCommand::CleanupInternal(CommandState state)
{
  if (state == CommandState::WasDone)
  {
    GetDocument()->GetObjectManager()->DestroyObject(m_pObject);
    m_pObject = nullptr;
  }
}


////////////////////////////////////////////////////////////////////////
// wdMoveObjectCommand
////////////////////////////////////////////////////////////////////////

wdMoveObjectCommand::wdMoveObjectCommand()
{
  m_pObject = nullptr;
  m_pOldParent = nullptr;
  m_pNewParent = nullptr;
}

wdStatus wdMoveObjectCommand::DoInternal(bool bRedo)
{
  wdDocument* pDocument = GetDocument();

  if (!bRedo)
  {
    {
      m_pObject = pDocument->GetObjectManager()->GetObject(m_Object);
      if (m_pObject == nullptr)
        return wdStatus("Move Object: The given object does not exist!");
    }

    if (m_NewParent.IsValid())
    {
      m_pNewParent = pDocument->GetObjectManager()->GetObject(m_NewParent);
      if (m_pNewParent == nullptr)
        return wdStatus("Move Object: The new parent does not exist!");
    }

    m_pOldParent = const_cast<wdDocumentObject*>(m_pObject->GetParent());
    m_sOldParentProperty = m_pObject->GetParentProperty();
    const wdIReflectedTypeAccessor& accessor = m_pOldParent->GetTypeAccessor();
    m_OldIndex = accessor.GetPropertyChildIndex(m_pObject->GetParentProperty(), m_pObject->GetGuid());

    WD_SUCCEED_OR_RETURN(pDocument->GetObjectManager()->CanMove(m_pObject, m_pNewParent, m_sParentProperty, m_Index));
  }

  pDocument->GetObjectManager()->MoveObject(m_pObject, m_pNewParent, m_sParentProperty, m_Index);
  return wdStatus(WD_SUCCESS);
}

wdStatus wdMoveObjectCommand::UndoInternal(bool bFireEvents)
{
  WD_ASSERT_DEV(bFireEvents, "This command does not support temporary commands");

  wdDocument* pDocument = GetDocument();

  wdVariant FinalOldPosition = m_OldIndex;

  if (m_Index.CanConvertTo<wdInt32>() && m_pOldParent == m_pNewParent)
  {
    // If we are moving an object downwards, we must move by more than 1 (+1 would be behind the same object, which is still the same
    // position) so an object must always be moved by at least +2 moving UP can be done by -1, so when we undo that, we must ensure to move
    // +2

    wdInt32 iNew = m_Index.ConvertTo<wdInt32>();
    wdInt32 iOld = m_OldIndex.ConvertTo<wdInt32>();

    if (iNew < iOld)
    {
      FinalOldPosition = iOld + 1;
    }
  }

  WD_SUCCEED_OR_RETURN(pDocument->GetObjectManager()->CanMove(m_pObject, m_pOldParent, m_sOldParentProperty, FinalOldPosition));

  pDocument->GetObjectManager()->MoveObject(m_pObject, m_pOldParent, m_sOldParentProperty, FinalOldPosition);

  return wdStatus(WD_SUCCESS);
}


////////////////////////////////////////////////////////////////////////
// wdSetObjectPropertyCommand
////////////////////////////////////////////////////////////////////////

wdSetObjectPropertyCommand::wdSetObjectPropertyCommand()
{
  m_pObject = nullptr;
}

wdStatus wdSetObjectPropertyCommand::DoInternal(bool bRedo)
{
  wdDocument* pDocument = GetDocument();

  if (!bRedo)
  {
    if (m_Object.IsValid())
    {
      m_pObject = pDocument->GetObjectManager()->GetObject(m_Object);
      if (m_pObject == nullptr)
        return wdStatus("Set Property: The given object does not exist!");
    }
    else
      return wdStatus("Set Property: The given object does not exist!");

    wdIReflectedTypeAccessor& accessor0 = m_pObject->GetTypeAccessor();

    wdStatus res;
    m_OldValue = accessor0.GetValue(m_sProperty, m_Index, &res);
    if (res.Failed())
      return res;
    wdAbstractProperty* pProp = accessor0.GetType()->FindPropertyByName(m_sProperty);
    if (pProp == nullptr)
      return wdStatus(wdFmt("Set Property: The property '{0}' does not exist", m_sProperty));

    if (pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner))
    {
      return wdStatus(wdFmt("Set Property: The property '{0}' is a PointerOwner, use wdAddObjectCommand instead", m_sProperty));
    }

    if (pProp->GetAttributeByType<wdTemporaryAttribute>())
    {
      // if we modify a 'temporary' property, ie. one that is not serialized,
      // don't mark the document as modified
      m_bModifiedDocument = false;
    }
  }

  return pDocument->GetObjectManager()->SetValue(m_pObject, m_sProperty, m_NewValue, m_Index);
}

wdStatus wdSetObjectPropertyCommand::UndoInternal(bool bFireEvents)
{
  if (bFireEvents)
  {
    return GetDocument()->GetObjectManager()->SetValue(m_pObject, m_sProperty, m_OldValue, m_Index);
  }
  else
  {
    wdIReflectedTypeAccessor& accessor = m_pObject->GetTypeAccessor();
    if (!accessor.SetValue(m_sProperty, m_OldValue, m_Index))
    {
      return wdStatus(wdFmt("Set Property: The property '{0}' does not exist", m_sProperty));
    }
  }
  return wdStatus(WD_SUCCESS);
}

////////////////////////////////////////////////////////////////////////
// wdSetObjectPropertyCommand
////////////////////////////////////////////////////////////////////////

wdResizeAndSetObjectPropertyCommand::wdResizeAndSetObjectPropertyCommand()
{
  m_pObject = nullptr;
}

wdStatus wdResizeAndSetObjectPropertyCommand::DoInternal(bool bRedo)
{
  wdDocument* pDocument = GetDocument();

  if (!bRedo)
  {
    if (m_Object.IsValid())
    {
      m_pObject = pDocument->GetObjectManager()->GetObject(m_Object);
      if (m_pObject == nullptr)
        return wdStatus("Set Property: The given object does not exist!");
    }
    else
      return wdStatus("Set Property: The given object does not exist!");

    const wdInt32 uiIndex = m_Index.ConvertTo<wdInt32>();

    wdIReflectedTypeAccessor& accessor0 = m_pObject->GetTypeAccessor();

    const wdInt32 iCount = accessor0.GetCount(m_sProperty);

    for (wdInt32 i = iCount; i <= uiIndex; ++i)
    {
      wdInsertObjectPropertyCommand ins;
      ins.m_Object = m_Object;
      ins.m_sProperty = m_sProperty;
      ins.m_Index = i;
      ins.m_NewValue = wdReflectionUtils::GetDefaultVariantFromType(m_NewValue.GetType());

      AddSubCommand(ins);
    }

    wdSetObjectPropertyCommand set;
    set.m_sProperty = m_sProperty;
    set.m_Index = m_Index;
    set.m_NewValue = m_NewValue;
    set.m_Object = m_Object;

    AddSubCommand(set);
  }

  return wdStatus(WD_SUCCESS);
}

////////////////////////////////////////////////////////////////////////
// wdInsertObjectPropertyCommand
////////////////////////////////////////////////////////////////////////

wdInsertObjectPropertyCommand::wdInsertObjectPropertyCommand()
{
  m_pObject = nullptr;
}

wdStatus wdInsertObjectPropertyCommand::DoInternal(bool bRedo)
{
  wdDocument* pDocument = GetDocument();

  if (!bRedo)
  {
    if (m_Object.IsValid())
    {
      m_pObject = pDocument->GetObjectManager()->GetObject(m_Object);
      if (m_pObject == nullptr)
        return wdStatus("Insert Property: The given object does not exist!");
    }
    else
      return wdStatus("Insert Property: The given object does not exist!");

    if (m_Index.CanConvertTo<wdInt32>() && m_Index.ConvertTo<wdInt32>() == -1)
    {
      wdIReflectedTypeAccessor& accessor = m_pObject->GetTypeAccessor();
      m_Index = accessor.GetCount(m_sProperty.GetData());
    }
  }

  return pDocument->GetObjectManager()->InsertValue(m_pObject, m_sProperty, m_NewValue, m_Index);
}

wdStatus wdInsertObjectPropertyCommand::UndoInternal(bool bFireEvents)
{
  if (bFireEvents)
  {
    return GetDocument()->GetObjectManager()->RemoveValue(m_pObject, m_sProperty, m_Index);
  }
  else
  {
    wdIReflectedTypeAccessor& accessor = m_pObject->GetTypeAccessor();
    if (!accessor.RemoveValue(m_sProperty, m_Index))
    {
      return wdStatus(wdFmt("Insert Property: The property '{0}' does not exist", m_sProperty));
    }
  }

  return wdStatus(WD_SUCCESS);
}


////////////////////////////////////////////////////////////////////////
// wdRemoveObjectPropertyCommand
////////////////////////////////////////////////////////////////////////

wdRemoveObjectPropertyCommand::wdRemoveObjectPropertyCommand()
{
  m_pObject = nullptr;
}

wdStatus wdRemoveObjectPropertyCommand::DoInternal(bool bRedo)
{
  wdDocument* pDocument = GetDocument();

  if (!bRedo)
  {
    if (m_Object.IsValid())
    {
      m_pObject = pDocument->GetObjectManager()->GetObject(m_Object);
      if (m_pObject == nullptr)
        return wdStatus("Remove Property: The given object does not exist!");
      wdStatus res;
      m_OldValue = m_pObject->GetTypeAccessor().GetValue(m_sProperty, m_Index, &res);
      if (res.Failed())
        return res;
    }
    else
      return wdStatus("Remove Property: The given object does not exist!");
  }

  return pDocument->GetObjectManager()->RemoveValue(m_pObject, m_sProperty, m_Index);
}

wdStatus wdRemoveObjectPropertyCommand::UndoInternal(bool bFireEvents)
{
  if (bFireEvents)
  {
    return GetDocument()->GetObjectManager()->InsertValue(m_pObject, m_sProperty, m_OldValue, m_Index);
  }
  else
  {
    wdIReflectedTypeAccessor& accessor = m_pObject->GetTypeAccessor();
    if (!accessor.InsertValue(m_sProperty, m_Index, m_OldValue))
    {
      return wdStatus(wdFmt("Remove Property: Undo failed! The index '{0}' in property '{1}' does not exist", m_Index.ConvertTo<wdString>(), m_sProperty));
    }
  }
  return wdStatus(WD_SUCCESS);
}


////////////////////////////////////////////////////////////////////////
// wdMoveObjectPropertyCommand
////////////////////////////////////////////////////////////////////////

wdMoveObjectPropertyCommand::wdMoveObjectPropertyCommand()
{
  m_pObject = nullptr;
}

wdStatus wdMoveObjectPropertyCommand::DoInternal(bool bRedo)
{
  wdDocument* pDocument = GetDocument();

  if (!bRedo)
  {
    m_pObject = pDocument->GetObjectManager()->GetObject(m_Object);
    if (m_pObject == nullptr)
      return wdStatus("Move Property: The given object does not exist.");
  }

  return GetDocument()->GetObjectManager()->MoveValue(m_pObject, m_sProperty, m_OldIndex, m_NewIndex);
}

wdStatus wdMoveObjectPropertyCommand::UndoInternal(bool bFireEvents)
{
  WD_ASSERT_DEV(bFireEvents, "This command does not support temporary commands");

  wdVariant FinalOldPosition = m_OldIndex;
  wdVariant FinalNewPosition = m_NewIndex;

  if (m_OldIndex.CanConvertTo<wdInt32>())
  {
    // If we are moving an object downwards, we must move by more than 1 (+1 would be behind the same object, which is still the same
    // position) so an object must always be moved by at least +2 moving UP can be done by -1, so when we undo that, we must ensure to move
    // +2

    wdInt32 iNew = m_NewIndex.ConvertTo<wdInt32>();
    wdInt32 iOld = m_OldIndex.ConvertTo<wdInt32>();

    if (iNew < iOld)
    {
      FinalOldPosition = iOld + 1;
    }

    // The new position is relative to the original array, so we need to substract one to account for
    // the removal of the same element at the lower index.
    if (iNew > iOld)
    {
      FinalNewPosition = iNew - 1;
    }
  }

  return GetDocument()->GetObjectManager()->MoveValue(m_pObject, m_sProperty, FinalNewPosition, FinalOldPosition);
}
