#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Types/VariantTypeRegistry.h>
#include <ToolsFoundation/Document/Document.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>

////////////////////////////////////////////////////////////////////////
// nsDocumentObjectManager
////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsDocumentRoot, 1, nsRTTINoAllocator)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ARRAY_MEMBER_PROPERTY("Children", m_RootObjects)->AddFlags(nsPropertyFlags::PointerOwner),
    NS_ARRAY_MEMBER_PROPERTY("TempObjects", m_TempObjects)->AddFlags(nsPropertyFlags::PointerOwner)->AddAttributes(new nsTemporaryAttribute()),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

void nsDocumentRootObject::InsertSubObject(nsDocumentObject* pObject, nsStringView sProperty, const nsVariant& index)
{
  if (sProperty.IsEmpty())
    sProperty = "Children";
  return nsDocumentObject::InsertSubObject(pObject, sProperty, index);
}

void nsDocumentRootObject::RemoveSubObject(nsDocumentObject* pObject)
{
  return nsDocumentObject::RemoveSubObject(pObject);
}

nsVariant nsDocumentObjectPropertyEvent::getInsertIndex() const
{
  if (m_EventType == Type::PropertyMoved)
  {
    const nsIReflectedTypeAccessor& accessor = m_pObject->GetTypeAccessor();
    const nsRTTI* pType = accessor.GetType();
    auto* pProp = pType->FindPropertyByName(m_sProperty);
    if (pProp->GetCategory() == nsPropertyCategory::Array || pProp->GetCategory() == nsPropertyCategory::Set)
    {
      nsInt32 iCurrentIndex = m_OldIndex.ConvertTo<nsInt32>();
      nsInt32 iNewIndex = m_NewIndex.ConvertTo<nsInt32>();
      // Move after oneself?
      if (iNewIndex > iCurrentIndex)
      {
        iNewIndex -= 1;
        return nsVariant(iNewIndex);
      }
    }
  }
  return m_NewIndex;
}

nsDocumentObjectManager::Storage::Storage(const nsRTTI* pRootType)
  : m_RootObject(pRootType)
{
}

nsDocumentObjectManager::nsDocumentObjectManager(const nsRTTI* pRootType)
{
  auto pStorage = NS_DEFAULT_NEW(Storage, pRootType);
  pStorage->m_RootObject.m_pDocumentObjectManager = this;
  SwapStorage(pStorage);
}

nsDocumentObjectManager::~nsDocumentObjectManager()
{
  if (m_pObjectStorage->GetRefCount() == 1)
  {
    NS_ASSERT_DEV(m_pObjectStorage->m_GuidToObject.IsEmpty(), "Not all objects have been destroyed!");
  }
}

////////////////////////////////////////////////////////////////////////
// nsDocumentObjectManager Object Construction / Destruction
////////////////////////////////////////////////////////////////////////

nsDocumentObject* nsDocumentObjectManager::CreateObject(const nsRTTI* pRtti, nsUuid guid)
{
  NS_ASSERT_DEV(pRtti != nullptr, "Unknown RTTI type");

  nsDocumentObject* pObject = InternalCreateObject(pRtti);
  // In case the storage is swapped, objects should still be created in their original document manager.
  pObject->m_pDocumentObjectManager = m_pObjectStorage->m_RootObject.GetDocumentObjectManager();

  if (guid.IsValid())
    pObject->m_Guid = guid;
  else
    pObject->m_Guid = nsUuid::MakeUuid();

  PatchEmbeddedClassObjectsInternal(pObject, pRtti, false);

  nsDocumentObjectEvent e;
  e.m_pObject = pObject;
  e.m_EventType = nsDocumentObjectEvent::Type::AfterObjectCreated;
  m_pObjectStorage->m_ObjectEvents.Broadcast(e);

  return pObject;
}

void nsDocumentObjectManager::DestroyObject(nsDocumentObject* pObject)
{
  for (nsDocumentObject* pChild : pObject->m_Children)
  {
    DestroyObject(pChild);
  }

  nsDocumentObjectEvent e;
  e.m_pObject = pObject;
  e.m_EventType = nsDocumentObjectEvent::Type::BeforeObjectDestroyed;
  m_pObjectStorage->m_ObjectEvents.Broadcast(e);

  InternalDestroyObject(pObject);
}

