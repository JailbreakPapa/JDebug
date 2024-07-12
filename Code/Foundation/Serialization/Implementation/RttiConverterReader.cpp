#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/Log.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <Foundation/Serialization/RttiConverter.h>
#include <Foundation/Types/VariantTypeRegistry.h>

nsRttiConverterReader::nsRttiConverterReader(const nsAbstractObjectGraph* pGraph, nsRttiConverterContext* pContext)
{
  m_pGraph = pGraph;
  m_pContext = pContext;
}

nsInternal::NewInstance<void> nsRttiConverterReader::CreateObjectFromNode(const nsAbstractObjectNode* pNode)
{
  const nsRTTI* pRtti = m_pContext->FindTypeByName(pNode->GetType());
  if (pRtti == nullptr)
  {
    m_pContext->OnUnknownTypeError(pNode->GetType());
    return nullptr;
  }

  auto pObject = m_pContext->CreateObject(pNode->GetGuid(), pRtti);
  if (pObject)
  {
    ApplyPropertiesToObject(pNode, pRtti, pObject);
  }

  CallOnObjectCreated(pNode, pRtti, pObject);
  return pObject;
}

void nsRttiConverterReader::ApplyPropertiesToObject(const nsAbstractObjectNode* pNode, const nsRTTI* pRtti, void* pObject)
{
  NS_ASSERT_DEBUG(pNode != nullptr, "Invalid node");

  if (pRtti->GetParentType() != nullptr)
    ApplyPropertiesToObject(pNode, pRtti->GetParentType(), pObject);

  for (auto* prop : pRtti->GetProperties())
  {
    auto* pOtherProp = pNode->FindProperty(prop->GetPropertyName());
    if (pOtherProp == nullptr)
      continue;

    ApplyProperty(pObject, prop, pOtherProp);
  }
}

