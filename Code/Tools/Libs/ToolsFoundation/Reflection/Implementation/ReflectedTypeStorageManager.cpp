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

nsMap<const nsRTTI*, nsReflectedTypeStorageManager::ReflectedTypeStorageMapping*> nsReflectedTypeStorageManager::s_ReflectedTypeToStorageMapping;

// clang-format off
// 
NS_BEGIN_SUBSYSTEM_DECLARATION(ToolsFoundation, ReflectedTypeStorageManager)

  BEGIN_SUBSYSTEM_DEPENDENCIES
  "Foundation",
  "ReflectedTypeManager"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    nsReflectedTypeStorageManager::Startup();
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    nsReflectedTypeStorageManager::Shutdown();
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

////////////////////////////////////////////////////////////////////////
// nsReflectedTypeStorageManager::ReflectedTypeStorageMapping public functions
////////////////////////////////////////////////////////////////////////

void nsReflectedTypeStorageManager::ReflectedTypeStorageMapping::AddProperties(const nsRTTI* pType)
{
  // Mark all properties as invalid. Thus, when a property is dropped we know it is no longer valid.
  // All others will be set to their old or new value by the AddPropertiesRecursive function.
  for (auto it = m_PathToStorageInfoTable.GetIterator(); it.IsValid(); ++it)
  {
    it.Value().m_Type = nsVariant::Type::Invalid;
  }

  nsSet<const nsDocumentObject*> requiresPatchingEmbeddedClass;
  AddPropertiesRecursive(pType, requiresPatchingEmbeddedClass);

  for (const nsDocumentObject* pObject : requiresPatchingEmbeddedClass)
  {
    pObject->GetDocumentObjectManager()->PatchEmbeddedClassObjects(pObject);
  }
}

void nsReflectedTypeStorageManager::ReflectedTypeStorageMapping::AddPropertiesRecursive(
  const nsRTTI* pType, nsSet<const nsDocumentObject*>& ref_requiresPatchingEmbeddedClass)
{
  // Parse parent class
  const nsRTTI* pParent = pType->GetParentType();
  if (pParent != nullptr)
    AddPropertiesRecursive(pParent, ref_requiresPatchingEmbeddedClass);

  // Parse properties
  const nsUInt32 uiPropertyCount = pType->GetProperties().GetCount();
  for (nsUInt32 i = 0; i < uiPropertyCount; ++i)
  {
    const nsAbstractProperty* pProperty = pType->GetProperties()[i];

    nsString path = pProperty->GetPropertyName();

    StorageInfo* storageInfo = nullptr;
    if (m_PathToStorageInfoTable.TryGetValue(path, storageInfo))
    {
      // Value already present, update type and instances
      storageInfo->m_Type = nsToolsReflectionUtils::GetStorageType(pProperty);
      storageInfo->m_DefaultValue = nsToolsReflectionUtils::GetStorageDefault(pProperty);
      UpdateInstances(storageInfo->m_uiIndex, pProperty, ref_requiresPatchingEmbeddedClass);
    }
    else
    {
      const nsUInt16 uiIndex = (nsUInt16)m_PathToStorageInfoTable.GetCount();

      // Add value, new entries are appended
      m_PathToStorageInfoTable.Insert(path, StorageInfo(uiIndex, nsToolsReflectionUtils::GetStorageType(pProperty), nsToolsReflectionUtils::GetStorageDefault(pProperty)));
      AddPropertyToInstances(uiIndex, pProperty, ref_requiresPatchingEmbeddedClass);
    }
  }
}

