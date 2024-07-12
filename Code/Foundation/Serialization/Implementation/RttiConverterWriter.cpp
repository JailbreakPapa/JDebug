#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/Log.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <Foundation/Serialization/RttiConverter.h>
#include <Foundation/Types/ScopeExit.h>
#include <Foundation/Types/VariantTypeRegistry.h>

void nsRttiConverterContext::Clear()
{
  m_GuidToObject.Clear();
  m_ObjectToGuid.Clear();
  m_QueuedObjects.Clear();
}

void nsRttiConverterContext::OnUnknownTypeError(nsStringView sTypeName)
{
  nsLog::Error("RTTI type '{0}' is unknown, CreateObjectFromNode failed.", sTypeName);
}

nsUuid nsRttiConverterContext::GenerateObjectGuid(const nsUuid& parentGuid, const nsAbstractProperty* pProp, nsVariant index, void* pObject) const
{
  nsUuid guid = parentGuid;
  guid.HashCombine(nsUuid::MakeStableUuidFromString(pProp->GetPropertyName()));
  if (index.IsA<nsString>())
  {
    guid.HashCombine(nsUuid::MakeStableUuidFromString(index.Get<nsString>()));
  }
  else if (index.CanConvertTo<nsUInt32>())
  {
    guid.HashCombine(nsUuid::MakeStableUuidFromInt(index.ConvertTo<nsUInt32>()));
  }
  else if (index.IsValid())
  {
    NS_REPORT_FAILURE("Index type must be nsUInt32 or nsString.");
  }
  // nsLog::Warning("{0},{1},{2} -> {3}", parentGuid, pProp->GetPropertyName(), index, guid);
  return guid;
}

nsInternal::NewInstance<void> nsRttiConverterContext::CreateObject(const nsUuid& guid, const nsRTTI* pRtti)
{
  NS_ASSERT_DEBUG(pRtti != nullptr, "Cannot create object, RTTI type is unknown");
  if (!pRtti->GetAllocator() || !pRtti->GetAllocator()->CanAllocate())
    return nullptr;

  auto pObj = pRtti->GetAllocator()->Allocate<void>();
  RegisterObject(guid, pRtti, pObj);
  return pObj;
}

void nsRttiConverterContext::DeleteObject(const nsUuid& guid)
{
  auto object = GetObjectByGUID(guid);
  if (object.m_pObject)
  {
    object.m_pType->GetAllocator()->Deallocate(object.m_pObject);
  }
  UnregisterObject(guid);
}

void nsRttiConverterContext::RegisterObject(const nsUuid& guid, const nsRTTI* pRtti, void* pObject)
{
  NS_ASSERT_DEV(pObject != nullptr, "cannot register null object!");
  nsRttiConverterObject& co = m_GuidToObject[guid];

  if (pRtti->IsDerivedFrom<nsReflectedClass>())
  {
    pRtti = static_cast<nsReflectedClass*>(pObject)->GetDynamicRTTI();
  }

  // TODO: Actually remove child owner ptr from register when deleting an object
  // NS_ASSERT_DEV(co.m_pObject == nullptr || (co.m_pObject == pObject && co.m_pType == pRtti), "Registered same guid twice with different
  // values");

  co.m_pObject = pObject;
  co.m_pType = pRtti;

  m_ObjectToGuid[pObject] = guid;
}

void nsRttiConverterContext::UnregisterObject(const nsUuid& guid)
{
  nsRttiConverterObject* pObj;
  if (m_GuidToObject.TryGetValue(guid, pObj))
  {
    m_GuidToObject.Remove(guid);
    m_ObjectToGuid.Remove(pObj->m_pObject);
  }
}

nsRttiConverterObject nsRttiConverterContext::GetObjectByGUID(const nsUuid& guid) const
{
  nsRttiConverterObject object;
  m_GuidToObject.TryGetValue(guid, object);
  return object;
}

nsUuid nsRttiConverterContext::GetObjectGUID(const nsRTTI* pRtti, const void* pObject) const
{
  nsUuid guid;

  if (pObject != nullptr)
    m_ObjectToGuid.TryGetValue(pObject, guid);

  return guid;
}

const nsRTTI* nsRttiConverterContext::FindTypeByName(nsStringView sName) const
{
  return nsRTTI::FindTypeByName(sName);
}

