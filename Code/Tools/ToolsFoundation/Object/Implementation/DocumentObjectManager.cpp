#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Types/VariantTypeRegistry.h>
#include <ToolsFoundation/Document/Document.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>

////////////////////////////////////////////////////////////////////////
// wdDocumentObjectManager
////////////////////////////////////////////////////////////////////////

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdDocumentRoot, 1, wdRTTINoAllocator)
{
  WD_BEGIN_PROPERTIES
  {
    WD_ARRAY_MEMBER_PROPERTY("Children", m_RootObjects)->AddFlags(wdPropertyFlags::PointerOwner),
    WD_ARRAY_MEMBER_PROPERTY("TempObjects", m_TempObjects)->AddFlags(wdPropertyFlags::PointerOwner)->AddAttributes(new wdTemporaryAttribute()),
  }
  WD_END_PROPERTIES;
}
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

void wdDocumentRootObject::InsertSubObject(wdDocumentObject* pObject, const char* szProperty, const wdVariant& index)
{
  if (wdStringUtils::IsNullOrEmpty(szProperty))
    szProperty = "Children";
  return wdDocumentObject::InsertSubObject(pObject, szProperty, index);
}

void wdDocumentRootObject::RemoveSubObject(wdDocumentObject* pObject)
{
  return wdDocumentObject::RemoveSubObject(pObject);
}

wdVariant wdDocumentObjectPropertyEvent::getInsertIndex() const
{
  if (m_EventType == Type::PropertyMoved)
  {
    const wdIReflectedTypeAccessor& accessor = m_pObject->GetTypeAccessor();
    const wdRTTI* pType = accessor.GetType();
    auto* pProp = pType->FindPropertyByName(m_sProperty);
    if (pProp->GetCategory() == wdPropertyCategory::Array || pProp->GetCategory() == wdPropertyCategory::Set)
    {
      wdInt32 iCurrentIndex = m_OldIndex.ConvertTo<wdInt32>();
      wdInt32 iNewIndex = m_NewIndex.ConvertTo<wdInt32>();
      // Move after oneself?
      if (iNewIndex > iCurrentIndex)
      {
        iNewIndex -= 1;
        return wdVariant(iNewIndex);
      }
    }
  }
  return m_NewIndex;
}

wdDocumentObjectManager::Storage::Storage(const wdRTTI* pRootType)
  : m_RootObject(pRootType)
{
}

wdDocumentObjectManager::wdDocumentObjectManager(const wdRTTI* pRootType)
{
  auto pStorage = WD_DEFAULT_NEW(Storage, pRootType);
  pStorage->m_RootObject.m_pDocumentObjectManager = this;
  SwapStorage(pStorage);
}

wdDocumentObjectManager::~wdDocumentObjectManager()
{
  if (m_pObjectStorage->GetRefCount() == 1)
  {
    WD_ASSERT_DEV(m_pObjectStorage->m_GuidToObject.IsEmpty(), "Not all objects have been destroyed!");
  }
}

////////////////////////////////////////////////////////////////////////
// wdDocumentObjectManager Object Construction / Destruction
////////////////////////////////////////////////////////////////////////

wdDocumentObject* wdDocumentObjectManager::CreateObject(const wdRTTI* pRtti, wdUuid guid)
{
  WD_ASSERT_DEV(pRtti != nullptr, "Unknown RTTI type");

  wdDocumentObject* pObject = InternalCreateObject(pRtti);
  // In case the storage is swapped, objects should still be created in their original document manager.
  pObject->m_pDocumentObjectManager = m_pObjectStorage->m_RootObject.GetDocumentObjectManager();

  if (guid.IsValid())
    pObject->m_Guid = guid;
  else
    pObject->m_Guid.CreateNewUuid();

  PatchEmbeddedClassObjectsInternal(pObject, pRtti, false);

  wdDocumentObjectEvent e;
  e.m_pObject = pObject;
  e.m_EventType = wdDocumentObjectEvent::Type::AfterObjectCreated;
  m_pObjectStorage->m_ObjectEvents.Broadcast(e);

  return pObject;
}

