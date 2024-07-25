#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <ToolsFoundation/Object/DocumentObjectBase.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>

nsIReflectedTypeAccessor& nsDocumentObject::GetTypeAccessor()
{
  const nsDocumentObject* pMe = this;
  return const_cast<nsIReflectedTypeAccessor&>(pMe->GetTypeAccessor());
}

nsUInt32 nsDocumentObject::GetChildIndex(const nsDocumentObject* pChild) const
{
  return m_Children.IndexOf(const_cast<nsDocumentObject*>(pChild));
}

void nsDocumentObject::InsertSubObject(nsDocumentObject* pObject, nsStringView sProperty, const nsVariant& index)
{
  NS_ASSERT_DEV(pObject != nullptr, "");
  NS_ASSERT_DEV(!sProperty.IsEmpty(), "Child objects must have a parent property to insert into");
  nsIReflectedTypeAccessor& accessor = GetTypeAccessor();

  const nsRTTI* pType = accessor.GetType();
  auto* pProp = pType->FindPropertyByName(sProperty);
  NS_ASSERT_DEV(pProp && pProp->GetFlags().IsSet(nsPropertyFlags::Class) &&
                  (!pProp->GetFlags().IsSet(nsPropertyFlags::Pointer) || pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner)),
    "Only class type or pointer to class type that own the object can be inserted, everything else is handled by value.");

  if (pProp->GetCategory() == nsPropertyCategory::Array || pProp->GetCategory() == nsPropertyCategory::Set)
  {
    if (!index.IsValid() || (index.CanConvertTo<nsInt32>() && index.ConvertTo<nsInt32>() == -1))
    {
      nsVariant newIndex = accessor.GetCount(sProperty);
      bool bRes = accessor.InsertValue(sProperty, newIndex, pObject->GetGuid());
      NS_ASSERT_DEV(bRes, "");
    }
    else
    {
      bool bRes = accessor.InsertValue(sProperty, index, pObject->GetGuid());
      NS_ASSERT_DEV(bRes, "");
    }
  }
  else if (pProp->GetCategory() == nsPropertyCategory::Map)
  {
    NS_ASSERT_DEV(index.IsA<nsString>(), "Map key must be a string.");
    bool bRes = accessor.InsertValue(sProperty, index, pObject->GetGuid());
    NS_ASSERT_DEV(bRes, "");
  }
  else if (pProp->GetCategory() == nsPropertyCategory::Member)
  {
    bool bRes = accessor.SetValue(sProperty, pObject->GetGuid());
    NS_ASSERT_DEV(bRes, "");
  }

  // Object patching
  pObject->m_sParentProperty = sProperty;
  pObject->m_pParent = this;
  m_Children.PushBack(pObject);
}

void nsDocumentObject::RemoveSubObject(nsDocumentObject* pObject)
{
  NS_ASSERT_DEV(pObject != nullptr, "");
  NS_ASSERT_DEV(!pObject->m_sParentProperty.IsEmpty(), "");
  NS_ASSERT_DEV(this == pObject->m_pParent, "");
  nsIReflectedTypeAccessor& accessor = GetTypeAccessor();

  // Property patching
  const nsRTTI* pType = accessor.GetType();
  auto* pProp = pType->FindPropertyByName(pObject->m_sParentProperty);
  if (pProp->GetCategory() == nsPropertyCategory::Array || pProp->GetCategory() == nsPropertyCategory::Set ||
      pProp->GetCategory() == nsPropertyCategory::Map)
  {
    nsVariant index = accessor.GetPropertyChildIndex(pObject->m_sParentProperty, pObject->GetGuid());
    bool bRes = accessor.RemoveValue(pObject->m_sParentProperty, index);
    NS_ASSERT_DEV(bRes, "");
  }
  else if (pProp->GetCategory() == nsPropertyCategory::Member)
  {
    bool bRes = accessor.SetValue(pObject->m_sParentProperty, nsUuid());
    NS_ASSERT_DEV(bRes, "");
  }

  m_Children.RemoveAndCopy(pObject);
  pObject->m_pParent = nullptr;
}

