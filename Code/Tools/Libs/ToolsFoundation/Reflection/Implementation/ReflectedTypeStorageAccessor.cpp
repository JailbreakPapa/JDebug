#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/Types/Status.h>
#include <Foundation/Types/VariantTypeRegistry.h>
#include <ToolsFoundation/Reflection/ReflectedTypeStorageAccessor.h>
#include <ToolsFoundation/Reflection/ToolsReflectionUtils.h>


////////////////////////////////////////////////////////////////////////
// nsReflectedTypeStorageAccessor public functions
////////////////////////////////////////////////////////////////////////

nsReflectedTypeStorageAccessor::nsReflectedTypeStorageAccessor(const nsRTTI* pRtti, nsDocumentObject* pOwner)
  : nsIReflectedTypeAccessor(pRtti, pOwner)
{
  const nsRTTI* pType = pRtti;
  NS_ASSERT_DEV(pType != nullptr, "Trying to construct an nsReflectedTypeStorageAccessor for an invalid type!");
  m_pMapping = nsReflectedTypeStorageManager::AddStorageAccessor(this);
  NS_ASSERT_DEV(m_pMapping != nullptr, "The type for this nsReflectedTypeStorageAccessor is unknown to the nsReflectedTypeStorageManager!");

  auto& indexTable = m_pMapping->m_PathToStorageInfoTable;
  const nsUInt32 uiProperties = indexTable.GetCount();
  // To prevent re-allocs due to new properties being added we reserve 20% more space.
  m_Data.Reserve(uiProperties + uiProperties / 20);
  m_Data.SetCount(uiProperties);

  // Fill data storage with default values for the given types.
  for (auto it = indexTable.GetIterator(); it.IsValid(); ++it)
  {
    const auto storageInfo = it.Value();
    m_Data[storageInfo.m_uiIndex] = storageInfo.m_DefaultValue;
  }
}

nsReflectedTypeStorageAccessor::~nsReflectedTypeStorageAccessor()
{
  nsReflectedTypeStorageManager::RemoveStorageAccessor(this);
}

const nsVariant nsReflectedTypeStorageAccessor::GetValue(nsStringView sProperty, nsVariant index, nsStatus* pRes) const
{
  const nsAbstractProperty* pProp = GetType()->FindPropertyByName(sProperty);
  if (pProp == nullptr)
  {
    if (pRes)
      *pRes = nsStatus(nsFmt("Property '{0}' not found in type '{1}'", sProperty, GetType()->GetTypeName()));
    return nsVariant();
  }

  if (pRes)
    *pRes = nsStatus(NS_SUCCESS);
  const nsReflectedTypeStorageManager::ReflectedTypeStorageMapping::StorageInfo* storageInfo = nullptr;
  if (m_pMapping->m_PathToStorageInfoTable.TryGetValue(sProperty, storageInfo))
  {
    switch (pProp->GetCategory())
    {
      case nsPropertyCategory::Member:
        return m_Data[storageInfo->m_uiIndex];
      case nsPropertyCategory::Array:
      case nsPropertyCategory::Set:
      {
        if (!index.IsValid())
        {
          return m_Data[storageInfo->m_uiIndex];
        }

        const nsVariantArray& values = m_Data[storageInfo->m_uiIndex].Get<nsVariantArray>();
        if (index.CanConvertTo<nsUInt32>())
        {
          nsUInt32 uiIndex = index.ConvertTo<nsUInt32>();
          if (uiIndex < values.GetCount())
          {
            return values[uiIndex];
          }
        }
        if (pRes)
          *pRes = nsStatus(nsFmt("Index '{0}' for property '{1}' is invalid or out of bounds.", index, sProperty));
      }
      break;
      case nsPropertyCategory::Map:
      {
        if (!index.IsValid())
        {
          return m_Data[storageInfo->m_uiIndex];
        }

        const nsVariantDictionary& values = m_Data[storageInfo->m_uiIndex].Get<nsVariantDictionary>();
        if (index.IsA<nsString>())
        {
          const nsString& sIndex = index.Get<nsString>();
          if (const nsVariant* pValue = values.GetValue(sIndex))
          {
            return *pValue;
          }
        }
        if (pRes)
          *pRes = nsStatus(nsFmt("Index '{0}' for property '{1}' is invalid or out of bounds.", index, sProperty));
      }
      break;
      default:
        break;
    }
  }
  return nsVariant();
}