void wdDocumentObjectManager::DestroyObject(wdDocumentObject* pObject)
{
  for (wdDocumentObject* pChild : pObject->m_Children)
  {
    DestroyObject(pChild);
  }

  wdDocumentObjectEvent e;
  e.m_pObject = pObject;
  e.m_EventType = wdDocumentObjectEvent::Type::BeforeObjectDestroyed;
  m_pObjectStorage->m_ObjectEvents.Broadcast(e);

  InternalDestroyObject(pObject);
}

void wdDocumentObjectManager::DestroyAllObjects()
{
  for (auto child : m_pObjectStorage->m_RootObject.m_Children)
  {
    DestroyObject(child);
  }

  m_pObjectStorage->m_RootObject.m_Children.Clear();
  m_pObjectStorage->m_GuidToObject.Clear();
}

void wdDocumentObjectManager::PatchEmbeddedClassObjects(const wdDocumentObject* pObject) const
{
  // Functional should be callable from anywhere but will of course have side effects.
  const_cast<wdDocumentObjectManager*>(this)->PatchEmbeddedClassObjectsInternal(
    const_cast<wdDocumentObject*>(pObject), pObject->GetTypeAccessor().GetType(), true);
}

const wdDocumentObject* wdDocumentObjectManager::GetObject(const wdUuid& guid) const
{
  const wdDocumentObject* pObject = nullptr;
  if (m_pObjectStorage->m_GuidToObject.TryGetValue(guid, pObject))
  {
    return pObject;
  }
  else if (guid == m_pObjectStorage->m_RootObject.GetGuid())
    return &m_pObjectStorage->m_RootObject;
  return nullptr;
}

wdDocumentObject* wdDocumentObjectManager::GetObject(const wdUuid& guid)
{
  return const_cast<wdDocumentObject*>(((const wdDocumentObjectManager*)this)->GetObject(guid));
}

////////////////////////////////////////////////////////////////////////
// wdDocumentObjectManager Property Change
////////////////////////////////////////////////////////////////////////

wdStatus wdDocumentObjectManager::SetValue(wdDocumentObject* pObject, const char* szProperty, const wdVariant& newValue, wdVariant index)
{
  WD_ASSERT_DEBUG(pObject, "Object must not be null.");
  wdIReflectedTypeAccessor& accessor = pObject->GetTypeAccessor();
  wdVariant oldValue = accessor.GetValue(szProperty, index);

  if (!accessor.SetValue(szProperty, newValue, index))
  {
    return wdStatus(wdFmt("Set Property: The property '{0}' does not exist or value type does not match", szProperty));
  }

  wdDocumentObjectPropertyEvent e;
  e.m_EventType = wdDocumentObjectPropertyEvent::Type::PropertySet;
  e.m_pObject = pObject;
  e.m_OldValue = oldValue;
  e.m_NewValue = newValue;
  e.m_sProperty = szProperty;
  e.m_NewIndex = index;

  // Allow a recursion depth of 2 for property setters. This allowed for two levels of side-effects on property setters.
  m_pObjectStorage->m_PropertyEvents.Broadcast(e, 2);
  return wdStatus(WD_SUCCESS);
}

wdStatus wdDocumentObjectManager::InsertValue(wdDocumentObject* pObject, const char* szProperty, const wdVariant& newValue, wdVariant index)
{
  wdIReflectedTypeAccessor& accessor = pObject->GetTypeAccessor();
  if (!accessor.InsertValue(szProperty, index, newValue))
  {
    if (!accessor.GetType()->FindPropertyByName(szProperty))
    {
      return wdStatus(wdFmt("Insert Property: The property '{0}' does not exist", szProperty));
    }
    return wdStatus(wdFmt("Insert Property: The property '{0}' already has the key '{1}'", szProperty, index));
  }

  wdDocumentObjectPropertyEvent e;
  e.m_EventType = wdDocumentObjectPropertyEvent::Type::PropertyInserted;
  e.m_pObject = pObject;
  e.m_NewValue = newValue;
  e.m_NewIndex = index;
  e.m_sProperty = szProperty;

  m_pObjectStorage->m_PropertyEvents.Broadcast(e);

  return wdStatus(WD_SUCCESS);
}