void nsDocumentObject::ComputeObjectHash(nsUInt64& ref_uiHash) const
{
  const nsIReflectedTypeAccessor& acc = GetTypeAccessor();
  auto pType = acc.GetType();

  ref_uiHash = nsHashingUtils::xxHash64(&m_Guid, sizeof(nsUuid), ref_uiHash);
  HashPropertiesRecursive(acc, ref_uiHash, pType);
}


nsDocumentObject* nsDocumentObject::GetChild(const nsUuid& guid)
{
  for (auto* pChild : m_Children)
  {
    if (pChild->GetGuid() == guid)
      return pChild;
  }
  return nullptr;
}


const nsDocumentObject* nsDocumentObject::GetChild(const nsUuid& guid) const
{
  for (auto* pChild : m_Children)
  {
    if (pChild->GetGuid() == guid)
      return pChild;
  }
  return nullptr;
}

const nsAbstractProperty* nsDocumentObject::GetParentPropertyType() const
{
  if (!m_pParent)
    return nullptr;
  const nsIReflectedTypeAccessor& accessor = m_pParent->GetTypeAccessor();
  const nsRTTI* pType = accessor.GetType();
  return pType->FindPropertyByName(m_sParentProperty);
}

nsVariant nsDocumentObject::GetPropertyIndex() const
{
  if (m_pParent == nullptr)
    return nsVariant();
  const nsIReflectedTypeAccessor& accessor = m_pParent->GetTypeAccessor();
  return accessor.GetPropertyChildIndex(m_sParentProperty.GetData(), GetGuid());
}

bool nsDocumentObject::IsOnHeap() const
{
  /// \todo Christopher: This crashes when the pointer is nullptr, which appears to be possible
  /// It happened for me when duplicating (CTRL+D) 2 objects 2 times then moving them and finally undoing everything
  NS_ASSERT_DEV(m_pParent != nullptr,
    "Object being modified is not part of the document, e.g. may be in the undo stack instead. "
    "This could happen if within an undo / redo op some callback tries to create a new undo scope / update prefabs etc.");

  if (GetParent() == GetDocumentObjectManager()->GetRootObject())
    return true;

  auto* pProp = GetParentPropertyType();
  return pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner);
}


void nsDocumentObject::HashPropertiesRecursive(const nsIReflectedTypeAccessor& acc, nsUInt64& uiHash, const nsRTTI* pType) const
{
  // Parse parent class
  const nsRTTI* pParentType = pType->GetParentType();
  if (pParentType != nullptr)
    HashPropertiesRecursive(acc, uiHash, pParentType);

  // Parse properties
  nsUInt32 uiPropertyCount = pType->GetProperties().GetCount();
  for (nsUInt32 i = 0; i < uiPropertyCount; ++i)
  {
    const nsAbstractProperty* pProperty = pType->GetProperties()[i];

    if (pProperty->GetFlags().IsSet(nsPropertyFlags::ReadOnly))
      continue;
    if (pProperty->GetAttributeByType<nsTemporaryAttribute>() != nullptr)
      continue;

    if (pProperty->GetCategory() == nsPropertyCategory::Member)
    {
      const nsVariant var = acc.GetValue(pProperty->GetPropertyName());
      uiHash = var.ComputeHash(uiHash);
    }
    else if (pProperty->GetCategory() == nsPropertyCategory::Array || pProperty->GetCategory() == nsPropertyCategory::Set)
    {
      nsHybridArray<nsVariant, 16> keys;
      acc.GetValues(pProperty->GetPropertyName(), keys);
      for (const nsVariant& var : keys)
      {
        uiHash = var.ComputeHash(uiHash);
      }
    }
    else if (pProperty->GetCategory() == nsPropertyCategory::Map)
    {
      nsHybridArray<nsVariant, 16> keys;
      acc.GetKeys(pProperty->GetPropertyName(), keys);
      keys.Sort([](const nsVariant& a, const nsVariant& b)
        { return a.Get<nsString>().Compare(b.Get<nsString>()) < 0; });
      for (const nsVariant& key : keys)
      {
        uiHash = key.ComputeHash(uiHash);
        nsVariant value = acc.GetValue(pProperty->GetPropertyName(), key);
        uiHash = value.ComputeHash(uiHash);
      }
    }
  }
}
