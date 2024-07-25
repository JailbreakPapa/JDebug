#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Serialization/DdlSerializer.h>
#include <ToolsFoundation/Command/TreeCommands.h>
#include <ToolsFoundation/Document/PrefabUtils.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>
#include <ToolsFoundation/Reflection/PhantomRttiManager.h>
#include <ToolsFoundation/Serialization/DocumentObjectConverter.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsAddObjectCommand, 1, nsRTTIDefaultAllocator<nsAddObjectCommand>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("Type", GetType, SetType),
    NS_MEMBER_PROPERTY("ParentGuid", m_Parent),
    NS_MEMBER_PROPERTY("ParentProperty", m_sParentProperty),
    NS_MEMBER_PROPERTY("Index", m_Index),
    NS_MEMBER_PROPERTY("NewGuid", m_NewObjectGuid),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsPasteObjectsCommand, 1, nsRTTIDefaultAllocator<nsPasteObjectsCommand>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("ParentGuid", m_Parent),
    NS_MEMBER_PROPERTY("TextGraph", m_sGraphTextFormat),
    NS_MEMBER_PROPERTY("Mime", m_sMimeType),
    NS_MEMBER_PROPERTY("AllowPickedPosition", m_bAllowPickedPosition),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsInstantiatePrefabCommand, 1, nsRTTIDefaultAllocator<nsInstantiatePrefabCommand>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("ParentGuid", m_Parent),
    NS_MEMBER_PROPERTY("CreateFromPrefab", m_CreateFromPrefab),
    NS_MEMBER_PROPERTY("BaseGraph", m_sBasePrefabGraph),
    NS_MEMBER_PROPERTY("ObjectGraph", m_sObjectGraph),
    NS_MEMBER_PROPERTY("RemapGuid", m_RemapGuid),
    NS_MEMBER_PROPERTY("CreatedObjects", m_CreatedRootObject),
    NS_MEMBER_PROPERTY("AllowPickedPos", m_bAllowPickedPosition),
    NS_MEMBER_PROPERTY("Index", m_Index),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsUnlinkPrefabCommand, 1, nsRTTIDefaultAllocator<nsUnlinkPrefabCommand>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Object", m_Object),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsRemoveObjectCommand, 1, nsRTTIDefaultAllocator<nsRemoveObjectCommand>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("ObjectGuid", m_Object),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMoveObjectCommand, 1, nsRTTIDefaultAllocator<nsMoveObjectCommand>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("ObjectGuid", m_Object),
    NS_MEMBER_PROPERTY("NewParentGuid", m_NewParent),
    NS_MEMBER_PROPERTY("ParentProperty", m_sParentProperty),
    NS_MEMBER_PROPERTY("Index", m_Index),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSetObjectPropertyCommand, 1, nsRTTIDefaultAllocator<nsSetObjectPropertyCommand>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("ObjectGuid", m_Object),
    NS_MEMBER_PROPERTY("NewValue", m_NewValue),
    NS_MEMBER_PROPERTY("Index", m_Index),
    NS_MEMBER_PROPERTY("Property", m_sProperty),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsResizeAndSetObjectPropertyCommand, 1, nsRTTIDefaultAllocator<nsResizeAndSetObjectPropertyCommand>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("ObjectGuid", m_Object),
    NS_MEMBER_PROPERTY("NewValue", m_NewValue),
    NS_MEMBER_PROPERTY("Index", m_Index),
    NS_MEMBER_PROPERTY("Property", m_sProperty),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsInsertObjectPropertyCommand, 1, nsRTTIDefaultAllocator<nsInsertObjectPropertyCommand>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("ObjectGuid", m_Object),
    NS_MEMBER_PROPERTY("NewValue", m_NewValue),
    NS_MEMBER_PROPERTY("Index", m_Index),
    NS_MEMBER_PROPERTY("Property", m_sProperty),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsRemoveObjectPropertyCommand, 1, nsRTTIDefaultAllocator<nsRemoveObjectPropertyCommand>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("ObjectGuid", m_Object),
    NS_MEMBER_PROPERTY("Index", m_Index),
    NS_MEMBER_PROPERTY("Property", m_sProperty),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMoveObjectPropertyCommand, 1, nsRTTIDefaultAllocator<nsMoveObjectPropertyCommand>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("ObjectGuid", m_Object),
    NS_MEMBER_PROPERTY("OldIndex", m_OldIndex),
    NS_MEMBER_PROPERTY("NewIndex", m_NewIndex),
    NS_MEMBER_PROPERTY("Property", m_sProperty),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