wdStatus wdDocumentObjectManager::RemoveValue(wdDocumentObject* pObject, const char* szProperty, wdVariant index)
{
  wdIReflectedTypeAccessor& accessor = pObject->GetTypeAccessor();
  wdVariant oldValue = accessor.GetValue(szProperty, index);

  if (!accessor.RemoveValue(szProperty, index))
  {
    return wdStatus(wdFmt("Remove Property: The index '{0}' in property '{1}' does not exist!", index.ConvertTo<wdString>(), szProperty));
  }

  wdDocumentObjectPropertyEvent e;
  e.m_EventType = wdDocumentObjectPropertyEvent::Type::PropertyRemoved;
  e.m_pObject = pObject;
  e.m_OldValue = oldValue;
  e.m_OldIndex = index;
  e.m_sProperty = szProperty;

  m_pObjectStorage->m_PropertyEvents.Broadcast(e);

  return wdStatus(WD_SUCCESS);
}

wdStatus wdDocumentObjectManager::MoveValue(wdDocumentObject* pObject, const char* szProperty, const wdVariant& oldIndex, const wdVariant& newIndex)
{
  if (!oldIndex.CanConvertTo<wdInt32>() || !newIndex.CanConvertTo<wdInt32>())
    return wdStatus("Move Property: Invalid indices provided.");

  wdIReflectedTypeAccessor& accessor = pObject->GetTypeAccessor();
  wdInt32 iCount = accessor.GetCount(szProperty);
  if (iCount < 0)
    return wdStatus("Move Property: Invalid property.");
  if (oldIndex.ConvertTo<wdInt32>() < 0 || oldIndex.ConvertTo<wdInt32>() >= iCount)
    return wdStatus(wdFmt("Move Property: Invalid old index '{0}'.", oldIndex.ConvertTo<wdInt32>()));
  if (newIndex.ConvertTo<wdInt32>() < 0 || newIndex.ConvertTo<wdInt32>() > iCount)
    return wdStatus(wdFmt("Move Property: Invalid new index '{0}'.", newIndex.ConvertTo<wdInt32>()));

  if (!accessor.MoveValue(szProperty, oldIndex, newIndex))
    return wdStatus("Move Property: Move value failed.");

  {
    wdDocumentObjectPropertyEvent e;
    e.m_EventType = wdDocumentObjectPropertyEvent::Type::PropertyMoved;
    e.m_pObject = pObject;
    e.m_OldIndex = oldIndex;
    e.m_NewIndex = newIndex;
    e.m_sProperty = szProperty;
    e.m_NewValue = accessor.GetValue(szProperty, e.getInsertIndex());
    WD_ASSERT_DEV(e.m_NewValue.IsValid(), "Value at new pos should be valid now, index missmatch?");
    m_pObjectStorage->m_PropertyEvents.Broadcast(e);
  }

  return wdStatus(WD_SUCCESS);
}

////////////////////////////////////////////////////////////////////////
// wdDocumentObjectManager Structure Change
////////////////////////////////////////////////////////////////////////

void wdDocumentObjectManager::AddObject(wdDocumentObject* pObject, wdDocumentObject* pParent, const char* szParentProperty, wdVariant index)
{
  if (pParent == nullptr)
    pParent = &m_pObjectStorage->m_RootObject;
  if (pParent == &m_pObjectStorage->m_RootObject && wdStringUtils::IsNullOrEmpty(szParentProperty))
    szParentProperty = "Children";

  WD_ASSERT_DEV(pObject->GetGuid().IsValid(), "Object Guid invalid! Object was not created via an wdObjectManagerBase!");
  WD_ASSERT_DEV(
    CanAdd(pObject->GetTypeAccessor().GetType(), pParent, szParentProperty, index).m_Result.Succeeded(), "Trying to execute invalid add!");

  InternalAddObject(pObject, pParent, szParentProperty, index);
}

void wdDocumentObjectManager::RemoveObject(wdDocumentObject* pObject)
{
  WD_ASSERT_DEV(CanRemove(pObject).m_Result.Succeeded(), "Trying to execute invalid remove!");
  InternalRemoveObject(pObject);
}