void nsDocumentObjectManager::DestroyAllObjects()
{
  for (auto child : m_pObjectStorage->m_RootObject.m_Children)
  {
    DestroyObject(child);
  }

  m_pObjectStorage->m_RootObject.m_Children.Clear();
  m_pObjectStorage->m_GuidToObject.Clear();
}

void nsDocumentObjectManager::PatchEmbeddedClassObjects(const nsDocumentObject* pObject) const
{
  // Functional should be callable from anywhere but will of course have side effects.
  const_cast<nsDocumentObjectManager*>(this)->PatchEmbeddedClassObjectsInternal(
    const_cast<nsDocumentObject*>(pObject), pObject->GetTypeAccessor().GetType(), true);
}

const nsDocumentObject* nsDocumentObjectManager::GetObject(const nsUuid& guid) const
{
  const nsDocumentObject* pObject = nullptr;
  if (m_pObjectStorage->m_GuidToObject.TryGetValue(guid, pObject))
  {
    return pObject;
  }
  else if (guid == m_pObjectStorage->m_RootObject.GetGuid())
    return &m_pObjectStorage->m_RootObject;
  return nullptr;
}

nsDocumentObject* nsDocumentObjectManager::GetObject(const nsUuid& guid)
{
  return const_cast<nsDocumentObject*>(((const nsDocumentObjectManager*)this)->GetObject(guid));
}

////////////////////////////////////////////////////////////////////////
// nsDocumentObjectManager Property Change
////////////////////////////////////////////////////////////////////////

nsStatus nsDocumentObjectManager::SetValue(nsDocumentObject* pObject, nsStringView sProperty, const nsVariant& newValue, nsVariant index)
{
  NS_ASSERT_DEBUG(pObject, "Object must not be null.");
  nsIReflectedTypeAccessor& accessor = pObject->GetTypeAccessor();
  nsVariant oldValue = accessor.GetValue(sProperty, index);

  if (!accessor.SetValue(sProperty, newValue, index))
  {
    return nsStatus(nsFmt("Set Property: The property '{0}' does not exist or value type does not match", sProperty));
  }

  nsDocumentObjectPropertyEvent e;
  e.m_EventType = nsDocumentObjectPropertyEvent::Type::PropertySet;
  e.m_pObject = pObject;
  e.m_OldValue = oldValue;
  e.m_NewValue = newValue;
  e.m_sProperty = sProperty;
  e.m_NewIndex = index;

  // Allow a recursion depth of 2 for property setters. This allowed for two levels of side-effects on property setters.
  m_pObjectStorage->m_PropertyEvents.Broadcast(e, 2);
  return nsStatus(NS_SUCCESS);
}

nsStatus nsDocumentObjectManager::InsertValue(nsDocumentObject* pObject, nsStringView sProperty, const nsVariant& newValue, nsVariant index)
{
  nsIReflectedTypeAccessor& accessor = pObject->GetTypeAccessor();
  if (!accessor.InsertValue(sProperty, index, newValue))
  {
    if (!accessor.GetType()->FindPropertyByName(sProperty))
    {
      return nsStatus(nsFmt("Insert Property: The property '{0}' does not exist", sProperty));
    }
    return nsStatus(nsFmt("Insert Property: The property '{0}' already has the key '{1}'", sProperty, index));
  }

  nsDocumentObjectPropertyEvent e;
  e.m_EventType = nsDocumentObjectPropertyEvent::Type::PropertyInserted;
  e.m_pObject = pObject;
  e.m_NewValue = newValue;
  e.m_NewIndex = index;
  e.m_sProperty = sProperty;

  m_pObjectStorage->m_PropertyEvents.Broadcast(e);

  return nsStatus(NS_SUCCESS);
}

