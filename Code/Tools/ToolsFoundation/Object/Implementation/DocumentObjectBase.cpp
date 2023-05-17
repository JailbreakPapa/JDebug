#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <ToolsFoundation/Object/DocumentObjectBase.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>

wdIReflectedTypeAccessor& wdDocumentObject::GetTypeAccessor()
{
  const wdDocumentObject* pMe = this;
  return const_cast<wdIReflectedTypeAccessor&>(pMe->GetTypeAccessor());
}

wdUInt32 wdDocumentObject::GetChildIndex(const wdDocumentObject* pChild) const
{
  return m_Children.IndexOf(const_cast<wdDocumentObject*>(pChild));
}

void wdDocumentObject::InsertSubObject(wdDocumentObject* pObject, const char* szProperty, const wdVariant& index)
{
  WD_ASSERT_DEV(pObject != nullptr, "");
  WD_ASSERT_DEV(!wdStringUtils::IsNullOrEmpty(szProperty), "Child objects must have a parent property to insert into");
  wdIReflectedTypeAccessor& accessor = GetTypeAccessor();

  const wdRTTI* pType = accessor.GetType();
  auto* pProp = pType->FindPropertyByName(szProperty);
  WD_ASSERT_DEV(pProp && pProp->GetFlags().IsSet(wdPropertyFlags::Class) &&
                  (!pProp->GetFlags().IsSet(wdPropertyFlags::Pointer) || pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner)),
    "Only class type or pointer to class type that own the object can be inserted, everything else is handled by value.");

  if (pProp->GetCategory() == wdPropertyCategory::Array || pProp->GetCategory() == wdPropertyCategory::Set)
  {
    if (!index.IsValid() || (index.CanConvertTo<wdInt32>() && index.ConvertTo<wdInt32>() == -1))
    {
      wdVariant newIndex = accessor.GetCount(szProperty);
      bool bRes = accessor.InsertValue(szProperty, newIndex, pObject->GetGuid());
      WD_ASSERT_DEV(bRes, "");
    }
    else
    {
      bool bRes = accessor.InsertValue(szProperty, index, pObject->GetGuid());
      WD_ASSERT_DEV(bRes, "");
    }
  }
  else if (pProp->GetCategory() == wdPropertyCategory::Map)
  {
    WD_ASSERT_DEV(index.IsA<wdString>(), "Map key must be a string.");
    bool bRes = accessor.InsertValue(szProperty, index, pObject->GetGuid());
    WD_ASSERT_DEV(bRes, "");
  }
  else if (pProp->GetCategory() == wdPropertyCategory::Member)
  {
    bool bRes = accessor.SetValue(szProperty, pObject->GetGuid());
    WD_ASSERT_DEV(bRes, "");
  }

  // Object patching
  pObject->m_sParentProperty = szProperty;
  pObject->m_pParent = this;
  m_Children.PushBack(pObject);
}

void wdDocumentObject::RemoveSubObject(wdDocumentObject* pObject)
{
  WD_ASSERT_DEV(pObject != nullptr, "");
  WD_ASSERT_DEV(!pObject->m_sParentProperty.IsEmpty(), "");
  WD_ASSERT_DEV(this == pObject->m_pParent, "");
  wdIReflectedTypeAccessor& accessor = GetTypeAccessor();

  // Property patching
  const wdRTTI* pType = accessor.GetType();
  auto* pProp = pType->FindPropertyByName(pObject->m_sParentProperty);
  if (pProp->GetCategory() == wdPropertyCategory::Array || pProp->GetCategory() == wdPropertyCategory::Set ||
      pProp->GetCategory() == wdPropertyCategory::Map)
  {
    wdVariant index = accessor.GetPropertyChildIndex(pObject->m_sParentProperty, pObject->GetGuid());
    bool bRes = accessor.RemoveValue(pObject->m_sParentProperty, index);
    WD_ASSERT_DEV(bRes, "");
  }
  else if (pProp->GetCategory() == wdPropertyCategory::Member)
  {
    bool bRes = accessor.SetValue(pObject->m_sParentProperty, wdUuid());
    WD_ASSERT_DEV(bRes, "");
  }

  m_Children.RemoveAndCopy(pObject);
  pObject->m_pParent = nullptr;
}

