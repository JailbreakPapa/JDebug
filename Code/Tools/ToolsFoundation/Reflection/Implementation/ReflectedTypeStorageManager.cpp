#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Types/VariantTypeRegistry.h>
#include <ToolsFoundation/Document/Document.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>
#include <ToolsFoundation/Reflection/PhantomRttiManager.h>
#include <ToolsFoundation/Reflection/ReflectedTypeStorageAccessor.h>
#include <ToolsFoundation/Reflection/ReflectedTypeStorageManager.h>
#include <ToolsFoundation/Reflection/ToolsReflectionUtils.h>

wdMap<const wdRTTI*, wdReflectedTypeStorageManager::ReflectedTypeStorageMapping*> wdReflectedTypeStorageManager::s_ReflectedTypeToStorageMapping;

// clang-format off
// 
WD_BEGIN_SUBSYSTEM_DECLARATION(ToolsFoundation, ReflectedTypeStorageManager)

  BEGIN_SUBSYSTEM_DEPENDENCIES
  "Foundation",
  "ReflectedTypeManager"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    wdReflectedTypeStorageManager::Startup();
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    wdReflectedTypeStorageManager::Shutdown();
  }

WD_END_SUBSYSTEM_DECLARATION;
// clang-format on

////////////////////////////////////////////////////////////////////////
// wdReflectedTypeStorageManager::ReflectedTypeStorageMapping public functions
////////////////////////////////////////////////////////////////////////

void wdReflectedTypeStorageManager::ReflectedTypeStorageMapping::AddProperties(const wdRTTI* pType)
{
  // Mark all properties as invalid. Thus, when a property is dropped we know it is no longer valid.
  // All others will be set to their old or new value by the AddPropertiesRecursive function.
  for (auto it = m_PathToStorageInfoTable.GetIterator(); it.IsValid(); ++it)
  {
    it.Value().m_Type = wdVariant::Type::Invalid;
  }

  wdSet<const wdDocumentObject*> requiresPatchingEmbeddedClass;
  AddPropertiesRecursive(pType, requiresPatchingEmbeddedClass);

  for (const wdDocumentObject* pObject : requiresPatchingEmbeddedClass)
  {
    pObject->GetDocumentObjectManager()->PatchEmbeddedClassObjects(pObject);
  }
}

void wdReflectedTypeStorageManager::ReflectedTypeStorageMapping::AddPropertiesRecursive(
  const wdRTTI* pType, wdSet<const wdDocumentObject*>& ref_requiresPatchingEmbeddedClass)
{
  // Parse parent class
  const wdRTTI* pParent = pType->GetParentType();
  if (pParent != nullptr)
    AddPropertiesRecursive(pParent, ref_requiresPatchingEmbeddedClass);

  // Parse properties
  const wdUInt32 uiPropertyCount = pType->GetProperties().GetCount();
  for (wdUInt32 i = 0; i < uiPropertyCount; ++i)
  {
    const wdAbstractProperty* pProperty = pType->GetProperties()[i];

    wdString path = pProperty->GetPropertyName();

    StorageInfo* storageInfo = nullptr;
    if (m_PathToStorageInfoTable.TryGetValue(path, storageInfo))
    {
      // Value already present, update type and instances
      storageInfo->m_Type = GetStorageType(pProperty);
      storageInfo->m_DefaultValue = wdToolsReflectionUtils::GetStorageDefault(pProperty);
      UpdateInstances(storageInfo->m_uiIndex, pProperty, ref_requiresPatchingEmbeddedClass);
    }
    else
    {
      const wdUInt16 uiIndex = (wdUInt16)m_PathToStorageInfoTable.GetCount();

      // Add value, new entries are appended
      m_PathToStorageInfoTable.Insert(path, StorageInfo(uiIndex, GetStorageType(pProperty), wdToolsReflectionUtils::GetStorageDefault(pProperty)));
      AddPropertyToInstances(uiIndex, pProperty, ref_requiresPatchingEmbeddedClass);
    }
  }
}

