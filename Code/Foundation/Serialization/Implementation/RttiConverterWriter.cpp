#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/Log.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <Foundation/Serialization/RttiConverter.h>
#include <Foundation/Types/ScopeExit.h>
#include <Foundation/Types/VariantTypeRegistry.h>

void wdRttiConverterContext::Clear()
{
  m_GuidToObject.Clear();
  m_ObjectToGuid.Clear();
  m_QueuedObjects.Clear();
}

void wdRttiConverterContext::OnUnknownTypeError(wdStringView sTypeName)
{
  wdLog::Error("RTTI type '{0}' is unknown, CreateObjectFromNode failed.", sTypeName);
}

wdUuid wdRttiConverterContext::GenerateObjectGuid(const wdUuid& parentGuid, const wdAbstractProperty* pProp, wdVariant index, void* pObject) const
{
  wdUuid guid = parentGuid;
  guid.HashCombine(wdUuid::StableUuidForString(pProp->GetPropertyName()));
  if (index.IsA<wdString>())
  {
    guid.HashCombine(wdUuid::StableUuidForString(index.Get<wdString>()));
  }
  else if (index.CanConvertTo<wdUInt32>())
  {
    guid.HashCombine(wdUuid::StableUuidForInt(index.ConvertTo<wdUInt32>()));
  }
  else if (index.IsValid())
  {
    WD_REPORT_FAILURE("Index type must be wdUInt32 or wdString.");
  }
  // wdLog::Warning("{0},{1},{2} -> {3}", parentGuid, pProp->GetPropertyName(), index, guid);
  return guid;
}

wdInternal::NewInstance<void> wdRttiConverterContext::CreateObject(const wdUuid& guid, const wdRTTI* pRtti)
{
  WD_ASSERT_DEBUG(pRtti != nullptr, "Cannot create object, RTTI type is unknown");
  if (!pRtti->GetAllocator() || !pRtti->GetAllocator()->CanAllocate())
    return nullptr;

  auto pObj = pRtti->GetAllocator()->Allocate<void>();
  RegisterObject(guid, pRtti, pObj);
  return pObj;
}

void wdRttiConverterContext::DeleteObject(const wdUuid& guid)
{
  auto object = GetObjectByGUID(guid);
  if (object.m_pObject)
  {
    object.m_pType->GetAllocator()->Deallocate(object.m_pObject);
  }
  UnregisterObject(guid);
}

void wdRttiConverterContext::RegisterObject(const wdUuid& guid, const wdRTTI* pRtti, void* pObject)
{
  WD_ASSERT_DEV(pObject != nullptr, "cannot register null object!");
  wdRttiConverterObject& co = m_GuidToObject[guid];

  if (pRtti->IsDerivedFrom<wdReflectedClass>())
  {
    pRtti = static_cast<wdReflectedClass*>(pObject)->GetDynamicRTTI();
  }

  // TODO: Actually remove child owner ptr from register when deleting an object
  // WD_ASSERT_DEV(co.m_pObject == nullptr || (co.m_pObject == pObject && co.m_pType == pRtti), "Registered same guid twice with different
  // values");

  co.m_pObject = pObject;
  co.m_pType = pRtti;

  m_ObjectToGuid[pObject] = guid;
}

void wdRttiConverterContext::UnregisterObject(const wdUuid& guid)
{
  wdRttiConverterObject* pObj;
  if (m_GuidToObject.TryGetValue(guid, pObj))
  {
    m_GuidToObject.Remove(guid);
    m_ObjectToGuid.Remove(pObj->m_pObject);
  }
}

wdRttiConverterObject wdRttiConverterContext::GetObjectByGUID(const wdUuid& guid) const
{
  wdRttiConverterObject object;
  m_GuidToObject.TryGetValue(guid, object);
  return object;
}