nsStatus nsDocumentObjectManager::RemoveValue(nsDocumentObject* pObject, nsStringView sProperty, nsVariant index)
{
  nsIReflectedTypeAccessor& accessor = pObject->GetTypeAccessor();
  nsVariant oldValue = accessor.GetValue(sProperty, index);

  if (!accessor.RemoveValue(sProperty, index))
  {
    return nsStatus(nsFmt("Remove Property: The index '{0}' in property '{1}' does not exist!", index.ConvertTo<nsString>(), sProperty));
  }

  nsDocumentObjectPropertyEvent e;
  e.m_EventType = nsDocumentObjectPropertyEvent::Type::PropertyRemoved;
  e.m_pObject = pObject;
  e.m_OldValue = oldValue;
  e.m_OldIndex = index;
  e.m_sProperty = sProperty;

  m_pObjectStorage->m_PropertyEvents.Broadcast(e);

  return nsStatus(NS_SUCCESS);
}

nsStatus nsDocumentObjectManager::MoveValue(nsDocumentObject* pObject, nsStringView sProperty, const nsVariant& oldIndex, const nsVariant& newIndex)
{
  if (!oldIndex.CanConvertTo<nsInt32>() || !newIndex.CanConvertTo<nsInt32>())
    return nsStatus("Move Property: Invalid indices provided.");

  nsIReflectedTypeAccessor& accessor = pObject->GetTypeAccessor();
  nsInt32 iCount = accessor.GetCount(sProperty);
  if (iCount < 0)
    return nsStatus("Move Property: Invalid property.");
  if (oldIndex.ConvertTo<nsInt32>() < 0 || oldIndex.ConvertTo<nsInt32>() >= iCount)
    return nsStatus(nsFmt("Move Property: Invalid old index '{0}'.", oldIndex.ConvertTo<nsInt32>()));
  if (newIndex.ConvertTo<nsInt32>() < 0 || newIndex.ConvertTo<nsInt32>() > iCount)
    return nsStatus(nsFmt("Move Property: Invalid new index '{0}'.", newIndex.ConvertTo<nsInt32>()));

  if (!accessor.MoveValue(sProperty, oldIndex, newIndex))
    return nsStatus("Move Property: Move value failed.");

  {
    nsDocumentObjectPropertyEvent e;
    e.m_EventType = nsDocumentObjectPropertyEvent::Type::PropertyMoved;
    e.m_pObject = pObject;
    e.m_OldIndex = oldIndex;
    e.m_NewIndex = newIndex;
    e.m_sProperty = sProperty;
    e.m_NewValue = accessor.GetValue(sProperty, e.getInsertIndex());
    // NewValue can be invalid if an invalid variant in a variant array is moved
    // NS_ASSERT_DEV(e.m_NewValue.IsValid(), "Value at new pos should be valid now, index missmatch?");
    m_pObjectStorage->m_PropertyEvents.Broadcast(e);
  }

  return nsStatus(NS_SUCCESS);
}

////////////////////////////////////////////////////////////////////////
// nsDocumentObjectManager Structure Change
////////////////////////////////////////////////////////////////////////

void nsDocumentObjectManager::AddObject(nsDocumentObject* pObject, nsDocumentObject* pParent, nsStringView sParentProperty, nsVariant index)
{
  if (pParent == nullptr)
    pParent = &m_pObjectStorage->m_RootObject;
  if (pParent == &m_pObjectStorage->m_RootObject && sParentProperty.IsEmpty())
    sParentProperty = "Children";

  NS_ASSERT_DEV(pObject->GetGuid().IsValid(), "Object Guid invalid! Object was not created via an nsObjectManagerBase!");
  NS_ASSERT_DEV(
    CanAdd(pObject->GetTypeAccessor().GetType(), pParent, sParentProperty, index).m_Result.Succeeded(), "Trying to execute invalid add!");

  InternalAddObject(pObject, pParent, sParentProperty, index);
}

void nsDocumentObjectManager::RemoveObject(nsDocumentObject* pObject)
{
  NS_ASSERT_DEV(CanRemove(pObject).m_Result.Succeeded(), "Trying to execute invalid remove!");
  InternalRemoveObject(pObject);
}

void nsDocumentObjectManager::MoveObject(nsDocumentObject* pObject, nsDocumentObject* pNewParent, nsStringView sParentProperty, nsVariant index)
{
  NS_ASSERT_DEV(CanMove(pObject, pNewParent, sParentProperty, index).m_Result.Succeeded(), "Trying to execute invalid move!");

  InternalMoveObject(pNewParent, pObject, sParentProperty, index);
}