bool nsReflectedTypeStorageAccessor::SetValue(nsStringView sProperty, const nsVariant& value, nsVariant index)
{
  const nsReflectedTypeStorageManager::ReflectedTypeStorageMapping::StorageInfo* storageInfo = nullptr;
  if (m_pMapping->m_PathToStorageInfoTable.TryGetValue(sProperty, storageInfo))
  {
    const nsAbstractProperty* pProp = GetType()->FindPropertyByName(sProperty);
    if (pProp == nullptr)
      return false;
    NS_ASSERT_DEV(pProp->GetSpecificType() == nsGetStaticRTTI<nsVariant>() || value.IsValid(), "");

    if (storageInfo->m_Type == nsVariantType::TypedObject && storageInfo->m_DefaultValue.GetReflectedType() != value.GetReflectedType())
    {
      // Typed objects must match exactly.
      return false;
    }

    const bool isValueType = nsReflectionUtils::IsValueType(pProp);
    const nsVariantType::Enum SpecVarType = pProp->GetFlags().IsSet(nsPropertyFlags::Pointer) || (pProp->GetFlags().IsSet(nsPropertyFlags::Class) && !isValueType) ? nsVariantType::Uuid : pProp->GetSpecificType()->GetVariantType();

    switch (pProp->GetCategory())
    {
      case nsPropertyCategory::Member:
      {
        if (value.IsA<nsString>() && pProp->GetFlags().IsAnySet(nsPropertyFlags::IsEnum | nsPropertyFlags::Bitflags))
        {
          nsInt64 iValue;
          nsReflectionUtils::StringToEnumeration(pProp->GetSpecificType(), value.Get<nsString>(), iValue);
          m_Data[storageInfo->m_uiIndex] = nsVariant(iValue).ConvertTo(storageInfo->m_Type);
          return true;
        }
        else if (pProp->GetSpecificType() == nsGetStaticRTTI<nsVariant>())
        {
          m_Data[storageInfo->m_uiIndex] = value;
          return true;
        }
        else if (value.CanConvertTo(storageInfo->m_Type))
        {
          // We are lenient here regarding the type, as we may have stored values in the undo-redo stack
          // that may have a different type now as someone reloaded the type information and replaced a type.
          m_Data[storageInfo->m_uiIndex] = value.ConvertTo(storageInfo->m_Type);
          return true;
        }
      }
      break;
      case nsPropertyCategory::Array:
      case nsPropertyCategory::Set:
      {
        const nsVariantArray& values = m_Data[storageInfo->m_uiIndex].Get<nsVariantArray>();
        if (index.CanConvertTo<nsUInt32>())
        {
          nsUInt32 uiIndex = index.ConvertTo<nsUInt32>();
          if (uiIndex < values.GetCount())
          {
            nsVariantArray changedValues = values;
            if (pProp->GetSpecificType() == nsGetStaticRTTI<nsVariant>())
            {
              changedValues[uiIndex] = value;
              m_Data[storageInfo->m_uiIndex] = changedValues;
              return true;
            }
            else
            {
              if (pProp->GetSpecificType() == nsGetStaticRTTI<nsVariant>())
              {
                m_Data[storageInfo->m_uiIndex] = value;
                return true;
              }
              else if (value.CanConvertTo(SpecVarType))
              {
                // We are lenient here regarding the type, as we may have stored values in the undo-redo stack
                // that may have a different type now as someone reloaded the type information and replaced a type.
                changedValues[uiIndex] = value.ConvertTo(SpecVarType);
                m_Data[storageInfo->m_uiIndex] = changedValues;
                return true;
              }
            }
          }
        }
      }
      break;
      case nsPropertyCategory::Map:
      {
        const nsVariantDictionary& values = m_Data[storageInfo->m_uiIndex].Get<nsVariantDictionary>();
        if (index.IsA<nsString>() && values.Contains(index.Get<nsString>()))
        {
          const nsString& sIndex = index.Get<nsString>();
          nsVariantDictionary changedValues = values;
          if (pProp->GetSpecificType() == nsGetStaticRTTI<nsVariant>())
          {
            changedValues[sIndex] = value;
            m_Data[storageInfo->m_uiIndex] = changedValues;
            return true;
          }
          else
          {
            if (value.CanConvertTo(SpecVarType))
            {
              // We are lenient here regarding the type, as we may have stored values in the undo-redo stack
              // that may have a different type now as someone reloaded the type information and replaced a type.
              changedValues[sIndex] = value.ConvertTo(SpecVarType);
              m_Data[storageInfo->m_uiIndex] = changedValues;
              return true;
            }
          }
        }
      }
      break;
      default:
        break;
    }
  }
  return false;
}

