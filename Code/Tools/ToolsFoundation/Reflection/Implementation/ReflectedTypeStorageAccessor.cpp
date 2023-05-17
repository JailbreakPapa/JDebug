#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/Types/Status.h>
#include <Foundation/Types/VariantTypeRegistry.h>
#include <ToolsFoundation/Reflection/ReflectedTypeStorageAccessor.h>
#include <ToolsFoundation/Reflection/ToolsReflectionUtils.h>


////////////////////////////////////////////////////////////////////////
// wdReflectedTypeStorageAccessor public functions
////////////////////////////////////////////////////////////////////////

wdReflectedTypeStorageAccessor::wdReflectedTypeStorageAccessor(const wdRTTI* pRtti, wdDocumentObject* pOwner)
  : wdIReflectedTypeAccessor(pRtti, pOwner)
{
  const wdRTTI* pType = pRtti;
  WD_ASSERT_DEV(pType != nullptr, "Trying to construct an wdReflectedTypeStorageAccessor for an invalid type!");
  m_pMapping = wdReflectedTypeStorageManager::AddStorageAccessor(this);
  WD_ASSERT_DEV(m_pMapping != nullptr, "The type for this wdReflectedTypeStorageAccessor is unknown to the wdReflectedTypeStorageManager!");

  auto& indexTable = m_pMapping->m_PathToStorageInfoTable;
  const wdUInt32 uiProperties = indexTable.GetCount();
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

wdReflectedTypeStorageAccessor::~wdReflectedTypeStorageAccessor()
{
  wdReflectedTypeStorageManager::RemoveStorageAccessor(this);
}

const wdVariant wdReflectedTypeStorageAccessor::GetValue(const char* szProperty, wdVariant index, wdStatus* pRes) const
{
  const wdAbstractProperty* pProp = GetType()->FindPropertyByName(szProperty);
  if (pProp == nullptr)
  {
    if (pRes)
      *pRes = wdStatus(wdFmt("Property '{0}' not found in type '{1}'", szProperty, GetType()->GetTypeName()));
    return wdVariant();
  }

  if (pRes)
    *pRes = wdStatus(WD_SUCCESS);
  const wdReflectedTypeStorageManager::ReflectedTypeStorageMapping::StorageInfo* storageInfo = nullptr;
  if (m_pMapping->m_PathToStorageInfoTable.TryGetValue(szProperty, storageInfo))
  {
    switch (pProp->GetCategory())
    {
      case wdPropertyCategory::Member:
        return m_Data[storageInfo->m_uiIndex];
      case wdPropertyCategory::Array:
      case wdPropertyCategory::Set:
      {
        if (!index.IsValid())
        {
          return m_Data[storageInfo->m_uiIndex];
        }

        const wdVariantArray& values = m_Data[storageInfo->m_uiIndex].Get<wdVariantArray>();
        if (index.CanConvertTo<wdUInt32>())
        {
          wdUInt32 uiIndex = index.ConvertTo<wdUInt32>();
          if (uiIndex < values.GetCount())
          {
            return values[uiIndex];
          }
        }
        if (pRes)
          *pRes = wdStatus(wdFmt("Index '{0}' for property '{1}' is invalid or out of bounds.", index, szProperty));
      }
      break;
      case wdPropertyCategory::Map:
      {
        if (!index.IsValid())
        {
          return m_Data[storageInfo->m_uiIndex];
        }

        const wdVariantDictionary& values = m_Data[storageInfo->m_uiIndex].Get<wdVariantDictionary>();
        if (index.IsA<wdString>())
        {
          const wdString& sIndex = index.Get<wdString>();
          if (const wdVariant* pValue = values.GetValue(sIndex))
          {
            return *pValue;
          }
        }
        if (pRes)
          *pRes = wdStatus(wdFmt("Index '{0}' for property '{1}' is invalid or out of bounds.", index, szProperty));
      }
      break;
      default:
        break;
    }
  }
  return wdVariant();
}

bool wdReflectedTypeStorageAccessor::SetValue(const char* szProperty, const wdVariant& value, wdVariant index)
{
  const wdReflectedTypeStorageManager::ReflectedTypeStorageMapping::StorageInfo* storageInfo = nullptr;
  if (m_pMapping->m_PathToStorageInfoTable.TryGetValue(szProperty, storageInfo))
  {
    const wdAbstractProperty* pProp = GetType()->FindPropertyByName(szProperty);
    if (pProp == nullptr)
      return false;
    WD_ASSERT_DEV(pProp->GetSpecificType() == wdGetStaticRTTI<wdVariant>() || value.IsValid(), "");

    if (storageInfo->m_Type == wdVariantType::TypedObject && storageInfo->m_DefaultValue.GetReflectedType() != value.GetReflectedType())
    {
      // Typed objects must match exactly.
      return false;
    }

    const bool isValueType = wdReflectionUtils::IsValueType(pProp);
    const wdVariantType::Enum SpecVarType = pProp->GetFlags().IsSet(wdPropertyFlags::Pointer) || (pProp->GetFlags().IsSet(wdPropertyFlags::Class) && !isValueType) ? wdVariantType::Uuid : pProp->GetSpecificType()->GetVariantType();

    switch (pProp->GetCategory())
    {
      case wdPropertyCategory::Member:
      {
        if (value.IsA<wdString>() && pProp->GetFlags().IsAnySet(wdPropertyFlags::IsEnum | wdPropertyFlags::Bitflags))
        {
          wdInt64 iValue;
          wdReflectionUtils::StringToEnumeration(pProp->GetSpecificType(), value.Get<wdString>(), iValue);
          m_Data[storageInfo->m_uiIndex] = wdVariant(iValue).ConvertTo(storageInfo->m_Type);
          return true;
        }
        else if (pProp->GetSpecificType() == wdGetStaticRTTI<wdVariant>())
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
      case wdPropertyCategory::Array:
      case wdPropertyCategory::Set:
      {
        const wdVariantArray& values = m_Data[storageInfo->m_uiIndex].Get<wdVariantArray>();
        if (index.CanConvertTo<wdUInt32>())
        {
          wdUInt32 uiIndex = index.ConvertTo<wdUInt32>();
          if (uiIndex < values.GetCount())
          {
            wdVariantArray changedValues = values;
            if (pProp->GetSpecificType() == wdGetStaticRTTI<wdVariant>())
            {
              changedValues[uiIndex] = value;
              m_Data[storageInfo->m_uiIndex] = changedValues;
              return true;
            }
            else
            {
              if (pProp->GetSpecificType() == wdGetStaticRTTI<wdVariant>())
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
      case wdPropertyCategory::Map:
      {
        const wdVariantDictionary& values = m_Data[storageInfo->m_uiIndex].Get<wdVariantDictionary>();
        if (index.IsA<wdString>() && values.Contains(index.Get<wdString>()))
        {
          const wdString& sIndex = index.Get<wdString>();
          wdVariantDictionary changedValues = values;
          if (pProp->GetSpecificType() == wdGetStaticRTTI<wdVariant>())
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

wdInt32 wdReflectedTypeStorageAccessor::GetCount(const char* szProperty) const
{
  const wdReflectedTypeStorageManager::ReflectedTypeStorageMapping::StorageInfo* storageInfo = nullptr;
  if (m_pMapping->m_PathToStorageInfoTable.TryGetValue(szProperty, storageInfo))
  {
    if (storageInfo->m_Type == wdVariant::Type::Invalid)
      return false;

    const wdAbstractProperty* pProp = GetType()->FindPropertyByName(szProperty);
    if (pProp == nullptr)
      return false;

    switch (pProp->GetCategory())
    {
      case wdPropertyCategory::Array:
      case wdPropertyCategory::Set:
      {
        const wdVariantArray& values = m_Data[storageInfo->m_uiIndex].Get<wdVariantArray>();
        return values.GetCount();
      }
      case wdPropertyCategory::Map:
      {
        const wdVariantDictionary& values = m_Data[storageInfo->m_uiIndex].Get<wdVariantDictionary>();
        return values.GetCount();
      }
      default:
        break;
    }
  }
  return -1;
}

bool wdReflectedTypeStorageAccessor::GetKeys(const char* szProperty, wdDynamicArray<wdVariant>& out_keys) const
{
  out_keys.Clear();

  const wdReflectedTypeStorageManager::ReflectedTypeStorageMapping::StorageInfo* storageInfo = nullptr;
  if (m_pMapping->m_PathToStorageInfoTable.TryGetValue(szProperty, storageInfo))
  {
    if (storageInfo->m_Type == wdVariant::Type::Invalid)
      return false;

    const wdAbstractProperty* pProp = GetType()->FindPropertyByName(szProperty);
    if (pProp == nullptr)
      return false;

    switch (pProp->GetCategory())
    {
      case wdPropertyCategory::Array:
      case wdPropertyCategory::Set:
      {
        const wdVariantArray& values = m_Data[storageInfo->m_uiIndex].Get<wdVariantArray>();
        out_keys.Reserve(values.GetCount());
        for (wdUInt32 i = 0; i < values.GetCount(); ++i)
        {
          out_keys.PushBack(i);
        }
        return true;
      }
      break;
      case wdPropertyCategory::Map:
      {
        const wdVariantDictionary& values = m_Data[storageInfo->m_uiIndex].Get<wdVariantDictionary>();
        out_keys.Reserve(values.GetCount());
        for (auto it = values.GetIterator(); it.IsValid(); ++it)
        {
          out_keys.PushBack(wdVariant(it.Key()));
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
bool wdReflectedTypeStorageAccessor::InsertValue(const char* szProperty, wdVariant index, const wdVariant& value)
{
  const wdReflectedTypeStorageManager::ReflectedTypeStorageMapping::StorageInfo* storageInfo = nullptr;
  if (m_pMapping->m_PathToStorageInfoTable.TryGetValue(szProperty, storageInfo))
  {
    if (storageInfo->m_Type == wdVariant::Type::Invalid)
      return false;

    const wdAbstractProperty* pProp = GetType()->FindPropertyByName(szProperty);
    if (pProp == nullptr)
      return false;

    if (storageInfo->m_Type == wdVariantType::TypedObject && storageInfo->m_DefaultValue.GetReflectedType() != value.GetReflectedType())
    {
      // Typed objects must match exactly.
      return false;
    }

    const bool isValueType = wdReflectionUtils::IsValueType(pProp);
    const wdVariantType::Enum SpecVarType = pProp->GetFlags().IsSet(wdPropertyFlags::Pointer) || (pProp->GetFlags().IsSet(wdPropertyFlags::Class) && !isValueType) ? wdVariantType::Uuid : pProp->GetSpecificType()->GetVariantType();

    switch (pProp->GetCategory())
    {
      case wdPropertyCategory::Array:
      case wdPropertyCategory::Set:
      {
        const wdVariantArray& values = m_Data[storageInfo->m_uiIndex].Get<wdVariantArray>();
        if (index.CanConvertTo<wdUInt32>())
        {
          wdUInt32 uiIndex = index.ConvertTo<wdUInt32>();
          if (uiIndex <= values.GetCount())
          {
            wdVariantArray changedValues = values;
            if (pProp->GetSpecificType() == wdGetStaticRTTI<wdVariant>())
            {
              changedValues.Insert(value, uiIndex);
              m_Data[storageInfo->m_uiIndex] = changedValues;
              return true;
            }
            else if (value.CanConvertTo(SpecVarType))
            {
              // We are lenient here regarding the type, as we may have stored values in the undo-redo stack
              // that may have a different type now as someone reloaded the type information and replaced a type.
              changedValues.Insert(value.ConvertTo(SpecVarType), uiIndex);
              m_Data[storageInfo->m_uiIndex] = changedValues;
              return true;
            }
          }
        }
      }
      break;
      case wdPropertyCategory::Map:
      {
        const wdVariantDictionary& values = m_Data[storageInfo->m_uiIndex].Get<wdVariantDictionary>();
        if (index.IsA<wdString>() && !values.Contains(index.Get<wdString>()))
        {
          const wdString& sIndex = index.Get<wdString>();
          wdVariantDictionary changedValues = values;
          if (pProp->GetSpecificType() == wdGetStaticRTTI<wdVariant>())
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

bool wdReflectedTypeStorageAccessor::RemoveValue(const char* szProperty, wdVariant index)
{
  const wdReflectedTypeStorageManager::ReflectedTypeStorageMapping::StorageInfo* storageInfo = nullptr;
  if (m_pMapping->m_PathToStorageInfoTable.TryGetValue(szProperty, storageInfo))
  {
    if (storageInfo->m_Type == wdVariant::Type::Invalid)
      return false;

    const wdAbstractProperty* pProp = GetType()->FindPropertyByName(szProperty);
    if (pProp == nullptr)
      return false;

    switch (pProp->GetCategory())
    {
      case wdPropertyCategory::Array:
      case wdPropertyCategory::Set:
      {
        const wdVariantArray& values = m_Data[storageInfo->m_uiIndex].Get<wdVariantArray>();
        if (index.CanConvertTo<wdUInt32>())
        {
          wdUInt32 uiIndex = index.ConvertTo<wdUInt32>();
          if (uiIndex < values.GetCount())
          {
            wdVariantArray changedValues = values;
            changedValues.RemoveAtAndCopy(uiIndex);
            m_Data[storageInfo->m_uiIndex] = changedValues;
            return true;
          }
        }
      }
      break;
      case wdPropertyCategory::Map:
      {
        const wdVariantDictionary& values = m_Data[storageInfo->m_uiIndex].Get<wdVariantDictionary>();
        if (index.IsA<wdString>() && values.Contains(index.Get<wdString>()))
        {
          const wdString& sIndex = index.Get<wdString>();
          wdVariantDictionary changedValues = values;
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

bool wdReflectedTypeStorageAccessor::MoveValue(const char* szProperty, wdVariant oldIndex, wdVariant newIndex)
{
  const wdReflectedTypeStorageManager::ReflectedTypeStorageMapping::StorageInfo* storageInfo = nullptr;
  if (m_pMapping->m_PathToStorageInfoTable.TryGetValue(szProperty, storageInfo))
  {
    if (storageInfo->m_Type == wdVariant::Type::Invalid)
      return false;

    const wdAbstractProperty* pProp = GetType()->FindPropertyByName(szProperty);
    if (pProp == nullptr)
      return false;

    switch (pProp->GetCategory())
    {
      case wdPropertyCategory::Array:
      case wdPropertyCategory::Set:
      {
        const wdVariantArray& values = m_Data[storageInfo->m_uiIndex].Get<wdVariantArray>();
        if (oldIndex.CanConvertTo<wdUInt32>() && newIndex.CanConvertTo<wdUInt32>())
        {
          wdUInt32 uiOldIndex = oldIndex.ConvertTo<wdUInt32>();
          wdUInt32 uiNewIndex = newIndex.ConvertTo<wdUInt32>();
          if (uiOldIndex < values.GetCount() && uiNewIndex <= values.GetCount())
          {
            wdVariantArray changedValues = values;
            wdVariant value = changedValues[uiOldIndex];
            changedValues.RemoveAtAndCopy(uiOldIndex);
            if (uiNewIndex > uiOldIndex)
            {
              uiNewIndex -= 1;
            }
            changedValues.Insert(value, uiNewIndex);

            m_Data[storageInfo->m_uiIndex] = changedValues;
            return true;
          }
        }
      }
      break;
      case wdPropertyCategory::Map:
      {
        const wdVariantDictionary& values = m_Data[storageInfo->m_uiIndex].Get<wdVariantDictionary>();
        if (oldIndex.IsA<wdString>() && values.Contains(oldIndex.Get<wdString>()) && newIndex.IsA<wdString>())
        {
          const wdString& sIndex = oldIndex.Get<wdString>();
          wdVariantDictionary changedValues = values;
          changedValues.Insert(newIndex.Get<wdString>(), changedValues[sIndex]);
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

wdVariant wdReflectedTypeStorageAccessor::GetPropertyChildIndex(const char* szProperty, const wdVariant& value) const
{
  const wdReflectedTypeStorageManager::ReflectedTypeStorageMapping::StorageInfo* storageInfo = nullptr;
  if (m_pMapping->m_PathToStorageInfoTable.TryGetValue(szProperty, storageInfo))
  {
    if (storageInfo->m_Type == wdVariant::Type::Invalid)
      return wdVariant();

    const wdAbstractProperty* pProp = GetType()->FindPropertyByName(szProperty);
    if (pProp == nullptr)
      return wdVariant();

    const bool isValueType = wdReflectionUtils::IsValueType(pProp);
    const wdVariantType::Enum SpecVarType = pProp->GetFlags().IsSet(wdPropertyFlags::Pointer) || (pProp->GetFlags().IsSet(wdPropertyFlags::Class) && !isValueType) ? wdVariantType::Uuid : pProp->GetSpecificType()->GetVariantType();

    switch (pProp->GetCategory())
    {
      case wdPropertyCategory::Array:
      case wdPropertyCategory::Set:
      {
        if (value.CanConvertTo(SpecVarType))
        {
          const wdVariantArray& values = m_Data[storageInfo->m_uiIndex].Get<wdVariantArray>();
          for (wdUInt32 i = 0; i < values.GetCount(); i++)
          {
            if (values[i] == value)
              return wdVariant((wdUInt32)i);
          }
        }
      }
      break;
      case wdPropertyCategory::Map:
      {
        if (value.CanConvertTo(SpecVarType))
        {
          const wdVariantDictionary& values = m_Data[storageInfo->m_uiIndex].Get<wdVariantDictionary>();
          for (auto it = values.GetIterator(); it.IsValid(); ++it)
          {
            if (it.Value() == value)
              return wdVariant(it.Key());
          }
        }
      }
      break;
      default:
        break;
    }
  }
  return wdVariant();
}