////////////////////////////////////////////////////////////////////////
// nsDocumentObjectManager Structure Change Test
////////////////////////////////////////////////////////////////////////

nsStatus nsDocumentObjectManager::CanAdd(
  const nsRTTI* pRtti, const nsDocumentObject* pParent, nsStringView sParentProperty, const nsVariant& index) const
{
  // Test whether parent exists in tree.
  if (pParent == GetRootObject())
    pParent = nullptr;

  if (pParent != nullptr)
  {
    const nsDocumentObject* pObjectInTree = GetObject(pParent->GetGuid());
    NS_ASSERT_DEV(pObjectInTree == pParent, "Tree Corruption!!!");
    if (pObjectInTree == nullptr)
      return nsStatus("Parent is not part of the object manager!");

    const nsIReflectedTypeAccessor& accessor = pParent->GetTypeAccessor();
    const nsRTTI* pType = accessor.GetType();
    auto* pProp = pType->FindPropertyByName(sParentProperty);
    if (pProp == nullptr)
      return nsStatus(nsFmt("Property '{0}' could not be found in type '{1}'", sParentProperty, pType->GetTypeName()));

    const bool bIsValueType = nsReflectionUtils::IsValueType(pProp);

    if (bIsValueType || pProp->GetFlags().IsAnySet(nsPropertyFlags::IsEnum | nsPropertyFlags::Bitflags))
    {
      return nsStatus("Need to use 'InsertValue' action instead.");
    }
    else if (pProp->GetFlags().IsSet(nsPropertyFlags::Class))
    {
      if (pProp->GetFlags().IsSet(nsPropertyFlags::Pointer))
      {
        if (!pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner))
          return nsStatus(nsFmt("Cannot add object to the pointer property '{0}' as it does not hold ownership.", sParentProperty));

        if (!pRtti->IsDerivedFrom(pProp->GetSpecificType()))
          return nsStatus(nsFmt("Cannot add object to the pointer property '{0}' as its type '{1}' is not derived from the property type '{2}'!",
            sParentProperty, pRtti->GetTypeName(), pProp->GetSpecificType()->GetTypeName()));
      }
      else
      {
        if (pRtti != pProp->GetSpecificType())
          return nsStatus(nsFmt("Cannot add object to the property '{0}' as its type '{1}' does not match the property type '{2}'!", sParentProperty,
            pRtti->GetTypeName(), pProp->GetSpecificType()->GetTypeName()));
      }
    }

    if (pProp->GetCategory() == nsPropertyCategory::Array || pProp->GetCategory() == nsPropertyCategory::Set)
    {
      nsInt32 iCount = accessor.GetCount(sParentProperty);
      if (!index.CanConvertTo<nsInt32>())
      {
        return nsStatus(nsFmt("Cannot add object to the property '{0}', the given index is an invalid nsVariant (Either use '-1' to append "
                              "or a valid index).",
          sParentProperty));
      }
      nsInt32 iNewIndex = index.ConvertTo<nsInt32>();
      if (iNewIndex > (nsInt32)iCount)
        return nsStatus(nsFmt(
          "Cannot add object to its new location '{0}' is out of the bounds of the parent's property range '{1}'!", iNewIndex, (nsInt32)iCount));
      if (iNewIndex < 0 && iNewIndex != -1)
        return nsStatus(nsFmt("Cannot add object to the property '{0}', the index '{1}' is not valid (Either use '-1' to append or a valid index).",
          sParentProperty, iNewIndex));
    }
    if (pProp->GetCategory() == nsPropertyCategory::Map)
    {
      if (!index.IsA<nsString>())
        return nsStatus(nsFmt("Cannot add object to the map property '{0}' as its index type is not a string.", sParentProperty));
      nsVariant value = accessor.GetValue(sParentProperty, index);
      if (value.IsValid() && value.IsA<nsUuid>())
      {
        nsUuid guid = value.Get<nsUuid>();
        if (guid.IsValid())
          return nsStatus(
            nsFmt("Cannot add object to the map property '{0}' at key '{1}'. Delete old value first.", sParentProperty, index.Get<nsString>()));
      }
    }
    else if (pProp->GetCategory() == nsPropertyCategory::Member)
    {
      if (!pProp->GetFlags().IsSet(nsPropertyFlags::Pointer))
        return nsStatus("Embedded classes cannot be changed manually.");

      nsVariant value = accessor.GetValue(sParentProperty);
      if (!value.IsA<nsUuid>())
        return nsStatus("Property is not a pointer and thus can't be added to.");

      if (value.Get<nsUuid>().IsValid())
        return nsStatus("Can't set pointer if it already has a value, need to delete value first.");
    }
  }

  return InternalCanAdd(pRtti, pParent, sParentProperty, index);
}