void wdDocumentObject::ComputeObjectHash(wdUInt64& ref_uiHash) const
{
  const wdIReflectedTypeAccessor& acc = GetTypeAccessor();
  auto pType = acc.GetType();

  ref_uiHash = wdHashingUtils::xxHash64(&m_Guid, sizeof(wdUuid), ref_uiHash);
  HashPropertiesRecursive(acc, ref_uiHash, pType);
}


wdDocumentObject* wdDocumentObject::GetChild(const wdUuid& guid)
{
  for (auto* pChild : m_Children)
  {
    if (pChild->GetGuid() == guid)
      return pChild;
  }
  return nullptr;
}


const wdDocumentObject* wdDocumentObject::GetChild(const wdUuid& guid) const
{
  for (auto* pChild : m_Children)
  {
    if (pChild->GetGuid() == guid)
      return pChild;
  }
  return nullptr;
}

wdAbstractProperty* wdDocumentObject::GetParentPropertyType() const
{
  if (!m_pParent)
    return nullptr;
  const wdIReflectedTypeAccessor& accessor = m_pParent->GetTypeAccessor();
  const wdRTTI* pType = accessor.GetType();
  return pType->FindPropertyByName(m_sParentProperty);
}

wdVariant wdDocumentObject::GetPropertyIndex() const
{
  if (m_pParent == nullptr)
    return wdVariant();
  const wdIReflectedTypeAccessor& accessor = m_pParent->GetTypeAccessor();
  return accessor.GetPropertyChildIndex(m_sParentProperty.GetData(), GetGuid());
}

bool wdDocumentObject::IsOnHeap() const
{
  /// \todo Christopher: This crashes when the pointer is nullptr, which appears to be possible
  /// It happened for me when duplicating (CTRL+D) 2 objects 2 times then moving them and finally undoing everything
  WD_ASSERT_DEV(m_pParent != nullptr,
    "Object being modified is not part of the document, e.g. may be in the undo stack instead. "
    "This could happen if within an undo / redo op some callback tries to create a new undo scope / update prefabs etc.");

  if (GetParent() == GetDocumentObjectManager()->GetRootObject())
    return true;

  auto* pProp = GetParentPropertyType();
  return pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner);
}


void wdDocumentObject::HashPropertiesRecursive(const wdIReflectedTypeAccessor& acc, wdUInt64& uiHash, const wdRTTI* pType) const
{
  // Parse parent class
  const wdRTTI* pParentType = pType->GetParentType();
  if (pParentType != nullptr)
    HashPropertiesRecursive(acc, uiHash, pParentType);

  // Parse properties
  wdUInt32 uiPropertyCount = pType->GetProperties().GetCount();
  for (wdUInt32 i = 0; i < uiPropertyCount; ++i)
  {
    const wdAbstractProperty* pProperty = pType->GetProperties()[i];

    if (pProperty->GetFlags().IsSet(wdPropertyFlags::ReadOnly))
      continue;
    if (pProperty->GetAttributeByType<wdTemporaryAttribute>() != nullptr)
      continue;

    if (pProperty->GetCategory() == wdPropertyCategory::Member)
    {
      const wdVariant var = acc.GetValue(pProperty->GetPropertyName());
      uiHash = var.ComputeHash(uiHash);
    }
    else if (pProperty->GetCategory() == wdPropertyCategory::Array || pProperty->GetCategory() == wdPropertyCategory::Set)
    {
      wdHybridArray<wdVariant, 16> keys;
      acc.GetValues(pProperty->GetPropertyName(), keys);
      for (const wdVariant& var : keys)
      {
        uiHash = var.ComputeHash(uiHash);
      }
    }
    else if (pProperty->GetCategory() == wdPropertyCategory::Map)
    {
      wdHybridArray<wdVariant, 16> keys;
      acc.GetKeys(pProperty->GetPropertyName(), keys);
      keys.Sort([](const wdVariant& a, const wdVariant& b) { return a.Get<wdString>().Compare(b.Get<wdString>()) < 0; });
      for (const wdVariant& key : keys)
      {
        uiHash = key.ComputeHash(uiHash);
        wdVariant value = acc.GetValue(pProperty->GetPropertyName(), key);
        uiHash = value.ComputeHash(uiHash);
      }
    }
  }
}