void wdReflectedTypeStorageManager::ReflectedTypeStorageMapping::UpdateInstances(
  wdUInt32 uiIndex, const wdAbstractProperty* pProperty, wdSet<const wdDocumentObject*>& ref_requiresPatchingEmbeddedClass)
{
  for (auto it = m_Instances.GetIterator(); it.IsValid(); ++it)
  {
    wdDynamicArray<wdVariant>& data = it.Key()->m_Data;
    WD_ASSERT_DEV(uiIndex < data.GetCount(), "wdReflectedTypeStorageAccessor found with fewer properties that is should have!");
    wdVariant& value = data[uiIndex];

    const auto SpecVarType = GetStorageType(pProperty);

    switch (pProperty->GetCategory())
    {
      case wdPropertyCategory::Member:
      {
        if (pProperty->GetFlags().IsSet(wdPropertyFlags::Class) && !pProperty->GetFlags().IsSet(wdPropertyFlags::Pointer))
        {
          // Did the type change from what it was previously?
          if (value.GetType() == SpecVarType)
          {
            if (!value.Get<wdUuid>().IsValid())
            {
              ref_requiresPatchingEmbeddedClass.Insert(it.Key()->GetOwner());
            }
          }
          else
          {
            value = wdToolsReflectionUtils::GetStorageDefault(pProperty);
            ref_requiresPatchingEmbeddedClass.Insert(it.Key()->GetOwner());
          }
          continue;
        }
        else
        {
          // Did the type change from what it was previously?
          if (value.GetType() == SpecVarType)
          {
            // The types are equal so nothing needs to be done. The current value will stay valid.
            // This should be the most common case.
            continue;
          }
          else
          {
            // The type is new or has changed but we have a valid value stored. Assume that the type of a property was changed
            // and try to convert the value.
            if (value.CanConvertTo(SpecVarType))
            {
              value = value.ConvertTo(SpecVarType);
            }
            else
            {
              value = wdToolsReflectionUtils::GetStorageDefault(pProperty);
            }
            continue;
          }
        }
      }
      break;
      case wdPropertyCategory::Array:
      case wdPropertyCategory::Set:
      {
        if (value.GetType() != wdVariantType::VariantArray)
        {
          value = wdVariantArray();
          continue;
        }
        wdVariantArray values = value.Get<wdVariantArray>();
        if (values.IsEmpty())
          continue;

        // Same conversion logic as for wdPropertyCategory::Member, but for each element instead.
        for (wdUInt32 i = 0; i < values.GetCount(); i++)
        {
          wdVariant& var = values[i];
          if (var.GetType() == SpecVarType)
          {
            continue;
          }
          else
          {
            wdResult res(WD_FAILURE);
            var = var.ConvertTo(SpecVarType, &res);
            if (res == WD_FAILURE)
            {
              var = wdReflectionUtils::GetDefaultValue(pProperty, i);
            }
          }
        }
        value = values;
      }
      break;
      case wdPropertyCategory::Map:
      {
        if (value.GetType() != wdVariantType::VariantDictionary)
        {
          value = wdVariantDictionary();
          continue;
        }
        wdVariantDictionary values = value.Get<wdVariantDictionary>();
        if (values.IsEmpty())
          continue;

        // Same conversion logic as for wdPropertyCategory::Member, but for each element instead.
        for (auto it2 = values.GetIterator(); it2.IsValid(); ++it2)
        {
          if (it2.Value().GetType() == SpecVarType)
          {
            continue;
          }
          else
          {
            wdResult res(WD_FAILURE);
            it2.Value() = it2.Value().ConvertTo(SpecVarType, &res);
            if (res == WD_FAILURE)
            {
              it2.Value() = wdReflectionUtils::GetDefaultValue(pProperty, it2.Key());
            }
          }
        }
        value = values;
      }
      break;
      default:
        break;
    }
  }
}

void wdReflectedTypeStorageManager::ReflectedTypeStorageMapping::AddPropertyToInstances(
  wdUInt32 uiIndex, const wdAbstractProperty* pProperty, wdSet<const wdDocumentObject*>& ref_requiresPatchingEmbeddedClass)
{
  if (pProperty->GetCategory() != wdPropertyCategory::Member)
    return;

  for (auto it = m_Instances.GetIterator(); it.IsValid(); ++it)
  {
    wdDynamicArray<wdVariant>& data = it.Key()->m_Data;
    WD_ASSERT_DEV(data.GetCount() == uiIndex, "wdReflectedTypeStorageAccessor found with a property count that does not match its storage mapping!");
    data.PushBack(wdToolsReflectionUtils::GetStorageDefault(pProperty));
    if (pProperty->GetFlags().IsSet(wdPropertyFlags::Class) && !pProperty->GetFlags().IsSet(wdPropertyFlags::Pointer))
    {
      ref_requiresPatchingEmbeddedClass.Insert(it.Key()->GetOwner());
    }
  }
}


wdVariantType::Enum wdReflectedTypeStorageManager::ReflectedTypeStorageMapping::GetStorageType(const wdAbstractProperty* pProperty)
{
  wdVariantType::Enum type = wdVariantType::Uuid;

  const bool bIsValueType = wdReflectionUtils::IsValueType(pProperty);

  switch (pProperty->GetCategory())
  {
    case wdPropertyCategory::Member:
    {
      if (bIsValueType)
        type = pProperty->GetSpecificType()->GetVariantType();
      else if (pProperty->GetFlags().IsAnySet(wdPropertyFlags::IsEnum | wdPropertyFlags::Bitflags))
        type = wdVariantType::Int64;
    }
    break;
    case wdPropertyCategory::Array:
    case wdPropertyCategory::Set:
    {
      type = wdVariantType::VariantArray;
    }
    break;
    case wdPropertyCategory::Map:
    {
      type = wdVariantType::VariantDictionary;
    }
    break;
    default:
      break;
  }

  return type;
}

////////////////////////////////////////////////////////////////////////
// wdReflectedTypeStorageManager private functions
////////////////////////////////////////////////////////////////////////