////////////////////////////////////////////////////////////////////////
// nsAddObjectCommand
////////////////////////////////////////////////////////////////////////

nsAddObjectCommand::nsAddObjectCommand()

  = default;

nsStringView nsAddObjectCommand::GetType() const
{
  if (m_pType == nullptr)
    return {};

  return m_pType->GetTypeName();
}

void nsAddObjectCommand::SetType(nsStringView sType)
{
  m_pType = nsRTTI::FindTypeByName(sType);
}

nsStatus nsAddObjectCommand::DoInternal(bool bRedo)
{
  nsDocument* pDocument = GetDocument();

  if (!bRedo)
  {
    if (!m_NewObjectGuid.IsValid())
      m_NewObjectGuid = nsUuid::MakeUuid();
  }

  nsDocumentObject* pParent = nullptr;
  if (m_Parent.IsValid())
  {
    pParent = pDocument->GetObjectManager()->GetObject(m_Parent);
    if (pParent == nullptr)
      return nsStatus("Add Object: The given parent does not exist!");
  }

  NS_SUCCEED_OR_RETURN(pDocument->GetObjectManager()->CanAdd(m_pType, pParent, m_sParentProperty, m_Index));

  if (!bRedo)
  {
    m_pObject = pDocument->GetObjectManager()->CreateObject(m_pType, m_NewObjectGuid);
  }

  pDocument->GetObjectManager()->AddObject(m_pObject, pParent, m_sParentProperty, m_Index);
  return nsStatus(NS_SUCCESS);
}

nsStatus nsAddObjectCommand::UndoInternal(bool bFireEvents)
{
  NS_ASSERT_DEV(bFireEvents, "This command does not support temporary commands");

  nsDocument* pDocument = GetDocument();
  NS_SUCCEED_OR_RETURN(pDocument->GetObjectManager()->CanRemove(m_pObject));

  pDocument->GetObjectManager()->RemoveObject(m_pObject);
  return nsStatus(NS_SUCCESS);
}

void nsAddObjectCommand::CleanupInternal(CommandState state)
{
  if (state == CommandState::WasUndone)
  {
    GetDocument()->GetObjectManager()->DestroyObject(m_pObject);
    m_pObject = nullptr;
  }
}


////////////////////////////////////////////////////////////////////////
// nsPasteObjectsCommand
////////////////////////////////////////////////////////////////////////

nsPasteObjectsCommand::nsPasteObjectsCommand() = default;

nsStatus nsPasteObjectsCommand::DoInternal(bool bRedo)
{
  nsDocument* pDocument = GetDocument();

  nsDocumentObject* pParent = nullptr;
  if (m_Parent.IsValid())
  {
    pParent = pDocument->GetObjectManager()->GetObject(m_Parent);
    if (pParent == nullptr)
      return nsStatus("Paste Objects: The given parent does not exist!");
  }

  if (!bRedo)
  {
    nsAbstractObjectGraph graph;

    {
      // Deserialize
      nsRawMemoryStreamReader memoryReader(m_sGraphTextFormat.GetData(), m_sGraphTextFormat.GetElementCount());
      NS_SUCCEED_OR_RETURN(nsAbstractGraphDdlSerializer::Read(memoryReader, &graph));
    }

    // Remap
    graph.ReMapNodeGuids(nsUuid::MakeUuid());

    nsDocumentObjectConverterReader reader(&graph, pDocument->GetObjectManager(), nsDocumentObjectConverterReader::Mode::CreateOnly);

    nsHybridArray<nsAbstractObjectNode*, 16> RootNodes;
    auto& nodes = graph.GetAllNodes();
    for (auto it = nodes.GetIterator(); it.IsValid(); ++it)
    {
      auto* pNode = it.Value();
      if (pNode->GetNodeName() == "root")
      {
        RootNodes.PushBack(pNode);
      }
    }

    RootNodes.Sort([](const nsAbstractObjectNode* a, const nsAbstractObjectNode* b)
      {
      auto* pOrderA = a->FindProperty("__Order");
      auto* pOrderB = b->FindProperty("__Order");
      if (pOrderA && pOrderB && pOrderA->m_Value.CanConvertTo<nsUInt32>() && pOrderB->m_Value.CanConvertTo<nsUInt32>())
      {
        return pOrderA->m_Value.ConvertTo<nsUInt32>() < pOrderB->m_Value.ConvertTo<nsUInt32>();
      }
      return a < b; });

    nsHybridArray<nsDocument::PasteInfo, 16> ToBePasted;
    for (nsAbstractObjectNode* pNode : RootNodes)
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
      return nsStatus("Paste Objects: nothing was pasted!");
  }
  else
  {
    // Re-add at recorded place.
    for (auto& po : m_PastedObjects)
    {
      pDocument->GetObjectManager()->AddObject(po.m_pObject, po.m_pParent, po.m_sParentProperty, po.m_Index);
    }
  }
  return nsStatus(NS_SUCCESS);
}