void wdDocumentObjectManager::MoveObject(wdDocumentObject* pObject, wdDocumentObject* pNewParent, const char* szParentProperty, wdVariant index)
{
  WD_ASSERT_DEV(CanMove(pObject, pNewParent, szParentProperty, index).m_Result.Succeeded(), "Trying to execute invalid move!");

  InternalMoveObject(pNewParent, pObject, szParentProperty, index);
}


////////////////////////////////////////////////////////////////////////
// wdDocumentObjectManager Structure Change Test
////////////////////////////////////////////////////////////////////////

wdStatus wdDocumentObjectManager::CanAdd(
  const wdRTTI* pRtti, const wdDocumentObject* pParent, const char* szParentProperty, const wdVariant& index) const
{
  // Test whether parent exists in tree.
  if (pParent == GetRootObject())
    pParent = nullptr;

  if (pParent != nullptr)
  {
    const wdDocumentObject* pObjectInTree = GetObject(pParent->GetGuid());
    WD_ASSERT_DEV(pObjectInTree == pParent, "Tree Corruption!!!");
    if (pObjectInTree == nullptr)
      return wdStatus("Parent is not part of the object manager!");

    const wdIReflectedTypeAccessor& accessor = pParent->GetTypeAccessor();
    const wdRTTI* pType = accessor.GetType();
    auto* pProp = pType->FindPropertyByName(szParentProperty);
    if (pProp == nullptr)
      return wdStatus(wdFmt("Property '{0}' could not be found in type '{1}'", szParentProperty, pType->GetTypeName()));

    const bool bIsValueType = wdReflectionUtils::IsValueType(pProp);

    if (bIsValueType || pProp->GetFlags().IsAnySet(wdPropertyFlags::IsEnum | wdPropertyFlags::Bitflags))
    {
      return wdStatus("Need to use 'InsertValue' action instead.");
    }
    else if (pProp->GetFlags().IsSet(wdPropertyFlags::Class))
    {
      if (pProp->GetFlags().IsSet(wdPropertyFlags::Pointer))
      {
        if (!pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner))
          return wdStatus(wdFmt("Cannot add object to the pointer property '{0}' as it does not hold ownership.", szParentProperty));

        if (!pRtti->IsDerivedFrom(pProp->GetSpecificType()))
          return wdStatus(wdFmt("Cannot add object to the pointer property '{0}' as its type '{1}' is not derived from the property type '{2}'!",
            szParentProperty, pRtti->GetTypeName(), pProp->GetSpecificType()->GetTypeName()));
      }
      else
      {
        if (pRtti != pProp->GetSpecificType())
          return wdStatus(wdFmt("Cannot add object to the property '{0}' as its type '{1}' does not match the property type '{2}'!", szParentProperty,
            pRtti->GetTypeName(), pProp->GetSpecificType()->GetTypeName()));
      }
    }

    if (pProp->GetCategory() == wdPropertyCategory::Array || pProp->GetCategory() == wdPropertyCategory::Set)
    {
      wdInt32 iCount = accessor.GetCount(szParentProperty);
      if (!index.CanConvertTo<wdInt32>())
      {
        return wdStatus(wdFmt("Cannot add object to the property '{0}', the given index is an invalid wdVariant (Either use '-1' to append "
                              "or a valid index).",
          szParentProperty));
      }
      wdInt32 iNewIndex = index.ConvertTo<wdInt32>();
      if (iNewIndex > (wdInt32)iCount)
        return wdStatus(wdFmt(
          "Cannot add object to its new location '{0}' is out of the bounds of the parent's property range '{1}'!", iNewIndex, (wdInt32)iCount));
      if (iNewIndex < 0 && iNewIndex != -1)
        return wdStatus(wdFmt("Cannot add object to the property '{0}', the index '{1}' is not valid (Either use '-1' to append or a valid index).",
          szParentProperty, iNewIndex));
    }
    if (pProp->GetCategory() == wdPropertyCategory::Map)
    {
      if (!index.IsA<wdString>())
        return wdStatus(wdFmt("Cannot add object to the map property '{0}' as its index type is not a string.", szParentProperty));
      wdVariant value = accessor.GetValue(szParentProperty, index);
      if (value.IsValid() && value.IsA<wdUuid>())
      {
        wdUuid guid = value.Get<wdUuid>();
        if (guid.IsValid())
          return wdStatus(
            wdFmt("Cannot add object to the map property '{0}' at key '{1}'. Delete old value first.", szParentProperty, index.Get<wdString>()));
      }
    }
    else if (pProp->GetCategory() == wdPropertyCategory::Member)
    {
      if (!pProp->GetFlags().IsSet(wdPropertyFlags::Pointer))
        return wdStatus("Embedded classes cannot be changed manually.");

      wdVariant value = accessor.GetValue(szParentProperty);
      if (!value.IsA<wdUuid>())
        return wdStatus("Property is not a pointer and thus can't be added to.");

      if (value.Get<wdUuid>().IsValid())
        return wdStatus("Can't set pointer if it already has a value, need to delete value first.");
    }
  }

  return InternalCanAdd(pRtti, pParent, szParentProperty, index);
}