nsUuid nsRttiConverterContext::EnqueObject(const nsUuid& guid, const nsRTTI* pRtti, void* pObject)
{
  NS_ASSERT_DEBUG(guid.IsValid(), "For stable serialization, guid must be well defined");
  nsUuid res = guid;

  if (pObject != nullptr)
  {
    // In the rare case that this succeeds we already encountered the object with a different guid before.
    // This can happen if two pointer owner point to the same object.
    if (!m_ObjectToGuid.TryGetValue(pObject, res))
    {
      RegisterObject(guid, pRtti, pObject);
    }

    m_QueuedObjects.Insert(res);
  }
  else
  {
    // Replace nullptr with invalid uuid.
    res = nsUuid();
  }
  return res;
}

nsRttiConverterObject nsRttiConverterContext::DequeueObject()
{
  if (!m_QueuedObjects.IsEmpty())
  {
    auto it = m_QueuedObjects.GetIterator();
    auto object = GetObjectByGUID(it.Key());
    NS_ASSERT_DEV(object.m_pObject != nullptr, "Enqueued object was never registered!");

    m_QueuedObjects.Remove(it);

    return object;
  }

  return nsRttiConverterObject();
}


nsRttiConverterWriter::nsRttiConverterWriter(nsAbstractObjectGraph* pGraph, nsRttiConverterContext* pContext, bool bSerializeReadOnly, bool bSerializeOwnerPtrs)
{
  m_pGraph = pGraph;
  m_pContext = pContext;

  m_Filter = [bSerializeReadOnly, bSerializeOwnerPtrs](const void* pObject, const nsAbstractProperty* pProp)
  {
    if (pProp->GetFlags().IsSet(nsPropertyFlags::ReadOnly) && !bSerializeReadOnly)
      return false;

    if (pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner) && !bSerializeOwnerPtrs)
      return false;

    return true;
  };
}

nsRttiConverterWriter::nsRttiConverterWriter(nsAbstractObjectGraph* pGraph, nsRttiConverterContext* pContext, FilterFunction filter)
  : m_pContext(pContext)
  , m_pGraph(pGraph)
  , m_Filter(filter)
{
  NS_ASSERT_DEBUG(filter.IsValid(), "Either filter function must be valid or a different ctor must be chosen.");
}

nsAbstractObjectNode* nsRttiConverterWriter::AddObjectToGraph(const nsRTTI* pRtti, const void* pObject, const char* szNodeName)
{
  const nsUuid guid = m_pContext->GetObjectGUID(pRtti, pObject);
  NS_ASSERT_DEV(guid.IsValid(), "The object was not registered. Call nsRttiConverterContext::RegisterObject before adding.");
  nsAbstractObjectNode* pNode = AddSubObjectToGraph(pRtti, pObject, guid, szNodeName);

  nsRttiConverterObject obj = m_pContext->DequeueObject();
  while (obj.m_pObject != nullptr)
  {
    const nsUuid objectGuid = m_pContext->GetObjectGUID(obj.m_pType, obj.m_pObject);
    AddSubObjectToGraph(obj.m_pType, obj.m_pObject, objectGuid, nullptr);

    obj = m_pContext->DequeueObject();
  }

  return pNode;
}

nsAbstractObjectNode* nsRttiConverterWriter::AddSubObjectToGraph(const nsRTTI* pRtti, const void* pObject, const nsUuid& guid, const char* szNodeName)
{
  nsAbstractObjectNode* pNode = m_pGraph->AddNode(guid, pRtti->GetTypeName(), pRtti->GetTypeVersion(), szNodeName);
  AddProperties(pNode, pRtti, pObject);
  return pNode;
}