wdUuid wdRttiConverterContext::GetObjectGUID(const wdRTTI* pRtti, const void* pObject) const
{
  wdUuid guid;

  if (pObject != nullptr)
    m_ObjectToGuid.TryGetValue(pObject, guid);

  return guid;
}

wdUuid wdRttiConverterContext::EnqueObject(const wdUuid& guid, const wdRTTI* pRtti, void* pObject)
{
  WD_ASSERT_DEBUG(guid.IsValid(), "For stable serialization, guid must be well defined");
  wdUuid res = guid;

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
    res = wdUuid();
  }
  return res;
}

wdRttiConverterObject wdRttiConverterContext::DequeueObject()
{
  if (!m_QueuedObjects.IsEmpty())
  {
    auto it = m_QueuedObjects.GetIterator();
    auto object = GetObjectByGUID(it.Key());
    WD_ASSERT_DEV(object.m_pObject != nullptr, "Enqueued object was never registered!");

    m_QueuedObjects.Remove(it);

    return object;
  }

  return wdRttiConverterObject();
}


wdRttiConverterWriter::wdRttiConverterWriter(wdAbstractObjectGraph* pGraph, wdRttiConverterContext* pContext, bool bSerializeReadOnly, bool bSerializeOwnerPtrs)
{
  m_pGraph = pGraph;
  m_pContext = pContext;

  m_Filter = [bSerializeReadOnly, bSerializeOwnerPtrs](const void* pObject, const wdAbstractProperty* pProp) {
    if (pProp->GetFlags().IsSet(wdPropertyFlags::ReadOnly) && !bSerializeReadOnly)
      return false;

    if (pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner) && !bSerializeOwnerPtrs)
      return false;

    return true;
  };
}

wdRttiConverterWriter::wdRttiConverterWriter(wdAbstractObjectGraph* pGraph, wdRttiConverterContext* pContext, FilterFunction filter)
{
  WD_ASSERT_DEBUG(filter.IsValid(), "Either filter function must be valid or a different ctor must be chosen.");
  m_pGraph = pGraph;
  m_pContext = pContext;
  m_Filter = filter;
}

wdAbstractObjectNode* wdRttiConverterWriter::AddObjectToGraph(const wdRTTI* pRtti, const void* pObject, const char* szNodeName)
{
  const wdUuid guid = m_pContext->GetObjectGUID(pRtti, pObject);
  WD_ASSERT_DEV(guid.IsValid(), "The object was not registered. Call wdRttiConverterContext::RegisterObject before adding.");
  wdAbstractObjectNode* pNode = AddSubObjectToGraph(pRtti, pObject, guid, szNodeName);

  wdRttiConverterObject obj = m_pContext->DequeueObject();
  while (obj.m_pObject != nullptr)
  {
    const wdUuid objectGuid = m_pContext->GetObjectGUID(obj.m_pType, obj.m_pObject);
    AddSubObjectToGraph(obj.m_pType, obj.m_pObject, objectGuid, nullptr);

    obj = m_pContext->DequeueObject();
  }

  return pNode;
}

wdAbstractObjectNode* wdRttiConverterWriter::AddSubObjectToGraph(const wdRTTI* pRtti, const void* pObject, const wdUuid& guid, const char* szNodeName)
{
  wdAbstractObjectNode* pNode = m_pGraph->AddNode(guid, pRtti->GetTypeName(), pRtti->GetTypeVersion(), szNodeName);
  AddProperties(pNode, pRtti, pObject);
  return pNode;
}