wdStatus wdDocumentObjectManager::CanRemove(const wdDocumentObject* pObject) const
{
  const wdDocumentObject* pObjectInTree = GetObject(pObject->GetGuid());

  if (pObjectInTree == nullptr)
    return wdStatus("Object is not part of the object manager!");

  if (pObject->GetParent())
  {
    wdAbstractProperty* pProp = pObject->GetParentPropertyType();
    WD_ASSERT_DEV(pProp != nullptr, "Parent property should always be valid!");
    if (pProp->GetCategory() == wdPropertyCategory::Member && !pProp->GetFlags().IsSet(wdPropertyFlags::Pointer))
      return wdStatus("Non pointer members can't be deleted!");
  }
  WD_ASSERT_DEV(pObjectInTree == pObject, "Tree Corruption!!!");

  return InternalCanRemove(pObject);
}

wdStatus wdDocumentObjectManager::CanMove(
  const wdDocumentObject* pObject, const wdDocumentObject* pNewParent, const char* szParentProperty, const wdVariant& index) const
{
  WD_SUCCEED_OR_RETURN(CanAdd(pObject->GetTypeAccessor().GetType(), pNewParent, szParentProperty, index));

  WD_SUCCEED_OR_RETURN(CanRemove(pObject));

  if (pNewParent == nullptr)
    pNewParent = GetRootObject();

  if (pObject == pNewParent)
    return wdStatus("Can't move object onto itself!");

  const wdDocumentObject* pObjectInTree = GetObject(pObject->GetGuid());

  if (pObjectInTree == nullptr)
    return wdStatus("Object is not part of the object manager!");

  WD_ASSERT_DEV(pObjectInTree == pObject, "Tree Corruption!!!");

  if (pNewParent != GetRootObject())
  {
    const wdDocumentObject* pNewParentInTree = GetObject(pNewParent->GetGuid());

    if (pNewParentInTree == nullptr)
      return wdStatus("New parent is not part of the object manager!");

    WD_ASSERT_DEV(pNewParentInTree == pNewParent, "Tree Corruption!!!");
  }

  const wdDocumentObject* pCurParent = pNewParent->GetParent();

  while (pCurParent)
  {
    if (pCurParent == pObject)
      return wdStatus("Can't move object to one of its children!");

    pCurParent = pCurParent->GetParent();
  }

  const wdIReflectedTypeAccessor& accessor = pNewParent->GetTypeAccessor();
  const wdRTTI* pType = accessor.GetType();

  auto* pProp = pType->FindPropertyByName(szParentProperty);

  if (pProp == nullptr)
    return wdStatus(wdFmt("Property '{0}' could not be found in type '{1}'", szParentProperty, pType->GetTypeName()));

  if (pProp->GetCategory() == wdPropertyCategory::Array || pProp->GetCategory() == wdPropertyCategory::Set)
  {
    wdInt32 iChildIndex = index.ConvertTo<wdInt32>();
    if (iChildIndex == -1)
    {
      iChildIndex = pNewParent->GetTypeAccessor().GetCount(szParentProperty);
    }

    if (pNewParent == pObject->GetParent())
    {
      // Test whether we are moving before or after ourselves, both of which are not allowed and would not change the tree.
      wdIReflectedTypeAccessor& oldAccessor = pObject->m_pParent->GetTypeAccessor();
      wdInt32 iCurrentIndex = oldAccessor.GetPropertyChildIndex(szParentProperty, pObject->GetGuid()).ConvertTo<wdInt32>();
      if (iChildIndex == iCurrentIndex || iChildIndex == iCurrentIndex + 1)
        return wdStatus("Can't move object onto itself!");
    }
  }
  if (pProp->GetCategory() == wdPropertyCategory::Map)
  {
    if (!index.IsA<wdString>())
      return wdStatus(wdFmt("Cannot add object to the map property '{0}' as its index type is not a string.", szParentProperty));
    wdVariant value = accessor.GetValue(szParentProperty, index);
    if (value.IsValid() && value.IsA<wdUuid>())
    {
      wdUuid guid = value.Get<wdUuid>();
      if (guid.IsValid())
        return wdStatus(
          wdFmt("Cannot add object to the map property '{0}' at key '{1}'. Delete old value first.", szParentProperty, index.Get<wdString>()));
    }
  }

  if (pNewParent == GetRootObject())
    pNewParent = nullptr;

  return InternalCanMove(pObject, pNewParent, szParentProperty, index);
}