void nsRttiConverterWriter::AddProperty(nsAbstractObjectNode* pNode, const nsAbstractProperty* pProp, const void* pObject)
{
  if (!m_Filter(pObject, pProp))
    return;

  nsVariant vTemp;
  nsStringBuilder sTemp;
  const nsRTTI* pPropType = pProp->GetSpecificType();
  const bool bIsValueType = nsReflectionUtils::IsValueType(pProp);

  switch (pProp->GetCategory())
  {
    case nsPropertyCategory::Member:
    {
      const nsAbstractMemberProperty* pSpecific = static_cast<const nsAbstractMemberProperty*>(pProp);

      if (pProp->GetFlags().IsSet(nsPropertyFlags::Pointer))
      {
        vTemp = nsReflectionUtils::GetMemberPropertyValue(pSpecific, pObject);
        void* pRefrencedObject = vTemp.ConvertTo<void*>();

        nsUuid guid = m_pContext->GenerateObjectGuid(pNode->GetGuid(), pProp, nsVariant(), pRefrencedObject);
        if (pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner))
        {
          guid = m_pContext->EnqueObject(guid, pPropType, pRefrencedObject);
          pNode->AddProperty(pProp->GetPropertyName(), guid);
        }
        else
        {
          guid = m_pContext->GetObjectGUID(pPropType, pRefrencedObject);
          pNode->AddProperty(pProp->GetPropertyName(), guid);
        }
      }
      else
      {
        if (pProp->GetFlags().IsAnySet(nsPropertyFlags::IsEnum | nsPropertyFlags::Bitflags))
        {
          vTemp = nsReflectionUtils::GetMemberPropertyValue(pSpecific, pObject);
          nsReflectionUtils::EnumerationToString(pPropType, vTemp.Get<nsInt64>(), sTemp);

          pNode->AddProperty(pProp->GetPropertyName(), sTemp.GetData());
        }
        else if (bIsValueType)
        {
          pNode->AddProperty(pProp->GetPropertyName(), nsReflectionUtils::GetMemberPropertyValue(pSpecific, pObject));
        }
        else if (pProp->GetFlags().IsSet(nsPropertyFlags::Class) && pPropType->GetProperties().GetCount() > 0)
        {
          void* pSubObject = pSpecific->GetPropertyPointer(pObject);


          // Do we have direct access to the property?
          if (pSubObject != nullptr)
          {
            const nsUuid SubObjectGuid = m_pContext->GenerateObjectGuid(pNode->GetGuid(), pProp, nsVariant(), pSubObject);
            pNode->AddProperty(pProp->GetPropertyName(), SubObjectGuid);

            AddSubObjectToGraph(pPropType, pSubObject, SubObjectGuid, nullptr);
          }
          // If the property is behind an accessor, we need to retrieve it first.
          else if (pPropType->GetAllocator()->CanAllocate())
          {
            pSubObject = pPropType->GetAllocator()->Allocate<void>();

            pSpecific->GetValuePtr(pObject, pSubObject);
            const nsUuid SubObjectGuid = m_pContext->GenerateObjectGuid(pNode->GetGuid(), pProp, nsVariant(), pSubObject);
            pNode->AddProperty(pProp->GetPropertyName(), SubObjectGuid);

            AddSubObjectToGraph(pPropType, pSubObject, SubObjectGuid, nullptr);

            pPropType->GetAllocator()->Deallocate(pSubObject);
          }
        }
      }
    }
    break;
    case nsPropertyCategory::Array:
    {
      const nsAbstractArrayProperty* pSpecific = static_cast<const nsAbstractArrayProperty*>(pProp);
      nsUInt32 uiCount = pSpecific->GetCount(pObject);
      nsVariantArray values;
      values.SetCount(uiCount);

      if (pSpecific->GetFlags().IsSet(nsPropertyFlags::Pointer))
      {
        for (nsUInt32 i = 0; i < uiCount; ++i)
        {
          vTemp = nsReflectionUtils::GetArrayPropertyValue(pSpecific, pObject, i);
          void* pRefrencedObject = vTemp.ConvertTo<void*>();

          nsUuid guid;
          if (pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner))
          {
            guid = m_pContext->GenerateObjectGuid(pNode->GetGuid(), pProp, i, pRefrencedObject);
            guid = m_pContext->EnqueObject(guid, pPropType, pRefrencedObject);
          }
          else
            guid = m_pContext->GetObjectGUID(pPropType, pRefrencedObject);

          values[i] = guid;
        }

        pNode->AddProperty(pProp->GetPropertyName(), values);
      }
      else
      {
        if (bIsValueType)
        {
          for (nsUInt32 i = 0; i < uiCount; ++i)
          {
            values[i] = nsReflectionUtils::GetArrayPropertyValue(pSpecific, pObject, i);
          }
          pNode->AddProperty(pProp->GetPropertyName(), values);
        }
        else if (pSpecific->GetFlags().IsSet(nsPropertyFlags::Class) && pPropType->GetAllocator()->CanAllocate())
        {
          void* pSubObject = pPropType->GetAllocator()->Allocate<void>();

          for (nsUInt32 i = 0; i < uiCount; ++i)
          {
            pSpecific->GetValue(pObject, i, pSubObject);
            const nsUuid SubObjectGuid = m_pContext->GenerateObjectGuid(pNode->GetGuid(), pProp, i, pSubObject);
            AddSubObjectToGraph(pPropType, pSubObject, SubObjectGuid, nullptr);

            values[i] = SubObjectGuid;
          }
          pNode->AddProperty(pProp->GetPropertyName(), values);
          pPropType->GetAllocator()->Deallocate(pSubObject);
        }
      }
    }
    break;
    case nsPropertyCategory::Set:
    {
      const nsAbstractSetProperty* pSpecific = static_cast<const nsAbstractSetProperty*>(pProp);

      nsHybridArray<nsVariant, 16> values;
      pSpecific->GetValues(pObject, values);

      nsVariantArray ValuesCopied(values);

      if (pProp->GetFlags().IsSet(nsPropertyFlags::Pointer))
      {
        for (nsUInt32 i = 0; i < values.GetCount(); ++i)
        {
          void* pRefrencedObject = values[i].ConvertTo<void*>();

          nsUuid guid;
          if (pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner))
          {
            // TODO: pointer sets are never stable unless they use an array based pseudo set as storage.
            guid = m_pContext->GenerateObjectGuid(pNode->GetGuid(), pProp, i, pRefrencedObject);
            guid = m_pContext->EnqueObject(guid, pPropType, pRefrencedObject);
          }
          else
            guid = m_pContext->GetObjectGUID(pPropType, pRefrencedObject);

          ValuesCopied[i] = guid;
        }

        pNode->AddProperty(pProp->GetPropertyName(), ValuesCopied);
      }
      else
      {
        if (bIsValueType)
        {
          pNode->AddProperty(pProp->GetPropertyName(), ValuesCopied);
        }
      }
    }
    break;
    case nsPropertyCategory::Map:
    {
      const nsAbstractMapProperty* pSpecific = static_cast<const nsAbstractMapProperty*>(pProp);

      nsHybridArray<nsString, 16> keys;
      pSpecific->GetKeys(pObject, keys);

      nsVariantDictionary ValuesCopied;
      ValuesCopied.Reserve(keys.GetCount());

      if (pProp->GetFlags().IsSet(nsPropertyFlags::Pointer))
      {
        for (nsUInt32 i = 0; i < keys.GetCount(); ++i)
        {
          nsVariant value = nsReflectionUtils::GetMapPropertyValue(pSpecific, pObject, keys[i]);
          void* pRefrencedObject = value.ConvertTo<void*>();

          nsUuid guid;
          if (pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner))
          {
            guid = m_pContext->GenerateObjectGuid(pNode->GetGuid(), pProp, nsVariant(keys[i]), pRefrencedObject);
            guid = m_pContext->EnqueObject(guid, pPropType, pRefrencedObject);
          }
          else
            guid = m_pContext->GetObjectGUID(pPropType, pRefrencedObject);

          ValuesCopied.Insert(keys[i], guid);
        }

        pNode->AddProperty(pProp->GetPropertyName(), ValuesCopied);
      }
      else
      {
        if (bIsValueType)
        {
          for (nsUInt32 i = 0; i < keys.GetCount(); ++i)
          {
            nsVariant value = nsReflectionUtils::GetMapPropertyValue(pSpecific, pObject, keys[i]);
            ValuesCopied.Insert(keys[i], value);
          }
          pNode->AddProperty(pProp->GetPropertyName(), ValuesCopied);
        }
        else if (pProp->GetFlags().IsSet(nsPropertyFlags::Class))
        {
          for (nsUInt32 i = 0; i < keys.GetCount(); ++i)
          {
            void* pSubObject = pPropType->GetAllocator()->Allocate<void>();
            NS_SCOPE_EXIT(pPropType->GetAllocator()->Deallocate(pSubObject););
            NS_VERIFY(pSpecific->GetValue(pObject, keys[i], pSubObject), "Key should be valid.");

            const nsUuid SubObjectGuid = m_pContext->GenerateObjectGuid(pNode->GetGuid(), pProp, nsVariant(keys[i]), pSubObject);
            AddSubObjectToGraph(pPropType, pSubObject, SubObjectGuid, nullptr);
            ValuesCopied.Insert(keys[i], SubObjectGuid);
          }
          pNode->AddProperty(pProp->GetPropertyName(), ValuesCopied);
        }
      }
    }
    break;
    case nsPropertyCategory::Constant:
      // Nothing to do here.
      break;
    default:
      break;
  }
}

void nsRttiConverterWriter::AddProperties(nsAbstractObjectNode* pNode, const nsRTTI* pRtti, const void* pObject)
{
  if (pRtti->GetParentType())
    AddProperties(pNode, pRtti->GetParentType(), pObject);

  for (const auto* pProp : pRtti->GetProperties())
  {
    AddProperty(pNode, pProp, pObject);
  }
}