nsInt32 nsReflectedTypeStorageAccessor::GetCount(nsStringView sProperty) const
{
  const nsReflectedTypeStorageManager::ReflectedTypeStorageMapping::StorageInfo* storageInfo = nullptr;
  if (m_pMapping->m_PathToStorageInfoTable.TryGetValue(sProperty, storageInfo))
  {
    if (storageInfo->m_Type == nsVariant::Type::Invalid)
      return -1;

    const nsAbstractProperty* pProp = GetType()->FindPropertyByName(sProperty);
    if (pProp == nullptr)
      return -1;

    switch (pProp->GetCategory())
    {
      case nsPropertyCategory::Array:
      case nsPropertyCategory::Set:
      {
        const nsVariantArray& values = m_Data[storageInfo->m_uiIndex].Get<nsVariantArray>();
        return values.GetCount();
      }
      case nsPropertyCategory::Map:
      {
        const nsVariantDictionary& values = m_Data[storageInfo->m_uiIndex].Get<nsVariantDictionary>();
        return values.GetCount();
      }
      default:
        break;
    }
  }
  return -1;
}

bool nsReflectedTypeStorageAccessor::GetKeys(nsStringView sProperty, nsDynamicArray<nsVariant>& out_keys) const
{
  out_keys.Clear();

  const nsReflectedTypeStorageManager::ReflectedTypeStorageMapping::StorageInfo* storageInfo = nullptr;
  if (m_pMapping->m_PathToStorageInfoTable.TryGetValue(sProperty, storageInfo))
  {
    if (storageInfo->m_Type == nsVariant::Type::Invalid)
      return false;

    const nsAbstractProperty* pProp = GetType()->FindPropertyByName(sProperty);
    if (pProp == nullptr)
      return false;

    switch (pProp->GetCategory())
    {
      case nsPropertyCategory::Array:
      case nsPropertyCategory::Set:
      {
        const nsVariantArray& values = m_Data[storageInfo->m_uiIndex].Get<nsVariantArray>();
        out_keys.Reserve(values.GetCount());
        for (nsUInt32 i = 0; i < values.GetCount(); ++i)
        {
          out_keys.PushBack(i);
        }
        return true;
      }
      break;
      case nsPropertyCategory::Map:
      {
        const nsVariantDictionary& values = m_Data[storageInfo->m_uiIndex].Get<nsVariantDictionary>();
        out_keys.Reserve(values.GetCount());
        for (auto it = values.GetIterator(); it.IsValid(); ++it)
        {
          out_keys.PushBack(nsVariant(it.Key()));
        }
        return true;
      }
      break;

      default:
        break;
    }
  }
  return false;
}
bool nsReflectedTypeStorageAccessor::InsertValue(nsStringView sProperty, nsVariant index, const nsVariant& value)
{
  const nsReflectedTypeStorageManager::ReflectedTypeStorageMapping::StorageInfo* storageInfo = nullptr;
  if (m_pMapping->m_PathToStorageInfoTable.TryGetValue(sProperty, storageInfo))
  {
    if (storageInfo->m_Type == nsVariant::Type::Invalid)
      return false;

    const nsAbstractProperty* pProp = GetType()->FindPropertyByName(sProperty);
    if (pProp == nullptr)
      return false;

    if (storageInfo->m_Type == nsVariantType::TypedObject && storageInfo->m_DefaultValue.GetReflectedType() != value.GetReflectedType())
    {
      // Typed objects must match exactly.
      return false;
    }

    const bool isValueType = nsReflectionUtils::IsValueType(pProp);
    const nsVariantType::Enum SpecVarType = pProp->GetFlags().IsSet(nsPropertyFlags::Pointer) || (pProp->GetFlags().IsSet(nsPropertyFlags::Class) && !isValueType) ? nsVariantType::Uuid : pProp->GetSpecificType()->GetVariantType();

    switch (pProp->GetCategory())
    {
      case nsPropertyCategory::Array:
      case nsPropertyCategory::Set:
      {
        const nsVariantArray& values = m_Data[storageInfo->m_uiIndex].Get<nsVariantArray>();
        if (index.CanConvertTo<nsUInt32>())
        {
          nsUInt32 uiIndex = index.ConvertTo<nsUInt32>();
          if (uiIndex <= values.GetCount())
          {
            nsVariantArray changedValues = values;
            if (pProp->GetSpecificType() == nsGetStaticRTTI<nsVariant>())
            {
              changedValues.InsertAt(uiIndex, value);
              m_Data[storageInfo->m_uiIndex] = changedValues;
              return true;
            }
            else if (value.CanConvertTo(SpecVarType))
            {
              // We are lenient here regarding the type, as we may have stored values in the undo-redo stack
              // that may have a different type now as someone reloaded the type information and replaced a type.
              changedValues.InsertAt(uiIndex, value.ConvertTo(SpecVarType));
              m_Data[storageInfo->m_uiIndex] = changedValues;
              return true;
            }
          }
        }
      }
      break;
      case nsPropertyCategory::Map:
      {
        const nsVariantDictionary& values = m_Data[storageInfo->m_uiIndex].Get<nsVariantDictionary>();
        if (index.IsA<nsString>() && !values.Contains(index.Get<nsString>()))
        {
          const nsString& sIndex = index.Get<nsString>();
          nsVariantDictionary changedValues = values;
          if (pProp->GetSpecificType() == nsGetStaticRTTI<nsVariant>())
          {
            changedValues.Insert(sIndex, value);
            m_Data[storageInfo->m_uiIndex] = changedValues;
            return true;
          }
          else if (value.CanConvertTo(SpecVarType))
          {
            // We are lenient here regarding the type, as we may have stored values in the undo-redo stack
            // that may have a different type now as someone reloaded the type information and replaced a type.
            changedValues.Insert(sIndex, value.ConvertTo(SpecVarType));
            m_Data[storageInfo->m_uiIndex] = changedValues;
            return true;
          }
        }
      }
      break;
      default:
        break;
    }
  }
  return false;
}