wdStatus wdDocumentObjectManager::CanSelect(const wdDocumentObject* pObject) const
{
  WD_ASSERT_DEV(pObject != nullptr, "pObject must be valid");

  const wdDocumentObject* pOwnObject = GetObject(pObject->GetGuid());
  if (pOwnObject == nullptr)
    return wdStatus(
      wdFmt("Object of type '{0}' is not part of the document and can't be selected", pObject->GetTypeAccessor().GetType()->GetTypeName()));

  return InternalCanSelect(pObject);
}


bool wdDocumentObjectManager::IsUnderRootProperty(const char* szRootProperty, const wdDocumentObject* pObject) const
{
  WD_ASSERT_DEBUG(m_pObjectStorage->m_RootObject.GetDocumentObjectManager() == pObject->GetDocumentObjectManager(), "Passed in object does not belong to this object manager.");
  while (pObject->GetParent() != GetRootObject())
  {
    pObject = pObject->GetParent();
  }
  return wdStringUtils::IsEqual(pObject->GetParentProperty(), szRootProperty);
}


bool wdDocumentObjectManager::IsUnderRootProperty(const char* szRootProperty, const wdDocumentObject* pParent, const char* szParentProperty) const
{
  WD_ASSERT_DEBUG(pParent == nullptr || m_pObjectStorage->m_RootObject.GetDocumentObjectManager() == pParent->GetDocumentObjectManager(), "Passed in object does not belong to this object manager.");
  if (pParent == nullptr || pParent == GetRootObject())
  {
    return wdStringUtils::IsEqual(szParentProperty, szRootProperty);
  }
  return IsUnderRootProperty(szRootProperty, pParent);
}

wdSharedPtr<wdDocumentObjectManager::Storage> wdDocumentObjectManager::SwapStorage(wdSharedPtr<wdDocumentObjectManager::Storage> pNewStorage)
{
  WD_ASSERT_ALWAYS(pNewStorage != nullptr, "Need a valid history storage object");

  auto retVal = m_pObjectStorage;

  m_StructureEventsUnsubscriber.Unsubscribe();
  m_PropertyEventsUnsubscriber.Unsubscribe();
  m_ObjectEventsUnsubscriber.Unsubscribe();

  m_pObjectStorage = pNewStorage;

  m_pObjectStorage->m_StructureEvents.AddEventHandler([this](const wdDocumentObjectStructureEvent& e) { m_StructureEvents.Broadcast(e); }, m_StructureEventsUnsubscriber);
  m_pObjectStorage->m_PropertyEvents.AddEventHandler([this](const wdDocumentObjectPropertyEvent& e) { m_PropertyEvents.Broadcast(e, 2); }, m_PropertyEventsUnsubscriber);
  m_pObjectStorage->m_ObjectEvents.AddEventHandler([this](const wdDocumentObjectEvent& e) { m_ObjectEvents.Broadcast(e); }, m_ObjectEventsUnsubscriber);

  return retVal;
}

////////////////////////////////////////////////////////////////////////
// wdDocumentObjectManager Private Functions
////////////////////////////////////////////////////////////////////////