void nsReflectedTypeStorageManager::ReflectedTypeStorageMapping::UpdateInstances(
  nsUInt32 uiIndex, const nsAbstractProperty* pProperty, nsSet<const nsDocumentObject*>& ref_requiresPatchingEmbeddedClass)
{
  for (auto it = m_Instances.GetIterator(); it.IsValid(); ++it)
  {
    nsDynamicArray<nsVariant>& data = it.Key()->m_Data;
    NS_ASSERT_DEV(uiIndex < data.GetCount(), "nsReflectedTypeStorageAccessor found with fewer properties that is should have!");
    nsVariant& value = data[uiIndex];

    const auto SpecVarType = nsToolsReflectionUtils::GetStorageType(pProperty);

    switch (pProperty->GetCategory())
    {
      case nsPropertyCategory::Member:
      {
        if (pProperty->GetFlags().IsSet(nsPropertyFlags::Class) && !pProperty->GetFlags().IsSet(nsPropertyFlags::Pointer))
        {
          // Did the type change from what it was previously?
          if (value.GetType() == SpecVarType)
          {
            if (!value.Get<nsUuid>().IsValid())
            {
              ref_requiresPatchingEmbeddedClass.Insert(it.Key()->GetOwner());
            }
          }
          else
          {
            value = nsToolsReflectionUtils::GetStorageDefault(pProperty);
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
              value = nsToolsReflectionUtils::GetStorageDefault(pProperty);
            }
            continue;
          }
        }
      }
      break;
      case nsPropertyCategory::Array:
      case nsPropertyCategory::Set:
      {
        if (value.GetType() != nsVariantType::VariantArray)
        {
          value = nsVariantArray();
          continue;
        }
        nsVariantArray values = value.Get<nsVariantArray>();
        if (values.IsEmpty())
          continue;

        // Same conversion logic as for nsPropertyCategory::Member, but for each element instead.
        for (nsUInt32 i = 0; i < values.GetCount(); i++)
        {
          nsVariant& var = values[i];
          if (var.GetType() == SpecVarType)
          {
            continue;
          }
          else
          {
            nsResult res(NS_FAILURE);
            var = var.ConvertTo(SpecVarType, &res);
            if (res == NS_FAILURE)
            {
              var = nsReflectionUtils::GetDefaultValue(pProperty, i);
            }
          }
        }
        value = values;
      }
      break;
      case nsPropertyCategory::Map:
      {
        if (value.GetType() != nsVariantType::VariantDictionary)
        {
          value = nsVariantDictionary();
          continue;
        }
        nsVariantDictionary values = value.Get<nsVariantDictionary>();
        if (values.IsEmpty())
          continue;

        // Same conversion logic as for nsPropertyCategory::Member, but for each element instead.
        for (auto it2 = values.GetIterator(); it2.IsValid(); ++it2)
        {
          if (it2.Value().GetType() == SpecVarType)
          {
            continue;
          }
          else
          {
            nsResult res(NS_FAILURE);
            it2.Value() = it2.Value().ConvertTo(SpecVarType, &res);
            if (res == NS_FAILURE)
            {
              it2.Value() = nsReflectionUtils::GetDefaultValue(pProperty, it2.Key());
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

void nsReflectedTypeStorageManager::ReflectedTypeStorageMapping::AddPropertyToInstances(
  nsUInt32 uiIndex, const nsAbstractProperty* pProperty, nsSet<const nsDocumentObject*>& ref_requiresPatchingEmbeddedClass)
{
  if (pProperty->GetCategory() != nsPropertyCategory::Member)
    return;

  for (auto it = m_Instances.GetIterator(); it.IsValid(); ++it)
  {
    nsDynamicArray<nsVariant>& data = it.Key()->m_Data;
    NS_ASSERT_DEV(data.GetCount() == uiIndex, "nsReflectedTypeStorageAccessor found with a property count that does not match its storage mapping!");
    data.PushBack(nsToolsReflectionUtils::GetStorageDefault(pProperty));
    if (pProperty->GetFlags().IsSet(nsPropertyFlags::Class) && !pProperty->GetFlags().IsSet(nsPropertyFlags::Pointer))
    {
      ref_requiresPatchingEmbeddedClass.Insert(it.Key()->GetOwner());
    }
  }
}

////////////////////////////////////////////////////////////////////////
// nsReflectedTypeStorageManager private functions
////////////////////////////////////////////////////////////////////////

void nsReflectedTypeStorageManager::Startup()
{
  nsPhantomRttiManager::s_Events.AddEventHandler(TypeEventHandler);
}

void nsReflectedTypeStorageManager::Shutdown()
{
  nsPhantomRttiManager::s_Events.RemoveEventHandler(TypeEventHandler);

  for (auto it = s_ReflectedTypeToStorageMapping.GetIterator(); it.IsValid(); ++it)
  {
    ReflectedTypeStorageMapping* pMapping = it.Value();

    for (auto inst : pMapping->m_Instances)
    {
      nsLog::Error("Type '{0}' survived shutdown!", inst->GetType()->GetTypeName());
    }

    NS_ASSERT_DEV(pMapping->m_Instances.IsEmpty(), "A type was removed which still has instances using the type!");
    NS_DEFAULT_DELETE(pMapping);
  }
  s_ReflectedTypeToStorageMapping.Clear();
}

const nsReflectedTypeStorageManager::ReflectedTypeStorageMapping* nsReflectedTypeStorageManager::AddStorageAccessor(
  nsReflectedTypeStorageAccessor* pInstance)
{
  ReflectedTypeStorageMapping* pMapping = GetTypeStorageMapping(pInstance->GetType());
  pMapping->m_Instances.Insert(pInstance);
  return pMapping;
}

void nsReflectedTypeStorageManager::RemoveStorageAccessor(nsReflectedTypeStorageAccessor* pInstance)
{
  ReflectedTypeStorageMapping* pMapping = GetTypeStorageMapping(pInstance->GetType());
  pMapping->m_Instances.Remove(pInstance);
}

nsReflectedTypeStorageManager::ReflectedTypeStorageMapping* nsReflectedTypeStorageManager::GetTypeStorageMapping(const nsRTTI* pType)
{
  NS_ASSERT_DEV(pType != nullptr, "Nullptr is not a valid type!");
  auto it = s_ReflectedTypeToStorageMapping.Find(pType);
  if (it.IsValid())
    return it.Value();

  ReflectedTypeStorageMapping* pMapping = NS_DEFAULT_NEW(ReflectedTypeStorageMapping);
  pMapping->AddProperties(pType);
  s_ReflectedTypeToStorageMapping[pType] = pMapping;
  return pMapping;
}

void nsReflectedTypeStorageManager::TypeEventHandler(const nsPhantomRttiManagerEvent& e)
{
  switch (e.m_Type)
  {
    case nsPhantomRttiManagerEvent::Type::TypeAdded:
    {
      const nsRTTI* pType = e.m_pChangedType;
      NS_ASSERT_DEV(pType != nullptr, "A type was added but it has an invalid handle!");

      NS_ASSERT_DEV(!s_ReflectedTypeToStorageMapping.Find(e.m_pChangedType).IsValid(), "The type '{0}' was added twice!", pType->GetTypeName());
      GetTypeStorageMapping(e.m_pChangedType);
    }
    break;
    case nsPhantomRttiManagerEvent::Type::TypeChanged:
    {
      const nsRTTI* pNewType = e.m_pChangedType;
      NS_ASSERT_DEV(pNewType != nullptr, "A type was updated but its handle is invalid!");

      ReflectedTypeStorageMapping* pMapping = s_ReflectedTypeToStorageMapping[e.m_pChangedType];
      NS_ASSERT_DEV(pMapping != nullptr, "A type was updated but no mapping exists for it!");

      if (pNewType->GetParentType() != nullptr && pNewType->GetParentType()->GetTypeName() == "nsEnumBase")
      {
        // NS_ASSERT_DEV(false, "Updating enums not implemented yet!");
        break;
      }
      else if (pNewType->GetParentType() != nullptr && pNewType->GetParentType()->GetTypeName() == "nsBitflagsBase")
      {
        NS_ASSERT_DEV(false, "Updating bitflags not implemented yet!");
      }

      pMapping->AddProperties(pNewType);

      nsSet<nsRTTI*> dependencies;
      // Update all types that either derive from the changed type or have the type as a member.
      for (auto it = s_ReflectedTypeToStorageMapping.GetIterator(); it.IsValid(); ++it)
      {
        if (it.Key() == e.m_pChangedType)
          continue;

        const nsRTTI* pType = it.Key();
        if (pType->IsDerivedFrom(e.m_pChangedType))
        {
          it.Value()->AddProperties(pType);
        }
      }
    }
    break;
    case nsPhantomRttiManagerEvent::Type::TypeRemoved:
    {
      ReflectedTypeStorageMapping* pMapping = s_ReflectedTypeToStorageMapping[e.m_pChangedType];
      NS_ASSERT_DEV(pMapping != nullptr, "A type was removed but no mapping ever exited for it!");
      NS_ASSERT_DEV(pMapping->m_Instances.IsEmpty(), "A type was removed which still has instances using the type!");
      s_ReflectedTypeToStorageMapping.Remove(e.m_pChangedType);
      NS_DEFAULT_DELETE(pMapping);
    }
    break;
  }
}