nsStatus nsPasteObjectsCommand::UndoInternal(bool bFireEvents)
{
  NS_ASSERT_DEV(bFireEvents, "This command does not support temporary commands");
  nsDocument* pDocument = GetDocument();

  for (auto& po : m_PastedObjects)
  {
    NS_SUCCEED_OR_RETURN(pDocument->GetObjectManager()->CanRemove(po.m_pObject));

    pDocument->GetObjectManager()->RemoveObject(po.m_pObject);
  }

  return nsStatus(NS_SUCCESS);
}

void nsPasteObjectsCommand::CleanupInternal(CommandState state)
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
// nsInstantiatePrefabCommand
////////////////////////////////////////////////////////////////////////

nsInstantiatePrefabCommand::nsInstantiatePrefabCommand()
{
  m_bAllowPickedPosition = true;
}

nsStatus nsInstantiatePrefabCommand::DoInternal(bool bRedo)
{
  nsDocument* pDocument = GetDocument();

  nsDocumentObject* pParent = nullptr;
  if (m_Parent.IsValid())
  {
    pParent = pDocument->GetObjectManager()->GetObject(m_Parent);
    if (pParent == nullptr)
      return nsStatus("Instantiate Prefab: The given parent does not exist!");
  }

  if (!bRedo)
  {
    // TODO: this is hard-coded, it only works for scene documents !
    const nsRTTI* pRootObjectType = nsRTTI::FindTypeByName("nsGameObject");
    nsStringView sParentProperty = "Children"_nssv;

    nsDocumentObject* pRootObject = nullptr;
    nsHybridArray<nsDocument::PasteInfo, 16> ToBePasted;
    nsAbstractObjectGraph graph;

    // create root object
    {
      NS_SUCCEED_OR_RETURN(pDocument->GetObjectManager()->CanAdd(pRootObjectType, pParent, sParentProperty, m_Index));

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
        pDocument->m_DocumentObjectMetaData->EndModifyMetaData(nsDocumentObjectMetaData::PrefabFlag);
      }
      else
      {
        pDocument->ShowDocumentStatus("Nested prefabs are not allowed. Instantiated object will not be linked to prefab template.");
      }
    }

    if (pDocument->Paste(ToBePasted, graph, m_bAllowPickedPosition, "application/nsEditor.nsAbstractGraph"))
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
      return nsStatus("Paste Objects: nothing was pasted!");

    if (!m_sObjectGraph.IsEmpty())
      nsPrefabUtils::LoadGraph(graph, m_sObjectGraph);
    else
      nsPrefabUtils::LoadGraph(graph, m_sBasePrefabGraph);

    graph.ReMapNodeGuids(m_RemapGuid);

    // a prefab can have multiple top level nodes
    nsHybridArray<nsAbstractObjectNode*, 4> rootNodes;
    nsPrefabUtils::GetRootNodes(graph, rootNodes);

    for (auto* pPrefabRoot : rootNodes)
    {
      nsDocumentObjectConverterReader reader(&graph, pDocument->GetObjectManager(), nsDocumentObjectConverterReader::Mode::CreateOnly);

      if (auto* pNewObject = reader.CreateObjectFromNode(pPrefabRoot))
      {
        reader.ApplyPropertiesToObject(pPrefabRoot, pNewObject);

        // attach all prefab nodes to the main group node
        pDocument->GetObjectManager()->AddObject(pNewObject, pRootObject, sParentProperty, -1);
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

  return nsStatus(NS_SUCCESS);
}

nsStatus nsInstantiatePrefabCommand::UndoInternal(bool bFireEvents)
{
  NS_ASSERT_DEV(bFireEvents, "This command does not support temporary commands");
  nsDocument* pDocument = GetDocument();

  for (auto& po : m_PastedObjects)
  {
    NS_SUCCEED_OR_RETURN(pDocument->GetObjectManager()->CanRemove(po.m_pObject));

    pDocument->GetObjectManager()->RemoveObject(po.m_pObject);
  }

  return nsStatus(NS_SUCCESS);
}

void nsInstantiatePrefabCommand::CleanupInternal(CommandState state)
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
// nsUnlinkPrefabCommand
//////////////////////////////////////////////////////////////////////////

nsStatus nsUnlinkPrefabCommand::DoInternal(bool bRedo)
{
  nsDocument* pDocument = GetDocument();
  nsDocumentObject* pObject = pDocument->GetObjectManager()->GetObject(m_Object);

  if (pObject == nullptr)
    return nsStatus("Unlink Prefab: The given object does not exist!");

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
    pMeta->m_CreateFromPrefab = nsUuid();
    pMeta->m_PrefabSeedGuid = nsUuid();
    pMeta->m_sBasePrefab.Clear();
    pDocument->m_DocumentObjectMetaData->EndModifyMetaData(nsDocumentObjectMetaData::PrefabFlag);
  }

  return nsStatus(NS_SUCCESS);
}

nsStatus nsUnlinkPrefabCommand::UndoInternal(bool bFireEvents)
{
  nsDocument* pDocument = GetDocument();
  nsDocumentObject* pObject = pDocument->GetObjectManager()->GetObject(m_Object);

  if (pObject == nullptr)
    return nsStatus("Unlink Prefab: The given object does not exist!");

  // restore link
  {
    auto pMeta = pDocument->m_DocumentObjectMetaData->BeginModifyMetaData(m_Object);
    pMeta->m_CreateFromPrefab = m_OldCreateFromPrefab;
    pMeta->m_PrefabSeedGuid = m_OldRemapGuid;
    pMeta->m_sBasePrefab = m_sOldGraphTextFormat;
    pDocument->m_DocumentObjectMetaData->EndModifyMetaData(nsDocumentObjectMetaData::PrefabFlag);
  }

  return nsStatus(NS_SUCCESS);
}


////////////////////////////////////////////////////////////////////////
// nsRemoveObjectCommand
////////////////////////////////////////////////////////////////////////

nsRemoveObjectCommand::nsRemoveObjectCommand()

  = default;

nsStatus nsRemoveObjectCommand::DoInternal(bool bRedo)
{
  nsDocument* pDocument = GetDocument();

  if (!bRedo)
  {
    if (m_Object.IsValid())
    {
      m_pObject = pDocument->GetObjectManager()->GetObject(m_Object);
      if (m_pObject == nullptr)
        return nsStatus("Remove Object: The given object does not exist!");
    }
    else
      return nsStatus("Remove Object: The given object does not exist!");

    NS_SUCCEED_OR_RETURN(pDocument->GetObjectManager()->CanRemove(m_pObject));

    m_pParent = const_cast<nsDocumentObject*>(m_pObject->GetParent());
    m_sParentProperty = m_pObject->GetParentProperty();
    const nsIReflectedTypeAccessor& accessor = m_pObject->GetParent()->GetTypeAccessor();
    m_Index = accessor.GetPropertyChildIndex(m_pObject->GetParentProperty(), m_pObject->GetGuid());
  }

  pDocument->GetObjectManager()->RemoveObject(m_pObject);
  return nsStatus(NS_SUCCESS);
}

nsStatus nsRemoveObjectCommand::UndoInternal(bool bFireEvents)
{
  NS_ASSERT_DEV(bFireEvents, "This command does not support temporary commands");

  nsDocument* pDocument = GetDocument();
  NS_SUCCEED_OR_RETURN(pDocument->GetObjectManager()->CanAdd(m_pObject->GetTypeAccessor().GetType(), m_pParent, m_sParentProperty, m_Index));

  pDocument->GetObjectManager()->AddObject(m_pObject, m_pParent, m_sParentProperty, m_Index);
  return nsStatus(NS_SUCCESS);
}

void nsRemoveObjectCommand::CleanupInternal(CommandState state)
{
  if (state == CommandState::WasDone)
  {
    GetDocument()->GetObjectManager()->DestroyObject(m_pObject);
    m_pObject = nullptr;
  }
}


////////////////////////////////////////////////////////////////////////
// nsMoveObjectCommand
////////////////////////////////////////////////////////////////////////

nsMoveObjectCommand::nsMoveObjectCommand()
{
  m_pObject = nullptr;
  m_pOldParent = nullptr;
  m_pNewParent = nullptr;
}

nsStatus nsMoveObjectCommand::DoInternal(bool bRedo)
{
  nsDocument* pDocument = GetDocument();

  if (!bRedo)
  {
    {
      m_pObject = pDocument->GetObjectManager()->GetObject(m_Object);
      if (m_pObject == nullptr)
        return nsStatus("Move Object: The given object does not exist!");
    }

    if (m_NewParent.IsValid())
    {
      m_pNewParent = pDocument->GetObjectManager()->GetObject(m_NewParent);
      if (m_pNewParent == nullptr)
        return nsStatus("Move Object: The new parent does not exist!");
    }

    m_pOldParent = const_cast<nsDocumentObject*>(m_pObject->GetParent());
    m_sOldParentProperty = m_pObject->GetParentProperty();
    const nsIReflectedTypeAccessor& accessor = m_pOldParent->GetTypeAccessor();
    m_OldIndex = accessor.GetPropertyChildIndex(m_pObject->GetParentProperty(), m_pObject->GetGuid());

    NS_SUCCEED_OR_RETURN(pDocument->GetObjectManager()->CanMove(m_pObject, m_pNewParent, m_sParentProperty, m_Index));
  }

  pDocument->GetObjectManager()->MoveObject(m_pObject, m_pNewParent, m_sParentProperty, m_Index);
  return nsStatus(NS_SUCCESS);
}

nsStatus nsMoveObjectCommand::UndoInternal(bool bFireEvents)
{
  NS_ASSERT_DEV(bFireEvents, "This command does not support temporary commands");

  nsDocument* pDocument = GetDocument();

  nsVariant FinalOldPosition = m_OldIndex;

  if (m_Index.CanConvertTo<nsInt32>() && m_pOldParent == m_pNewParent)
  {
    // If we are moving an object downwards, we must move by more than 1 (+1 would be behind the same object, which is still the same
    // position) so an object must always be moved by at least +2 moving UP can be done by -1, so when we undo that, we must ensure to move
    // +2

    nsInt32 iNew = m_Index.ConvertTo<nsInt32>();
    nsInt32 iOld = m_OldIndex.ConvertTo<nsInt32>();

    if (iNew < iOld)
    {
      FinalOldPosition = iOld + 1;
    }
  }

  NS_SUCCEED_OR_RETURN(pDocument->GetObjectManager()->CanMove(m_pObject, m_pOldParent, m_sOldParentProperty, FinalOldPosition));

  pDocument->GetObjectManager()->MoveObject(m_pObject, m_pOldParent, m_sOldParentProperty, FinalOldPosition);

  return nsStatus(NS_SUCCESS);
}


////////////////////////////////////////////////////////////////////////
// nsSetObjectPropertyCommand
////////////////////////////////////////////////////////////////////////

nsSetObjectPropertyCommand::nsSetObjectPropertyCommand()
{
  m_pObject = nullptr;
}

nsStatus nsSetObjectPropertyCommand::DoInternal(bool bRedo)
{
  nsDocument* pDocument = GetDocument();

  if (!bRedo)
  {
    NS_ASSERT_DEBUG(m_NewValue.GetType() != nsVariantType::StringView && m_NewValue.GetType() != nsVariantType::TypedPointer, "Variants that are stored in the command history must hold ownership of their value.");

    if (m_Object.IsValid())
    {
      m_pObject = pDocument->GetObjectManager()->GetObject(m_Object);
      if (m_pObject == nullptr)
        return nsStatus("Set Property: The given object does not exist!");
    }
    else
      return nsStatus("Set Property: The given object does not exist!");

    nsIReflectedTypeAccessor& accessor0 = m_pObject->GetTypeAccessor();

    nsStatus res;
    m_OldValue = accessor0.GetValue(m_sProperty, m_Index, &res);
    if (res.Failed())
      return res;
    const nsAbstractProperty* pProp = accessor0.GetType()->FindPropertyByName(m_sProperty);
    if (pProp == nullptr)
      return nsStatus(nsFmt("Set Property: The property '{0}' does not exist", m_sProperty));

    if (pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner))
    {
      return nsStatus(nsFmt("Set Property: The property '{0}' is a PointerOwner, use nsAddObjectCommand instead", m_sProperty));
    }

    if (pProp->GetAttributeByType<nsTemporaryAttribute>())
    {
      // if we modify a 'temporary' property, ie. one that is not serialized,
      // don't mark the document as modified
      m_bModifiedDocument = false;
    }
  }

  return pDocument->GetObjectManager()->SetValue(m_pObject, m_sProperty, m_NewValue, m_Index);
}