bool nsReflectedTypeStorageAccessor::RemoveValue(nsStringView sProperty, nsVariant index)
{
  const nsReflectedTypeStorageManager::ReflectedTypeStorageMapping::StorageInfo* storageInfo = nullptr;
  if (m_pMapping->m_PathToStorageInfoTable.TryGetValue(sProperty, storageInfo))
  {
    if (storageInfo->m_Type == nsVariant::Type::Invalid)
      return false;

    const nsAbstractProperty* pProp = GetType()->FindPropertyByName(sProperty);
    if (pProp == nullptr)
      return false;

    switch (pProp->GetCategory())
    {
      case nsPropertyCategory::Array:
      case nsPropertyCategory::Set:
      {
        const nsVariantArray& values = m_Data[storageInfo->m_uiIndex].Get<nsVariantArray>();
        if (index.CanConvertTo<nsUInt32>())
        {
          nsUInt32 uiIndex = index.ConvertTo<nsUInt32>();
          if (uiIndex < values.GetCount())
          {
            nsVariantArray changedValues = values;
            changedValues.RemoveAtAndCopy(uiIndex);
            m_Data[storageInfo->m_uiIndex] = changedValues;
            return true;
          }
        }
      }
      break;
      case nsPropertyCategory::Map:
      {
        const nsVariantDictionary& values = m_Data[storageInfo->m_uiIndex].Get<nsVariantDictionary>();
        if (index.IsA<nsString>() && values.Contains(index.Get<nsString>()))
        {
          const nsString& sIndex = index.Get<nsString>();
          nsVariantDictionary changedValues = values;
          changedValues.Remove(sIndex);
          m_Data[storageInfo->m_uiIndex] = changedValues;
          return true;
        }
      }
      break;
      default:
        break;
    }
  }
  return false;
}