void wdRttiConverterWriter::AddProperty(wdAbstractObjectNode* pNode, const wdAbstractProperty* pProp, const void* pObject)
{
  if (!m_Filter(pObject, pProp))
    return;

  wdVariant vTemp;
  wdStringBuilder sTemp;
  const wdRTTI* pPropType = pProp->GetSpecificType();
  const bool bIsValueType = wdReflectionUtils::IsValueType(pProp);

  switch (pProp->GetCategory())
  {
    case wdPropertyCategory::Member:
    {
      const wdAbstractMemberProperty* pSpecific = static_cast<const wdAbstractMemberProperty*>(pProp);

      if (pProp->GetFlags().IsSet(wdPropertyFlags::Pointer))
      {
        vTemp = wdReflectionUtils::GetMemberPropertyValue(pSpecific, pObject);
        void* pRefrencedObject = vTemp.ConvertTo<void*>();

        wdUuid guid = m_pContext->GenerateObjectGuid(pNode->GetGuid(), pProp, wdVariant(), pRefrencedObject);
        if (pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner))
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
        if (pProp->GetFlags().IsAnySet(wdPropertyFlags::IsEnum | wdPropertyFlags::Bitflags))
        {
          vTemp = wdReflectionUtils::GetMemberPropertyValue(pSpecific, pObject);
          wdReflectionUtils::EnumerationToString(pPropType, vTemp.Get<wdInt64>(), sTemp);

          pNode->AddProperty(pProp->GetPropertyName(), sTemp.GetData());
        }
        else if (bIsValueType)
        {
          pNode->AddProperty(pProp->GetPropertyName(), wdReflectionUtils::GetMemberPropertyValue(pSpecific, pObject));
        }
        else if (pProp->GetFlags().IsSet(wdPropertyFlags::Class) && pPropType->GetProperties().GetCount() > 0)
        {
          void* pSubObject = pSpecific->GetPropertyPointer(pObject);


          // Do we have direct access to the property?
          if (pSubObject != nullptr)
          {
            const wdUuid SubObjectGuid = m_pContext->GenerateObjectGuid(pNode->GetGuid(), pProp, wdVariant(), pSubObject);
            pNode->AddProperty(pProp->GetPropertyName(), SubObjectGuid);

            AddSubObjectToGraph(pPropType, pSubObject, SubObjectGuid, nullptr);
          }
          // If the property is behind an accessor, we need to retrieve it first.
          else if (pPropType->GetAllocator()->CanAllocate())
          {
            pSubObject = pPropType->GetAllocator()->Allocate<void>();

            pSpecific->GetValuePtr(pObject, pSubObject);
            const wdUuid SubObjectGuid = m_pContext->GenerateObjectGuid(pNode->GetGuid(), pProp, wdVariant(), pSubObject);
            pNode->AddProperty(pProp->GetPropertyName(), SubObjectGuid);

            AddSubObjectToGraph(pPropType, pSubObject, SubObjectGuid, nullptr);

            pPropType->GetAllocator()->Deallocate(pSubObject);
          }
        }
      }
    }
    break;
    case wdPropertyCategory::Array:
    {
      const wdAbstractArrayProperty* pSpecific = static_cast<const wdAbstractArrayProperty*>(pProp);
      wdUInt32 uiCount = pSpecific->GetCount(pObject);
      wdVariantArray values;
      values.SetCount(uiCount);

      if (pSpecific->GetFlags().IsSet(wdPropertyFlags::Pointer))
      {
        for (wdUInt32 i = 0; i < uiCount; ++i)
        {
          vTemp = wdReflectionUtils::GetArrayPropertyValue(pSpecific, pObject, i);
          void* pRefrencedObject = vTemp.ConvertTo<void*>();

          wdUuid guid;
          if (pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner))
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
          for (wdUInt32 i = 0; i < uiCount; ++i)
          {
            values[i] = wdReflectionUtils::GetArrayPropertyValue(pSpecific, pObject, i);
          }
          pNode->AddProperty(pProp->GetPropertyName(), values);
        }
        else if (pSpecific->GetFlags().IsSet(wdPropertyFlags::Class) && pPropType->GetAllocator()->CanAllocate())
        {
          void* pSubObject = pPropType->GetAllocator()->Allocate<void>();

          for (wdUInt32 i = 0; i < uiCount; ++i)
          {
            pSpecific->GetValue(pObject, i, pSubObject);
            const wdUuid SubObjectGuid = m_pContext->GenerateObjectGuid(pNode->GetGuid(), pProp, i, pSubObject);
            AddSubObjectToGraph(pPropType, pSubObject, SubObjectGuid, nullptr);

            values[i] = SubObjectGuid;
          }
          pNode->AddProperty(pProp->GetPropertyName(), values);
          pPropType->GetAllocator()->Deallocate(pSubObject);
        }
      }
    }
    break;
    case wdPropertyCategory::Set:
    {
      const wdAbstractSetProperty* pSpecific = static_cast<const wdAbstractSetProperty*>(pProp);

      wdHybridArray<wdVariant, 16> values;
      pSpecific->GetValues(pObject, values);

      wdVariantArray ValuesCopied(values);

      if (pProp->GetFlags().IsSet(wdPropertyFlags::Pointer))
      {
        for (wdUInt32 i = 0; i < values.GetCount(); ++i)
        {
          void* pRefrencedObject = values[i].ConvertTo<void*>();

          wdUuid guid;
          if (pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner))
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
    case wdPropertyCategory::Map:
    {
      const wdAbstractMapProperty* pSpecific = static_cast<const wdAbstractMapProperty*>(pProp);

      wdHybridArray<wdString, 16> keys;
      pSpecific->GetKeys(pObject, keys);

      wdVariantDictionary ValuesCopied;
      ValuesCopied.Reserve(keys.GetCount());

      if (pProp->GetFlags().IsSet(wdPropertyFlags::Pointer))
      {
        for (wdUInt32 i = 0; i < keys.GetCount(); ++i)
        {
          wdVariant value = wdReflectionUtils::GetMapPropertyValue(pSpecific, pObject, keys[i]);
          void* pRefrencedObject = value.ConvertTo<void*>();

          wdUuid guid;
          if (pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner))
          {
            guid = m_pContext->GenerateObjectGuid(pNode->GetGuid(), pProp, wdVariant(keys[i]), pRefrencedObject);
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
          for (wdUInt32 i = 0; i < keys.GetCount(); ++i)
          {
            wdVariant value = wdReflectionUtils::GetMapPropertyValue(pSpecific, pObject, keys[i]);
            ValuesCopied.Insert(keys[i], value);
          }
          pNode->AddProperty(pProp->GetPropertyName(), ValuesCopied);
        }
        else if (pProp->GetFlags().IsSet(wdPropertyFlags::Class))
        {
          for (wdUInt32 i = 0; i < keys.GetCount(); ++i)
          {
            void* pSubObject = pPropType->GetAllocator()->Allocate<void>();
            WD_SCOPE_EXIT(pPropType->GetAllocator()->Deallocate(pSubObject););
            WD_VERIFY(pSpecific->GetValue(pObject, keys[i], pSubObject), "Key should be valid.");

            const wdUuid SubObjectGuid = m_pContext->GenerateObjectGuid(pNode->GetGuid(), pProp, wdVariant(keys[i]), pSubObject);
            AddSubObjectToGraph(pPropType, pSubObject, SubObjectGuid, nullptr);
            ValuesCopied.Insert(keys[i], SubObjectGuid);
          }
          pNode->AddProperty(pProp->GetPropertyName(), ValuesCopied);
        }
      }
    }
    break;
    case wdPropertyCategory::Constant:
      // Nothing to do here.
      break;
    default:
      break;
  }
}

void wdRttiConverterWriter::AddProperties(wdAbstractObjectNode* pNode, const wdRTTI* pRtti, const void* pObject)
{
  if (pRtti->GetParentType())
    AddProperties(pNode, pRtti->GetParentType(), pObject);

  for (const auto* pProp : pRtti->GetProperties())
  {
    AddProperty(pNode, pProp, pObject);
  }
}



WD_STATICLINK_FILE(Foundation, Foundation_Serialization_Implementation_RttiConverterWriter);