nsStatus nsSetObjectPropertyCommand::UndoInternal(bool bFireEvents)
{
  if (bFireEvents)
  {
    return GetDocument()->GetObjectManager()->SetValue(m_pObject, m_sProperty, m_OldValue, m_Index);
  }
  else
  {
    nsIReflectedTypeAccessor& accessor = m_pObject->GetTypeAccessor();
    if (!accessor.SetValue(m_sProperty, m_OldValue, m_Index))
    {
      return nsStatus(nsFmt("Set Property: The property '{0}' does not exist", m_sProperty));
    }
  }
  return nsStatus(NS_SUCCESS);
}

////////////////////////////////////////////////////////////////////////
// nsSetObjectPropertyCommand
////////////////////////////////////////////////////////////////////////

nsResizeAndSetObjectPropertyCommand::nsResizeAndSetObjectPropertyCommand()
{
  m_pObject = nullptr;
}

nsStatus nsResizeAndSetObjectPropertyCommand::DoInternal(bool bRedo)
{
  nsDocument* pDocument = GetDocument();

  if (!bRedo)
  {
    if (m_Object.IsValid())
    {
      m_pObject = pDocument->GetObjectManager()->GetObject(m_Object);
      if (m_pObject == nullptr)
        return nsStatus("Set Property: The given object does not exist!");
    }
    else
      return nsStatus("Set Property: The given object does not exist!");

    const nsInt32 uiIndex = m_Index.ConvertTo<nsInt32>();

    nsIReflectedTypeAccessor& accessor0 = m_pObject->GetTypeAccessor();

    const nsInt32 iCount = accessor0.GetCount(m_sProperty);

    for (nsInt32 i = iCount; i <= uiIndex; ++i)
    {
      nsInsertObjectPropertyCommand ins;
      ins.m_Object = m_Object;
      ins.m_sProperty = m_sProperty;
      ins.m_Index = i;
      ins.m_NewValue = nsReflectionUtils::GetDefaultVariantFromType(m_NewValue.GetType());

      AddSubCommand(ins).AssertSuccess();
    }

    nsSetObjectPropertyCommand set;
    set.m_sProperty = m_sProperty;
    set.m_Index = m_Index;
    set.m_NewValue = m_NewValue;
    set.m_Object = m_Object;

    AddSubCommand(set).AssertSuccess();
  }

  return nsStatus(NS_SUCCESS);
}