bool nsReflectedTypeStorageAccessor::MoveValue(nsStringView sProperty, nsVariant oldIndex, nsVariant newIndex)
{
  const nsReflectedTypeStorageManager::ReflectedTypeStorageMapping::StorageInfo* storageInfo = nullptr;
  if (m_pMapping->m_PathToStorageInfoTable.TryGetValue(sProperty, storageInfo))
  {
    if (storageInfo->m_Type == nsVariant::Type::Invalid)
      return false;

    const nsAbstractProperty* pProp = GetType()->FindPropertyByName(sProperty);
    if (pProp == nullptr)
      return false;

    switch (pProp->GetCategory())
    {
      case nsPropertyCategory::Array:
      case nsPropertyCategory::Set:
      {
        const nsVariantArray& values = m_Data[storageInfo->m_uiIndex].Get<nsVariantArray>();
        if (oldIndex.CanConvertTo<nsUInt32>() && newIndex.CanConvertTo<nsUInt32>())
        {
          nsUInt32 uiOldIndex = oldIndex.ConvertTo<nsUInt32>();
          nsUInt32 uiNewIndex = newIndex.ConvertTo<nsUInt32>();
          if (uiOldIndex < values.GetCount() && uiNewIndex <= values.GetCount())
          {
            nsVariantArray changedValues = values;
            nsVariant value = changedValues[uiOldIndex];
            changedValues.RemoveAtAndCopy(uiOldIndex);
            if (uiNewIndex > uiOldIndex)
            {
              uiNewIndex -= 1;
            }
            changedValues.InsertAt(uiNewIndex, value);

            m_Data[storageInfo->m_uiIndex] = changedValues;
            return true;
          }
        }
      }
      break;
      case nsPropertyCategory::Map:
      {
        const nsVariantDictionary& values = m_Data[storageInfo->m_uiIndex].Get<nsVariantDictionary>();
        if (oldIndex.IsA<nsString>() && values.Contains(oldIndex.Get<nsString>()) && newIndex.IsA<nsString>())
        {
          const nsString& sIndex = oldIndex.Get<nsString>();
          nsVariantDictionary changedValues = values;
          changedValues.Insert(newIndex.Get<nsString>(), changedValues[sIndex]);
          changedValues.Remove(sIndex);
          m_Data[storageInfo->m_uiIndex] = changedValues;
          return true;
        }
      }
      default:
        break;
    }
  }
  return false;
}

nsVariant nsReflectedTypeStorageAccessor::GetPropertyChildIndex(nsStringView sProperty, const nsVariant& value) const
{
  const nsReflectedTypeStorageManager::ReflectedTypeStorageMapping::StorageInfo* storageInfo = nullptr;
  if (m_pMapping->m_PathToStorageInfoTable.TryGetValue(sProperty, storageInfo))
  {
    if (storageInfo->m_Type == nsVariant::Type::Invalid)
      return nsVariant();

    const nsAbstractProperty* pProp = GetType()->FindPropertyByName(sProperty);
    if (pProp == nullptr)
      return nsVariant();

    const bool isValueType = nsReflectionUtils::IsValueType(pProp);
    const nsVariantType::Enum SpecVarType = pProp->GetFlags().IsSet(nsPropertyFlags::Pointer) || (pProp->GetFlags().IsSet(nsPropertyFlags::Class) && !isValueType) ? nsVariantType::Uuid : pProp->GetSpecificType()->GetVariantType();

    switch (pProp->GetCategory())
    {
      case nsPropertyCategory::Array:
      case nsPropertyCategory::Set:
      {
        if (value.CanConvertTo(SpecVarType))
        {
          const nsVariantArray& values = m_Data[storageInfo->m_uiIndex].Get<nsVariantArray>();
          for (nsUInt32 i = 0; i < values.GetCount(); i++)
          {
            if (values[i] == value)
              return nsVariant((nsUInt32)i);
          }
        }
      }
      break;
      case nsPropertyCategory::Map:
      {
        if (value.CanConvertTo(SpecVarType))
        {
          const nsVariantDictionary& values = m_Data[storageInfo->m_uiIndex].Get<nsVariantDictionary>();
          for (auto it = values.GetIterator(); it.IsValid(); ++it)
          {
            if (it.Value() == value)
              return nsVariant(it.Key());
          }
        }
      }
      break;
      default:
        break;
    }
  }
  return nsVariant();
}