nsStatus nsDocumentObjectManager::CanRemove(const nsDocumentObject* pObject) const
{
  const nsDocumentObject* pObjectInTree = GetObject(pObject->GetGuid());

  if (pObjectInTree == nullptr)
    return nsStatus("Object is not part of the object manager!");

  if (pObject->GetParent())
  {
    const nsAbstractProperty* pProp = pObject->GetParentPropertyType();
    NS_ASSERT_DEV(pProp != nullptr, "Parent property should always be valid!");
    if (pProp->GetCategory() == nsPropertyCategory::Member && !pProp->GetFlags().IsSet(nsPropertyFlags::Pointer))
      return nsStatus("Non pointer members can't be deleted!");
  }
  NS_ASSERT_DEV(pObjectInTree == pObject, "Tree Corruption!!!");

  return InternalCanRemove(pObject);
}

nsStatus nsDocumentObjectManager::CanMove(
  const nsDocumentObject* pObject, const nsDocumentObject* pNewParent, nsStringView sParentProperty, const nsVariant& index) const
{
  NS_SUCCEED_OR_RETURN(CanAdd(pObject->GetTypeAccessor().GetType(), pNewParent, sParentProperty, index));

  NS_SUCCEED_OR_RETURN(CanRemove(pObject));

  if (pNewParent == nullptr)
    pNewParent = GetRootObject();

  if (pObject == pNewParent)
    return nsStatus("Can't move object onto itself!");

  const nsDocumentObject* pObjectInTree = GetObject(pObject->GetGuid());

  if (pObjectInTree == nullptr)
    return nsStatus("Object is not part of the object manager!");

  NS_ASSERT_DEV(pObjectInTree == pObject, "Tree Corruption!!!");

  if (pNewParent != GetRootObject())
  {
    const nsDocumentObject* pNewParentInTree = GetObject(pNewParent->GetGuid());

    if (pNewParentInTree == nullptr)
      return nsStatus("New parent is not part of the object manager!");

    NS_ASSERT_DEV(pNewParentInTree == pNewParent, "Tree Corruption!!!");
  }

  const nsDocumentObject* pCurParent = pNewParent->GetParent();

  while (pCurParent)
  {
    if (pCurParent == pObject)
      return nsStatus("Can't move object to one of its children!");

    pCurParent = pCurParent->GetParent();
  }

  const nsIReflectedTypeAccessor& accessor = pNewParent->GetTypeAccessor();
  const nsRTTI* pType = accessor.GetType();

  auto* pProp = pType->FindPropertyByName(sParentProperty);

  if (pProp == nullptr)
    return nsStatus(nsFmt("Property '{0}' could not be found in type '{1}'", sParentProperty, pType->GetTypeName()));

  if (pProp->GetCategory() == nsPropertyCategory::Array || pProp->GetCategory() == nsPropertyCategory::Set)
  {
    nsInt32 iChildIndex = index.ConvertTo<nsInt32>();
    if (iChildIndex == -1)
    {
      iChildIndex = pNewParent->GetTypeAccessor().GetCount(sParentProperty);
    }

    if (pNewParent == pObject->GetParent())
    {
      // Test whether we are moving before or after ourselves, both of which are not allowed and would not change the tree.
      nsIReflectedTypeAccessor& oldAccessor = pObject->m_pParent->GetTypeAccessor();
      nsInt32 iCurrentIndex = oldAccessor.GetPropertyChildIndex(sParentProperty, pObject->GetGuid()).ConvertTo<nsInt32>();
      if (iChildIndex == iCurrentIndex || iChildIndex == iCurrentIndex + 1)
        return nsStatus("Can't move object onto itself!");
    }
  }
  if (pProp->GetCategory() == nsPropertyCategory::Map)
  {
    if (!index.IsA<nsString>())
      return nsStatus(nsFmt("Cannot add object to the map property '{0}' as its index type is not a string.", sParentProperty));
    nsVariant value = accessor.GetValue(sParentProperty, index);
    if (value.IsValid() && value.IsA<nsUuid>())
    {
      nsUuid guid = value.Get<nsUuid>();
      if (guid.IsValid())
        return nsStatus(
          nsFmt("Cannot add object to the map property '{0}' at key '{1}'. Delete old value first.", sParentProperty, index.Get<nsString>()));
    }
  }

  if (pNewParent == GetRootObject())
    pNewParent = nullptr;

  return InternalCanMove(pObject, pNewParent, sParentProperty, index);
}