////////////////////////////////////////////////////////////////////////
// nsInsertObjectPropertyCommand
////////////////////////////////////////////////////////////////////////

nsInsertObjectPropertyCommand::nsInsertObjectPropertyCommand()
{
  m_pObject = nullptr;
}

nsStatus nsInsertObjectPropertyCommand::DoInternal(bool bRedo)
{
  nsDocument* pDocument = GetDocument();

  if (!bRedo)
  {
    if (m_Object.IsValid())
    {
      m_pObject = pDocument->GetObjectManager()->GetObject(m_Object);
      if (m_pObject == nullptr)
        return nsStatus("Insert Property: The given object does not exist!");
    }
    else
      return nsStatus("Insert Property: The given object does not exist!");

    if (m_Index.CanConvertTo<nsInt32>() && m_Index.ConvertTo<nsInt32>() == -1)
    {
      nsIReflectedTypeAccessor& accessor = m_pObject->GetTypeAccessor();
      m_Index = accessor.GetCount(m_sProperty.GetData());
    }
  }

  return pDocument->GetObjectManager()->InsertValue(m_pObject, m_sProperty, m_NewValue, m_Index);
}

nsStatus nsInsertObjectPropertyCommand::UndoInternal(bool bFireEvents)
{
  if (bFireEvents)
  {
    return GetDocument()->GetObjectManager()->RemoveValue(m_pObject, m_sProperty, m_Index);
  }
  else
  {
    nsIReflectedTypeAccessor& accessor = m_pObject->GetTypeAccessor();
    if (!accessor.RemoveValue(m_sProperty, m_Index))
    {
      return nsStatus(nsFmt("Insert Property: The property '{0}' does not exist", m_sProperty));
    }
  }

  return nsStatus(NS_SUCCESS);
}