void wdReflectedTypeStorageManager::Startup()
{
  wdPhantomRttiManager::s_Events.AddEventHandler(TypeEventHandler);
}

void wdReflectedTypeStorageManager::Shutdown()
{
  wdPhantomRttiManager::s_Events.RemoveEventHandler(TypeEventHandler);

  for (auto it = s_ReflectedTypeToStorageMapping.GetIterator(); it.IsValid(); ++it)
  {
    ReflectedTypeStorageMapping* pMapping = it.Value();

    for (auto inst : pMapping->m_Instances)
    {
      const char* sz = inst->GetType()->GetTypeName();
      wdLog::Error("Type '{0}' survived shutdown!", sz);
    }

    WD_ASSERT_DEV(pMapping->m_Instances.IsEmpty(), "A type was removed which still has instances using the type!");
    WD_DEFAULT_DELETE(pMapping);
  }
  s_ReflectedTypeToStorageMapping.Clear();
}

const wdReflectedTypeStorageManager::ReflectedTypeStorageMapping* wdReflectedTypeStorageManager::AddStorageAccessor(
  wdReflectedTypeStorageAccessor* pInstance)
{
  ReflectedTypeStorageMapping* pMapping = GetTypeStorageMapping(pInstance->GetType());
  pMapping->m_Instances.Insert(pInstance);
  return pMapping;
}

void wdReflectedTypeStorageManager::RemoveStorageAccessor(wdReflectedTypeStorageAccessor* pInstance)
{
  ReflectedTypeStorageMapping* pMapping = GetTypeStorageMapping(pInstance->GetType());
  pMapping->m_Instances.Remove(pInstance);
}

wdReflectedTypeStorageManager::ReflectedTypeStorageMapping* wdReflectedTypeStorageManager::GetTypeStorageMapping(const wdRTTI* pType)
{
  WD_ASSERT_DEV(pType != nullptr, "Nullptr is not a valid type!");
  auto it = s_ReflectedTypeToStorageMapping.Find(pType);
  if (it.IsValid())
    return it.Value();

  ReflectedTypeStorageMapping* pMapping = WD_DEFAULT_NEW(ReflectedTypeStorageMapping);
  pMapping->AddProperties(pType);
  s_ReflectedTypeToStorageMapping[pType] = pMapping;
  return pMapping;
}

void wdReflectedTypeStorageManager::TypeEventHandler(const wdPhantomRttiManagerEvent& e)
{
  switch (e.m_Type)
  {
    case wdPhantomRttiManagerEvent::Type::TypeAdded:
    {
      const wdRTTI* pType = e.m_pChangedType;
      WD_ASSERT_DEV(pType != nullptr, "A type was added but it has an invalid handle!");

      WD_ASSERT_DEV(!s_ReflectedTypeToStorageMapping.Find(e.m_pChangedType).IsValid(), "The type '{0}' was added twice!", pType->GetTypeName());
      GetTypeStorageMapping(e.m_pChangedType);
    }
    break;
    case wdPhantomRttiManagerEvent::Type::TypeChanged:
    {
      const wdRTTI* pNewType = e.m_pChangedType;
      WD_ASSERT_DEV(pNewType != nullptr, "A type was updated but its handle is invalid!");

      ReflectedTypeStorageMapping* pMapping = s_ReflectedTypeToStorageMapping[e.m_pChangedType];
      WD_ASSERT_DEV(pMapping != nullptr, "A type was updated but no mapping exists for it!");

      if (pNewType->GetParentType() != nullptr && wdStringUtils::IsEqual(pNewType->GetParentType()->GetTypeName(), "wdEnumBase"))
      {
        // WD_ASSERT_DEV(false, "Updating enums not implemented yet!");
        break;
      }
      else if (pNewType->GetParentType() != nullptr && wdStringUtils::IsEqual(pNewType->GetParentType()->GetTypeName(), "wdBitflagsBase"))
      {
        WD_ASSERT_DEV(false, "Updating bitflags not implemented yet!");
      }

      pMapping->AddProperties(pNewType);

      wdSet<wdRTTI*> dependencies;
      // Update all types that either derive from the changed type or have the type as a member.
      for (auto it = s_ReflectedTypeToStorageMapping.GetIterator(); it.IsValid(); ++it)
      {
        if (it.Key() == e.m_pChangedType)
          continue;

        const wdRTTI* pType = it.Key();
        if (pType->IsDerivedFrom(e.m_pChangedType))
        {
          it.Value()->AddProperties(pType);
        }
      }
    }
    break;
    case wdPhantomRttiManagerEvent::Type::TypeRemoved:
    {
      ReflectedTypeStorageMapping* pMapping = s_ReflectedTypeToStorageMapping[e.m_pChangedType];
      WD_ASSERT_DEV(pMapping != nullptr, "A type was removed but no mapping ever exited for it!");
      WD_ASSERT_DEV(pMapping->m_Instances.IsEmpty(), "A type was removed which still has instances using the type!");
      s_ReflectedTypeToStorageMapping.Remove(e.m_pChangedType);
      WD_DEFAULT_DELETE(pMapping);
    }
    break;
  }
}