nsStatus nsDocumentObjectManager::CanSelect(const nsDocumentObject* pObject) const
{
  NS_ASSERT_DEV(pObject != nullptr, "pObject must be valid");

  const nsDocumentObject* pOwnObject = GetObject(pObject->GetGuid());
  if (pOwnObject == nullptr)
    return nsStatus(
      nsFmt("Object of type '{0}' is not part of the document and can't be selected", pObject->GetTypeAccessor().GetType()->GetTypeName()));

  return InternalCanSelect(pObject);
}


bool nsDocumentObjectManager::IsUnderRootProperty(nsStringView sRootProperty, const nsDocumentObject* pObject) const
{
  NS_ASSERT_DEBUG(m_pObjectStorage->m_RootObject.GetDocumentObjectManager() == pObject->GetDocumentObjectManager(), "Passed in object does not belong to this object manager.");
  while (pObject->GetParent() != GetRootObject())
  {
    pObject = pObject->GetParent();
  }
  return sRootProperty == pObject->GetParentProperty();
}


bool nsDocumentObjectManager::IsUnderRootProperty(nsStringView sRootProperty, const nsDocumentObject* pParent, nsStringView sParentProperty) const
{
  NS_ASSERT_DEBUG(pParent == nullptr || m_pObjectStorage->m_RootObject.GetDocumentObjectManager() == pParent->GetDocumentObjectManager(), "Passed in object does not belong to this object manager.");
  if (pParent == nullptr || pParent == GetRootObject())
  {
    return sParentProperty == sRootProperty;
  }
  return IsUnderRootProperty(sRootProperty, pParent);
}

bool nsDocumentObjectManager::IsTemporary(const nsDocumentObject* pObject) const
{
  return IsUnderRootProperty("TempObjects", pObject);
}

bool nsDocumentObjectManager::IsTemporary(const nsDocumentObject* pParent, nsStringView sParentProperty) const
{
  return IsUnderRootProperty("TempObjects", pParent, sParentProperty);
}

nsSharedPtr<nsDocumentObjectManager::Storage> nsDocumentObjectManager::SwapStorage(nsSharedPtr<nsDocumentObjectManager::Storage> pNewStorage)
{
  NS_ASSERT_ALWAYS(pNewStorage != nullptr, "Need a valid history storage object");

  auto retVal = m_pObjectStorage;

  m_StructureEventsUnsubscriber.Unsubscribe();
  m_PropertyEventsUnsubscriber.Unsubscribe();
  m_ObjectEventsUnsubscriber.Unsubscribe();

  m_pObjectStorage = pNewStorage;

  m_pObjectStorage->m_StructureEvents.AddEventHandler([this](const nsDocumentObjectStructureEvent& e)
    { m_StructureEvents.Broadcast(e); }, m_StructureEventsUnsubscriber);
  m_pObjectStorage->m_PropertyEvents.AddEventHandler([this](const nsDocumentObjectPropertyEvent& e)
    { m_PropertyEvents.Broadcast(e, 2); }, m_PropertyEventsUnsubscriber);
  m_pObjectStorage->m_ObjectEvents.AddEventHandler([this](const nsDocumentObjectEvent& e)
    { m_ObjectEvents.Broadcast(e); }, m_ObjectEventsUnsubscriber);

  return retVal;
}