void wdDocumentObjectManager::InternalAddObject(wdDocumentObject* pObject, wdDocumentObject* pParent, const char* szParentProperty, wdVariant index)
{
  wdDocumentObjectStructureEvent e;
  e.m_pDocument = m_pObjectStorage->m_pDocument;
  e.m_EventType = wdDocumentObjectStructureEvent::Type::BeforeObjectAdded;
  e.m_pObject = pObject;
  e.m_pPreviousParent = nullptr;
  e.m_pNewParent = pParent;
  e.m_sParentProperty = szParentProperty;
  e.m_NewPropertyIndex = index;

  if (e.m_NewPropertyIndex.CanConvertTo<wdInt32>() && e.m_NewPropertyIndex.ConvertTo<wdInt32>() == -1)
  {
    wdIReflectedTypeAccessor& accessor = pParent->GetTypeAccessor();
    e.m_NewPropertyIndex = accessor.GetCount(szParentProperty);
  }
  m_pObjectStorage->m_StructureEvents.Broadcast(e);

  pParent->InsertSubObject(pObject, szParentProperty, e.m_NewPropertyIndex);
  RecursiveAddGuids(pObject);

  e.m_EventType = wdDocumentObjectStructureEvent::Type::AfterObjectAdded;
  m_pObjectStorage->m_StructureEvents.Broadcast(e);
}

void wdDocumentObjectManager::InternalRemoveObject(wdDocumentObject* pObject)
{
  wdDocumentObjectStructureEvent e;
  e.m_pDocument = m_pObjectStorage->m_pDocument;
  e.m_EventType = wdDocumentObjectStructureEvent::Type::BeforeObjectRemoved;
  e.m_pObject = pObject;
  e.m_pPreviousParent = pObject->m_pParent;
  e.m_pNewParent = nullptr;
  e.m_sParentProperty = pObject->m_sParentProperty;
  e.m_OldPropertyIndex = pObject->GetPropertyIndex();
  m_pObjectStorage->m_StructureEvents.Broadcast(e);

  pObject->m_pParent->RemoveSubObject(pObject);
  RecursiveRemoveGuids(pObject);

  e.m_EventType = wdDocumentObjectStructureEvent::Type::AfterObjectRemoved;
  m_pObjectStorage->m_StructureEvents.Broadcast(e);
}

void wdDocumentObjectManager::InternalMoveObject(
  wdDocumentObject* pNewParent, wdDocumentObject* pObject, const char* szParentProperty, wdVariant index)
{
  if (pNewParent == nullptr)
    pNewParent = &m_pObjectStorage->m_RootObject;

  wdDocumentObjectStructureEvent e;
  e.m_pDocument = m_pObjectStorage->m_pDocument;
  e.m_EventType = wdDocumentObjectStructureEvent::Type::BeforeObjectMoved;
  e.m_pObject = pObject;
  e.m_pPreviousParent = pObject->m_pParent;
  e.m_pNewParent = pNewParent;
  e.m_sParentProperty = szParentProperty;
  e.m_OldPropertyIndex = pObject->GetPropertyIndex();
  e.m_NewPropertyIndex = index;
  if (e.m_NewPropertyIndex.CanConvertTo<wdInt32>() && e.m_NewPropertyIndex.ConvertTo<wdInt32>() == -1)
  {
    wdIReflectedTypeAccessor& accessor = pNewParent->GetTypeAccessor();
    e.m_NewPropertyIndex = accessor.GetCount(szParentProperty);
  }

  m_pObjectStorage->m_StructureEvents.Broadcast(e);

  wdVariant newIndex = e.getInsertIndex();

  pObject->m_pParent->RemoveSubObject(pObject);
  pNewParent->InsertSubObject(pObject, szParentProperty, newIndex);

  e.m_EventType = wdDocumentObjectStructureEvent::Type::AfterObjectMoved;
  m_pObjectStorage->m_StructureEvents.Broadcast(e);

  e.m_EventType = wdDocumentObjectStructureEvent::Type::AfterObjectMoved2;
  m_pObjectStorage->m_StructureEvents.Broadcast(e);
}

void wdDocumentObjectManager::RecursiveAddGuids(wdDocumentObject* pObject)
{
  m_pObjectStorage->m_GuidToObject[pObject->m_Guid] = pObject;

  for (wdUInt32 c = 0; c < pObject->GetChildren().GetCount(); ++c)
    RecursiveAddGuids(pObject->GetChildren()[c]);
}