////////////////////////////////////////////////////////////////////////
// nsRemoveObjectPropertyCommand
////////////////////////////////////////////////////////////////////////

nsRemoveObjectPropertyCommand::nsRemoveObjectPropertyCommand()
{
  m_pObject = nullptr;
}

nsStatus nsRemoveObjectPropertyCommand::DoInternal(bool bRedo)
{
  nsDocument* pDocument = GetDocument();

  if (!bRedo)
  {
    if (m_Object.IsValid())
    {
      m_pObject = pDocument->GetObjectManager()->GetObject(m_Object);
      if (m_pObject == nullptr)
        return nsStatus("Remove Property: The given object does not exist!");
      nsStatus res;
      m_OldValue = m_pObject->GetTypeAccessor().GetValue(m_sProperty, m_Index, &res);
      if (res.Failed())
        return res;
    }
    else
      return nsStatus("Remove Property: The given object does not exist!");
  }

  return pDocument->GetObjectManager()->RemoveValue(m_pObject, m_sProperty, m_Index);
}

nsStatus nsRemoveObjectPropertyCommand::UndoInternal(bool bFireEvents)
{
  if (bFireEvents)
  {
    return GetDocument()->GetObjectManager()->InsertValue(m_pObject, m_sProperty, m_OldValue, m_Index);
  }
  else
  {
    nsIReflectedTypeAccessor& accessor = m_pObject->GetTypeAccessor();
    if (!accessor.InsertValue(m_sProperty, m_Index, m_OldValue))
    {
      return nsStatus(nsFmt("Remove Property: Undo failed! The index '{0}' in property '{1}' does not exist", m_Index.ConvertTo<nsString>(), m_sProperty));
    }
  }
  return nsStatus(NS_SUCCESS);
}