////////////////////////////////////////////////////////////////////////
// nsDocumentObjectManager Private Functions
////////////////////////////////////////////////////////////////////////

void nsDocumentObjectManager::InternalAddObject(nsDocumentObject* pObject, nsDocumentObject* pParent, nsStringView sParentProperty, nsVariant index)
{
  nsDocumentObjectStructureEvent e;
  e.m_pDocument = m_pObjectStorage->m_pDocument;
  e.m_EventType = nsDocumentObjectStructureEvent::Type::BeforeObjectAdded;
  e.m_pObject = pObject;
  e.m_pPreviousParent = nullptr;
  e.m_pNewParent = pParent;
  e.m_sParentProperty = sParentProperty;
  e.m_NewPropertyIndex = index;

  if (e.m_NewPropertyIndex.CanConvertTo<nsInt32>() && e.m_NewPropertyIndex.ConvertTo<nsInt32>() == -1)
  {
    nsIReflectedTypeAccessor& accessor = pParent->GetTypeAccessor();
    e.m_NewPropertyIndex = accessor.GetCount(sParentProperty);
  }
  m_pObjectStorage->m_StructureEvents.Broadcast(e);

  pParent->InsertSubObject(pObject, sParentProperty, e.m_NewPropertyIndex);
  RecursiveAddGuids(pObject);

  e.m_EventType = nsDocumentObjectStructureEvent::Type::AfterObjectAdded;
  m_pObjectStorage->m_StructureEvents.Broadcast(e);
}

void nsDocumentObjectManager::InternalRemoveObject(nsDocumentObject* pObject)
{
  nsDocumentObjectStructureEvent e;
  e.m_pDocument = m_pObjectStorage->m_pDocument;
  e.m_EventType = nsDocumentObjectStructureEvent::Type::BeforeObjectRemoved;
  e.m_pObject = pObject;
  e.m_pPreviousParent = pObject->m_pParent;
  e.m_pNewParent = nullptr;
  e.m_sParentProperty = pObject->m_sParentProperty;
  e.m_OldPropertyIndex = pObject->GetPropertyIndex();
  m_pObjectStorage->m_StructureEvents.Broadcast(e);

  pObject->m_pParent->RemoveSubObject(pObject);
  RecursiveRemoveGuids(pObject);

  e.m_EventType = nsDocumentObjectStructureEvent::Type::AfterObjectRemoved;
  m_pObjectStorage->m_StructureEvents.Broadcast(e);
}

void nsDocumentObjectManager::InternalMoveObject(
  nsDocumentObject* pNewParent, nsDocumentObject* pObject, nsStringView sParentProperty, nsVariant index)
{
  if (pNewParent == nullptr)
    pNewParent = &m_pObjectStorage->m_RootObject;

  nsDocumentObjectStructureEvent e;
  e.m_pDocument = m_pObjectStorage->m_pDocument;
  e.m_EventType = nsDocumentObjectStructureEvent::Type::BeforeObjectMoved;
  e.m_pObject = pObject;
  e.m_pPreviousParent = pObject->m_pParent;
  e.m_pNewParent = pNewParent;
  e.m_sParentProperty = sParentProperty;
  e.m_OldPropertyIndex = pObject->GetPropertyIndex();
  e.m_NewPropertyIndex = index;
  if (e.m_NewPropertyIndex.CanConvertTo<nsInt32>() && e.m_NewPropertyIndex.ConvertTo<nsInt32>() == -1)
  {
    nsIReflectedTypeAccessor& accessor = pNewParent->GetTypeAccessor();
    e.m_NewPropertyIndex = accessor.GetCount(sParentProperty);
  }

  m_pObjectStorage->m_StructureEvents.Broadcast(e);

  nsVariant newIndex = e.getInsertIndex();

  pObject->m_pParent->RemoveSubObject(pObject);
  pNewParent->InsertSubObject(pObject, sParentProperty, newIndex);

  e.m_EventType = nsDocumentObjectStructureEvent::Type::AfterObjectMoved;
  m_pObjectStorage->m_StructureEvents.Broadcast(e);

  e.m_EventType = nsDocumentObjectStructureEvent::Type::AfterObjectMoved2;
  m_pObjectStorage->m_StructureEvents.Broadcast(e);
}