void wdDocumentObjectManager::RecursiveRemoveGuids(wdDocumentObject* pObject)
{
  m_pObjectStorage->m_GuidToObject.Remove(pObject->m_Guid);

  for (wdUInt32 c = 0; c < pObject->GetChildren().GetCount(); ++c)
    RecursiveRemoveGuids(pObject->GetChildren()[c]);
}

void wdDocumentObjectManager::PatchEmbeddedClassObjectsInternal(wdDocumentObject* pObject, const wdRTTI* pType, bool addToDoc)
{
  const wdRTTI* pParent = pType->GetParentType();
  if (pParent != nullptr)
    PatchEmbeddedClassObjectsInternal(pObject, pParent, addToDoc);

  wdIReflectedTypeAccessor& accessor = pObject->GetTypeAccessor();
  const wdUInt32 uiPropertyCount = pType->GetProperties().GetCount();
  for (wdUInt32 i = 0; i < uiPropertyCount; ++i)
  {
    const wdAbstractProperty* pProperty = pType->GetProperties()[i];
    const wdVariantTypeInfo* pInfo = wdVariantTypeRegistry::GetSingleton()->FindVariantTypeInfo(pProperty->GetSpecificType());

    if (pProperty->GetCategory() == wdPropertyCategory::Member && pProperty->GetFlags().IsSet(wdPropertyFlags::Class) && !pInfo &&
        !pProperty->GetFlags().IsSet(wdPropertyFlags::Pointer))
    {
      wdUuid value = accessor.GetValue(pProperty->GetPropertyName()).Get<wdUuid>();
      WD_ASSERT_DEV(addToDoc || !value.IsValid(), "If addToDoc is false, the current value must be invalid!");
      if (value.IsValid())
      {
        wdDocumentObject* pEmbeddedObject = GetObject(value);
        if (pEmbeddedObject)
        {
          if (pEmbeddedObject->GetTypeAccessor().GetType() == pProperty->GetSpecificType())
            continue;
          else
          {
            // Type mismatch, delete old.
            InternalRemoveObject(pEmbeddedObject);
          }
        }
      }

      // Create new
      wdStringBuilder sTemp;
      wdConversionUtils::ToString(pObject->GetGuid(), sTemp);
      sTemp.Append("/", pProperty->GetPropertyName());
      const wdUuid subObjectGuid = wdUuid::StableUuidForString(sTemp);
      wdDocumentObject* pEmbeddedObject = CreateObject(pProperty->GetSpecificType(), subObjectGuid);
      if (addToDoc)
      {
        InternalAddObject(pEmbeddedObject, pObject, pProperty->GetPropertyName(), wdVariant());
      }
      else
      {
        pObject->InsertSubObject(pEmbeddedObject, pProperty->GetPropertyName(), wdVariant());
      }
    }
  }
}


const wdAbstractProperty* wdDocumentObjectStructureEvent::GetProperty() const
{
  return m_pObject->GetParentPropertyType();
}

wdVariant wdDocumentObjectStructureEvent::getInsertIndex() const
{
  if ((m_EventType == Type::BeforeObjectMoved || m_EventType == Type::AfterObjectMoved || m_EventType == Type::AfterObjectMoved2) &&
      m_pNewParent == m_pPreviousParent)
  {
    const wdIReflectedTypeAccessor& accessor = m_pPreviousParent->GetTypeAccessor();
    const wdRTTI* pType = accessor.GetType();
    auto* pProp = pType->FindPropertyByName(m_sParentProperty);
    if (pProp->GetCategory() == wdPropertyCategory::Array || pProp->GetCategory() == wdPropertyCategory::Set)
    {
      wdInt32 iCurrentIndex = m_OldPropertyIndex.ConvertTo<wdInt32>();
      wdInt32 iNewIndex = m_NewPropertyIndex.ConvertTo<wdInt32>();
      // Move after oneself?
      if (iNewIndex > iCurrentIndex)
      {
        iNewIndex -= 1;
        return wdVariant(iNewIndex);
      }
    }
  }
  return m_NewPropertyIndex;
}