////////////////////////////////////////////////////////////////////////
// nsMoveObjectPropertyCommand
////////////////////////////////////////////////////////////////////////

nsMoveObjectPropertyCommand::nsMoveObjectPropertyCommand()
{
  m_pObject = nullptr;
}

nsStatus nsMoveObjectPropertyCommand::DoInternal(bool bRedo)
{
  nsDocument* pDocument = GetDocument();

  if (!bRedo)
  {
    m_pObject = pDocument->GetObjectManager()->GetObject(m_Object);
    if (m_pObject == nullptr)
      return nsStatus("Move Property: The given object does not exist.");
  }

  return GetDocument()->GetObjectManager()->MoveValue(m_pObject, m_sProperty, m_OldIndex, m_NewIndex);
}

nsStatus nsMoveObjectPropertyCommand::UndoInternal(bool bFireEvents)
{
  NS_ASSERT_DEV(bFireEvents, "This command does not support temporary commands");

  nsVariant FinalOldPosition = m_OldIndex;
  nsVariant FinalNewPosition = m_NewIndex;

  if (m_OldIndex.CanConvertTo<nsInt32>())
  {
    // If we are moving an object downwards, we must move by more than 1 (+1 would be behind the same object, which is still the same
    // position) so an object must always be moved by at least +2 moving UP can be done by -1, so when we undo that, we must ensure to move
    // +2

    nsInt32 iNew = m_NewIndex.ConvertTo<nsInt32>();
    nsInt32 iOld = m_OldIndex.ConvertTo<nsInt32>();

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