void nsDocumentObjectManager::RecursiveAddGuids(nsDocumentObject* pObject)
{
  m_pObjectStorage->m_GuidToObject[pObject->m_Guid] = pObject;

  for (nsUInt32 c = 0; c < pObject->GetChildren().GetCount(); ++c)
    RecursiveAddGuids(pObject->GetChildren()[c]);
}

void nsDocumentObjectManager::RecursiveRemoveGuids(nsDocumentObject* pObject)
{
  m_pObjectStorage->m_GuidToObject.Remove(pObject->m_Guid);

  for (nsUInt32 c = 0; c < pObject->GetChildren().GetCount(); ++c)
    RecursiveRemoveGuids(pObject->GetChildren()[c]);
}

void nsDocumentObjectManager::PatchEmbeddedClassObjectsInternal(nsDocumentObject* pObject, const nsRTTI* pType, bool addToDoc)
{
  const nsRTTI* pParent = pType->GetParentType();
  if (pParent != nullptr)
    PatchEmbeddedClassObjectsInternal(pObject, pParent, addToDoc);

  nsIReflectedTypeAccessor& accessor = pObject->GetTypeAccessor();
  const nsUInt32 uiPropertyCount = pType->GetProperties().GetCount();
  for (nsUInt32 i = 0; i < uiPropertyCount; ++i)
  {
    const nsAbstractProperty* pProperty = pType->GetProperties()[i];
    const nsVariantTypeInfo* pInfo = nsVariantTypeRegistry::GetSingleton()->FindVariantTypeInfo(pProperty->GetSpecificType());

    if (pProperty->GetCategory() == nsPropertyCategory::Member && pProperty->GetFlags().IsSet(nsPropertyFlags::Class) && !pInfo &&
        !pProperty->GetFlags().IsSet(nsPropertyFlags::Pointer))
    {
      nsUuid value = accessor.GetValue(pProperty->GetPropertyName()).Get<nsUuid>();
      NS_ASSERT_DEV(addToDoc || !value.IsValid(), "If addToDoc is false, the current value must be invalid!");
      if (value.IsValid())
      {
        nsDocumentObject* pEmbeddedObject = GetObject(value);
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
      nsStringBuilder sTemp;
      nsConversionUtils::ToString(pObject->GetGuid(), sTemp);
      sTemp.Append("/", pProperty->GetPropertyName());
      const nsUuid subObjectGuid = nsUuid::MakeStableUuidFromString(sTemp);
      nsDocumentObject* pEmbeddedObject = CreateObject(pProperty->GetSpecificType(), subObjectGuid);
      if (addToDoc)
      {
        InternalAddObject(pEmbeddedObject, pObject, pProperty->GetPropertyName(), nsVariant());
      }
      else
      {
        pObject->InsertSubObject(pEmbeddedObject, pProperty->GetPropertyName(), nsVariant());
      }
    }
  }
}


const nsAbstractProperty* nsDocumentObjectStructureEvent::GetProperty() const
{
  return m_pObject->GetParentPropertyType();
}

nsVariant nsDocumentObjectStructureEvent::getInsertIndex() const
{
  if ((m_EventType == Type::BeforeObjectMoved || m_EventType == Type::AfterObjectMoved || m_EventType == Type::AfterObjectMoved2) &&
      m_pNewParent == m_pPreviousParent)
  {
    const nsIReflectedTypeAccessor& accessor = m_pPreviousParent->GetTypeAccessor();
    const nsRTTI* pType = accessor.GetType();
    auto* pProp = pType->FindPropertyByName(m_sParentProperty);
    if (pProp->GetCategory() == nsPropertyCategory::Array || pProp->GetCategory() == nsPropertyCategory::Set)
    {
      nsInt32 iCurrentIndex = m_OldPropertyIndex.ConvertTo<nsInt32>();
      nsInt32 iNewIndex = m_NewPropertyIndex.ConvertTo<nsInt32>();
      // Move after oneself?
      if (iNewIndex > iCurrentIndex)
      {
        iNewIndex -= 1;
        return nsVariant(iNewIndex);
      }
    }
  }
  return m_NewPropertyIndex;
}