void nsRttiConverterReader::ApplyProperty(void* pObject, const nsAbstractProperty* pProp, const nsAbstractObjectNode::Property* pSource)
{
  const nsRTTI* pPropType = pProp->GetSpecificType();

  if (pProp->GetFlags().IsSet(nsPropertyFlags::ReadOnly))
    return;

  const bool bIsValueType = nsReflectionUtils::IsValueType(pProp);

  switch (pProp->GetCategory())
  {
    case nsPropertyCategory::Member:
    {
      auto pSpecific = static_cast<const nsAbstractMemberProperty*>(pProp);

      if (pProp->GetFlags().IsSet(nsPropertyFlags::Pointer))
      {
        if (!pSource->m_Value.IsA<nsUuid>())
          return;

        nsUuid guid = pSource->m_Value.Get<nsUuid>();
        void* pRefrencedObject = nullptr;

        if (guid.IsValid())
        {
          if (pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner))
          {
            auto* pNode = m_pGraph->GetNode(guid);
            NS_ASSERT_DEV(pNode != nullptr, "node must exist");
            pRefrencedObject = CreateObjectFromNode(pNode);
            if (pRefrencedObject == nullptr)
            {
              // nsLog::Error("Failed to set property '{0}', type could not be created!", pProp->GetPropertyName());
              return;
            }
          }
          else
          {
            pRefrencedObject = m_pContext->GetObjectByGUID(guid).m_pObject;
          }
        }

        void* pOldObject = nullptr;
        pSpecific->GetValuePtr(pObject, &pOldObject);
        pSpecific->SetValuePtr(pObject, &pRefrencedObject);
        if (pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner))
          nsReflectionUtils::DeleteObject(pOldObject, pProp);
      }
      else
      {
        if (bIsValueType || pProp->GetFlags().IsAnySet(nsPropertyFlags::IsEnum | nsPropertyFlags::Bitflags))
        {
          nsReflectionUtils::SetMemberPropertyValue(pSpecific, pObject, pSource->m_Value);
        }
        else if (pProp->GetFlags().IsSet(nsPropertyFlags::Class))
        {
          if (!pSource->m_Value.IsA<nsUuid>())
            return;

          void* pDirectPtr = pSpecific->GetPropertyPointer(pObject);
          bool bDelete = false;
          const nsUuid sourceGuid = pSource->m_Value.Get<nsUuid>();

          if (pDirectPtr == nullptr)
          {
            bDelete = true;
            pDirectPtr = m_pContext->CreateObject(sourceGuid, pPropType);
          }

          auto* pNode = m_pGraph->GetNode(sourceGuid);
          NS_ASSERT_DEV(pNode != nullptr, "node must exist");

          ApplyPropertiesToObject(pNode, pPropType, pDirectPtr);

          if (bDelete)
          {
            pSpecific->SetValuePtr(pObject, pDirectPtr);
            m_pContext->DeleteObject(sourceGuid);
          }
        }
      }
    }
    break;
    case nsPropertyCategory::Array:
    {
      auto pSpecific = static_cast<const nsAbstractArrayProperty*>(pProp);
      if (!pSource->m_Value.IsA<nsVariantArray>())
        return;
      const nsVariantArray& array = pSource->m_Value.Get<nsVariantArray>();
      // Delete old values
      if (pProp->GetFlags().AreAllSet(nsPropertyFlags::Pointer | nsPropertyFlags::PointerOwner))
      {
        const nsInt32 uiOldCount = (nsInt32)pSpecific->GetCount(pObject);
        for (nsInt32 i = uiOldCount - 1; i >= 0; --i)
        {
          void* pOldObject = nullptr;
          pSpecific->GetValue(pObject, i, &pOldObject);
          pSpecific->Remove(pObject, i);
          if (pOldObject)
            nsReflectionUtils::DeleteObject(pOldObject, pProp);
        }
      }

      pSpecific->SetCount(pObject, array.GetCount());
      if (pProp->GetFlags().IsAnySet(nsPropertyFlags::Pointer))
      {
        for (nsUInt32 i = 0; i < array.GetCount(); ++i)
        {
          if (!array[i].IsA<nsUuid>())
            continue;
          nsUuid guid = array[i].Get<nsUuid>();
          void* pRefrencedObject = nullptr;
          if (pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner))
          {
            if (guid.IsValid())
            {
              auto* pNode = m_pGraph->GetNode(guid);
              NS_ASSERT_DEV(pNode != nullptr, "node must exist");
              pRefrencedObject = CreateObjectFromNode(pNode);
              if (pRefrencedObject == nullptr)
              {
                nsLog::Error("Failed to set array property '{0}' element, type could not be created!", pProp->GetPropertyName());
                continue;
              }
            }
          }
          else
          {
            pRefrencedObject = m_pContext->GetObjectByGUID(guid).m_pObject;
          }
          pSpecific->SetValue(pObject, i, &pRefrencedObject);
        }
      }
      else
      {
        if (bIsValueType)
        {
          for (nsUInt32 i = 0; i < array.GetCount(); ++i)
          {
            nsReflectionUtils::SetArrayPropertyValue(pSpecific, pObject, i, array[i]);
          }
        }
        else if (pProp->GetFlags().IsAnySet(nsPropertyFlags::Class))
        {
          const nsUuid temp = nsUuid::MakeUuid();

          void* pValuePtr = m_pContext->CreateObject(temp, pPropType);

          for (nsUInt32 i = 0; i < array.GetCount(); ++i)
          {
            if (!array[i].IsA<nsUuid>())
              continue;

            const nsUuid sourceGuid = array[i].Get<nsUuid>();
            auto* pNode = m_pGraph->GetNode(sourceGuid);
            NS_ASSERT_DEV(pNode != nullptr, "node must exist");

            ApplyPropertiesToObject(pNode, pPropType, pValuePtr);
            pSpecific->SetValue(pObject, i, pValuePtr);
          }

          m_pContext->DeleteObject(temp);
        }
      }
    }
    break;
    case nsPropertyCategory::Set:
    {
      auto pSpecific = static_cast<const nsAbstractSetProperty*>(pProp);
      if (!pSource->m_Value.IsA<nsVariantArray>())
        return;

      const nsVariantArray& array = pSource->m_Value.Get<nsVariantArray>();

      // Delete old values
      if (pProp->GetFlags().AreAllSet(nsPropertyFlags::Pointer | nsPropertyFlags::PointerOwner))
      {
        nsHybridArray<nsVariant, 16> keys;
        pSpecific->GetValues(pObject, keys);
        pSpecific->Clear(pObject);
        for (nsVariant& value : keys)
        {
          void* pOldObject = value.ConvertTo<void*>();
          if (pOldObject)
            nsReflectionUtils::DeleteObject(pOldObject, pProp);
        }
      }

      pSpecific->Clear(pObject);

      if (pProp->GetFlags().IsAnySet(nsPropertyFlags::Pointer))
      {
        for (nsUInt32 i = 0; i < array.GetCount(); ++i)
        {
          if (!array[i].IsA<nsUuid>())
            continue;

          nsUuid guid = array[i].Get<nsUuid>();
          void* pRefrencedObject = nullptr;
          if (pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner))
          {
            auto* pNode = m_pGraph->GetNode(guid);
            NS_ASSERT_DEV(pNode != nullptr, "node must exist");
            pRefrencedObject = CreateObjectFromNode(pNode);
            if (pRefrencedObject == nullptr)
            {
              nsLog::Error("Failed to insert set element into property '{0}', type could not be created!", pProp->GetPropertyName());
              continue;
            }
          }
          else
          {
            pRefrencedObject = m_pContext->GetObjectByGUID(guid).m_pObject;
          }
          pSpecific->Insert(pObject, &pRefrencedObject);
        }
      }
      else
      {
        if (bIsValueType)
        {
          for (nsUInt32 i = 0; i < array.GetCount(); ++i)
          {
            nsReflectionUtils::InsertSetPropertyValue(pSpecific, pObject, array[i]);
          }
        }
        else if (pProp->GetFlags().IsAnySet(nsPropertyFlags::Class))
        {
          const nsUuid temp = nsUuid::MakeUuid();

          void* pValuePtr = m_pContext->CreateObject(temp, pPropType);

          for (nsUInt32 i = 0; i < array.GetCount(); ++i)
          {
            if (!array[i].IsA<nsUuid>())
              continue;

            const nsUuid sourceGuid = array[i].Get<nsUuid>();
            auto* pNode = m_pGraph->GetNode(sourceGuid);
            NS_ASSERT_DEV(pNode != nullptr, "node must exist");

            ApplyPropertiesToObject(pNode, pPropType, pValuePtr);
            pSpecific->Insert(pObject, pValuePtr);
          }

          m_pContext->DeleteObject(temp);
        }
      }
    }
    break;
    case nsPropertyCategory::Map:
    {
      auto pSpecific = static_cast<const nsAbstractMapProperty*>(pProp);
      if (!pSource->m_Value.IsA<nsVariantDictionary>())
        return;

      const nsVariantDictionary& dict = pSource->m_Value.Get<nsVariantDictionary>();

      // Delete old values
      if (pProp->GetFlags().AreAllSet(nsPropertyFlags::Pointer | nsPropertyFlags::PointerOwner))
      {
        nsHybridArray<nsString, 16> keys;
        pSpecific->GetKeys(pObject, keys);
        for (const nsString& sKey : keys)
        {
          nsVariant value = nsReflectionUtils::GetMapPropertyValue(pSpecific, pObject, sKey);
          void* pOldClone = value.ConvertTo<void*>();
          pSpecific->Remove(pObject, sKey);
          if (pOldClone)
            nsReflectionUtils::DeleteObject(pOldClone, pProp);
        }
      }

      pSpecific->Clear(pObject);

      if (pProp->GetFlags().IsAnySet(nsPropertyFlags::Pointer))
      {
        for (auto it = dict.GetIterator(); it.IsValid(); ++it)
        {
          if (!it.Value().IsA<nsUuid>())
            continue;

          nsUuid guid = it.Value().Get<nsUuid>();
          void* pRefrencedObject = nullptr;
          if (pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner))
          {
            if (guid.IsValid())
            {
              auto* pNode = m_pGraph->GetNode(guid);
              NS_ASSERT_DEV(pNode != nullptr, "node must exist");
              pRefrencedObject = CreateObjectFromNode(pNode);
              if (pRefrencedObject == nullptr)
              {
                nsLog::Error("Failed to insert set element into property '{0}', type could not be created!", pProp->GetPropertyName());
                continue;
              }
            }
          }
          else
          {
            pRefrencedObject = m_pContext->GetObjectByGUID(guid).m_pObject;
          }
          pSpecific->Insert(pObject, it.Key(), &pRefrencedObject);
        }
      }
      else
      {
        if (bIsValueType)
        {
          for (auto it = dict.GetIterator(); it.IsValid(); ++it)
          {
            nsReflectionUtils::SetMapPropertyValue(pSpecific, pObject, it.Key(), it.Value());
          }
        }
        else if (pProp->GetFlags().IsAnySet(nsPropertyFlags::Class))
        {
          const nsUuid temp = nsUuid::MakeUuid();

          void* pValuePtr = m_pContext->CreateObject(temp, pPropType);

          for (auto it = dict.GetIterator(); it.IsValid(); ++it)
          {
            if (!it.Value().IsA<nsUuid>())
              continue;

            const nsUuid sourceGuid = it.Value().Get<nsUuid>();
            auto* pNode = m_pGraph->GetNode(sourceGuid);
            NS_ASSERT_DEV(pNode != nullptr, "node must exist");

            ApplyPropertiesToObject(pNode, pPropType, pValuePtr);
            pSpecific->Insert(pObject, it.Key(), pValuePtr);
          }

          m_pContext->DeleteObject(temp);
        }
      }
    }
    break;

    default:
      NS_ASSERT_NOT_IMPLEMENTED;
      break;
  }
}

void nsRttiConverterReader::CallOnObjectCreated(const nsAbstractObjectNode* pNode, const nsRTTI* pRtti, void* pObject)
{
  auto functions = pRtti->GetFunctions();
  for (auto pFunc : functions)
  {
    // TODO: Make this compare faster
    if (nsStringUtils::IsEqual(pFunc->GetPropertyName(), "OnObjectCreated"))
    {
      nsHybridArray<nsVariant, 1> params;
      params.PushBack(nsVariant(pNode));
      nsVariant ret;
      pFunc->Execute(pObject, params, ret);
    }
  }
}
