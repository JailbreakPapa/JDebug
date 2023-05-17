#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/Log.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <Foundation/Serialization/RttiConverter.h>
#include <Foundation/Types/VariantTypeRegistry.h>

wdRttiConverterReader::wdRttiConverterReader(const wdAbstractObjectGraph* pGraph, wdRttiConverterContext* pContext)
{
  m_pGraph = pGraph;
  m_pContext = pContext;
}

wdInternal::NewInstance<void> wdRttiConverterReader::CreateObjectFromNode(const wdAbstractObjectNode* pNode)
{
  const wdRTTI* pRtti = wdRTTI::FindTypeByName(pNode->GetType());
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

void wdRttiConverterReader::ApplyPropertiesToObject(const wdAbstractObjectNode* pNode, const wdRTTI* pRtti, void* pObject)
{
  WD_ASSERT_DEBUG(pNode != nullptr, "Invalid node");

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

void wdRttiConverterReader::ApplyProperty(void* pObject, wdAbstractProperty* pProp, const wdAbstractObjectNode::Property* pSource)
{
  const wdRTTI* pPropType = pProp->GetSpecificType();

  if (pProp->GetFlags().IsSet(wdPropertyFlags::ReadOnly))
    return;

  const bool bIsValueType = wdReflectionUtils::IsValueType(pProp);

  switch (pProp->GetCategory())
  {
    case wdPropertyCategory::Member:
    {
      wdAbstractMemberProperty* pSpecific = static_cast<wdAbstractMemberProperty*>(pProp);

      if (pProp->GetFlags().IsSet(wdPropertyFlags::Pointer))
      {
        if (!pSource->m_Value.IsA<wdUuid>())
          return;

        wdUuid guid = pSource->m_Value.Get<wdUuid>();
        void* pRefrencedObject = nullptr;

        if (guid.IsValid())
        {
          if (pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner))
          {
            auto* pNode = m_pGraph->GetNode(guid);
            WD_ASSERT_DEV(pNode != nullptr, "node must exist");
            pRefrencedObject = CreateObjectFromNode(pNode);
            if (pRefrencedObject == nullptr)
            {
              // wdLog::Error("Failed to set property '{0}', type could not be created!", pProp->GetPropertyName());
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
        if (pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner))
          wdReflectionUtils::DeleteObject(pOldObject, pProp);
      }
      else
      {
        if (bIsValueType || pProp->GetFlags().IsAnySet(wdPropertyFlags::IsEnum | wdPropertyFlags::Bitflags))
        {
          wdReflectionUtils::SetMemberPropertyValue(pSpecific, pObject, pSource->m_Value);
        }
        else if (pProp->GetFlags().IsSet(wdPropertyFlags::Class))
        {
          if (!pSource->m_Value.IsA<wdUuid>())
            return;

          void* pDirectPtr = pSpecific->GetPropertyPointer(pObject);
          bool bDelete = false;
          const wdUuid sourceGuid = pSource->m_Value.Get<wdUuid>();

          if (pDirectPtr == nullptr)
          {
            bDelete = true;
            pDirectPtr = m_pContext->CreateObject(sourceGuid, pPropType);
          }

          auto* pNode = m_pGraph->GetNode(sourceGuid);
          WD_ASSERT_DEV(pNode != nullptr, "node must exist");

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
    case wdPropertyCategory::Array:
    {
      wdAbstractArrayProperty* pSpecific = static_cast<wdAbstractArrayProperty*>(pProp);
      if (!pSource->m_Value.IsA<wdVariantArray>())
        return;
      const wdVariantArray& array = pSource->m_Value.Get<wdVariantArray>();
      // Delete old values
      if (pProp->GetFlags().AreAllSet(wdPropertyFlags::Pointer | wdPropertyFlags::PointerOwner))
      {
        const wdInt32 uiOldCount = (wdInt32)pSpecific->GetCount(pObject);
        for (wdInt32 i = uiOldCount - 1; i >= 0; --i)
        {
          void* pOldObject = nullptr;
          pSpecific->GetValue(pObject, i, &pOldObject);
          pSpecific->Remove(pObject, i);
          if (pOldObject)
            wdReflectionUtils::DeleteObject(pOldObject, pProp);
        }
      }

      pSpecific->SetCount(pObject, array.GetCount());
      if (pProp->GetFlags().IsAnySet(wdPropertyFlags::Pointer))
      {
        for (wdUInt32 i = 0; i < array.GetCount(); ++i)
        {
          if (!array[i].IsA<wdUuid>())
            continue;
          wdUuid guid = array[i].Get<wdUuid>();
          void* pRefrencedObject = nullptr;
          if (pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner))
          {
            if (guid.IsValid())
            {
              auto* pNode = m_pGraph->GetNode(guid);
              WD_ASSERT_DEV(pNode != nullptr, "node must exist");
              pRefrencedObject = CreateObjectFromNode(pNode);
              if (pRefrencedObject == nullptr)
              {
                wdLog::Error("Failed to set array property '{0}' element, type could not be created!", pProp->GetPropertyName());
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
          for (wdUInt32 i = 0; i < array.GetCount(); ++i)
          {
            wdReflectionUtils::SetArrayPropertyValue(pSpecific, pObject, i, array[i]);
          }
        }
        else if (pProp->GetFlags().IsAnySet(wdPropertyFlags::Class))
        {
          wdUuid temp;
          temp.CreateNewUuid();
          void* pValuePtr = m_pContext->CreateObject(temp, pPropType);

          for (wdUInt32 i = 0; i < array.GetCount(); ++i)
          {
            if (!array[i].IsA<wdUuid>())
              continue;

            const wdUuid sourceGuid = array[i].Get<wdUuid>();
            auto* pNode = m_pGraph->GetNode(sourceGuid);
            WD_ASSERT_DEV(pNode != nullptr, "node must exist");

            ApplyPropertiesToObject(pNode, pPropType, pValuePtr);
            pSpecific->SetValue(pObject, i, pValuePtr);
          }

          m_pContext->DeleteObject(temp);
        }
      }
    }
    break;
    case wdPropertyCategory::Set:
    {
      wdAbstractSetProperty* pSpecific = static_cast<wdAbstractSetProperty*>(pProp);
      if (!pSource->m_Value.IsA<wdVariantArray>())
        return;

      const wdVariantArray& array = pSource->m_Value.Get<wdVariantArray>();

      // Delete old values
      if (pProp->GetFlags().AreAllSet(wdPropertyFlags::Pointer | wdPropertyFlags::PointerOwner))
      {
        wdHybridArray<wdVariant, 16> keys;
        pSpecific->GetValues(pObject, keys);
        pSpecific->Clear(pObject);
        for (wdVariant& value : keys)
        {
          void* pOldObject = value.ConvertTo<void*>();
          if (pOldObject)
            wdReflectionUtils::DeleteObject(pOldObject, pProp);
        }
      }

      pSpecific->Clear(pObject);

      if (pProp->GetFlags().IsAnySet(wdPropertyFlags::Pointer))
      {
        for (wdUInt32 i = 0; i < array.GetCount(); ++i)
        {
          if (!array[i].IsA<wdUuid>())
            continue;

          wdUuid guid = array[i].Get<wdUuid>();
          void* pRefrencedObject = nullptr;
          if (pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner))
          {
            auto* pNode = m_pGraph->GetNode(guid);
            WD_ASSERT_DEV(pNode != nullptr, "node must exist");
            pRefrencedObject = CreateObjectFromNode(pNode);
            if (pRefrencedObject == nullptr)
            {
              wdLog::Error("Failed to insert set element into property '{0}', type could not be created!", pProp->GetPropertyName());
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
          for (wdUInt32 i = 0; i < array.GetCount(); ++i)
          {
            wdReflectionUtils::InsertSetPropertyValue(pSpecific, pObject, array[i]);
          }
        }
        else if (pProp->GetFlags().IsAnySet(wdPropertyFlags::Class))
        {
          wdUuid temp;
          temp.CreateNewUuid();
          void* pValuePtr = m_pContext->CreateObject(temp, pPropType);

          for (wdUInt32 i = 0; i < array.GetCount(); ++i)
          {
            if (!array[i].IsA<wdUuid>())
              continue;

            const wdUuid sourceGuid = array[i].Get<wdUuid>();
            auto* pNode = m_pGraph->GetNode(sourceGuid);
            WD_ASSERT_DEV(pNode != nullptr, "node must exist");

            ApplyPropertiesToObject(pNode, pPropType, pValuePtr);
            pSpecific->Insert(pObject, pValuePtr);
          }

          m_pContext->DeleteObject(temp);
        }
      }
    }
    break;
    case wdPropertyCategory::Map:
    {
      wdAbstractMapProperty* pSpecific = static_cast<wdAbstractMapProperty*>(pProp);
      if (!pSource->m_Value.IsA<wdVariantDictionary>())
        return;

      const wdVariantDictionary& dict = pSource->m_Value.Get<wdVariantDictionary>();

      // Delete old values
      if (pProp->GetFlags().AreAllSet(wdPropertyFlags::Pointer | wdPropertyFlags::PointerOwner))
      {
        wdHybridArray<wdString, 16> keys;
        pSpecific->GetKeys(pObject, keys);
        for (const wdString& sKey : keys)
        {
          wdVariant value = wdReflectionUtils::GetMapPropertyValue(pSpecific, pObject, sKey);
          void* pOldClone = value.ConvertTo<void*>();
          pSpecific->Remove(pObject, sKey);
          if (pOldClone)
            wdReflectionUtils::DeleteObject(pOldClone, pProp);
        }
      }

      pSpecific->Clear(pObject);

      if (pProp->GetFlags().IsAnySet(wdPropertyFlags::Pointer))
      {
        for (auto it = dict.GetIterator(); it.IsValid(); ++it)
        {
          if (!it.Value().IsA<wdUuid>())
            continue;

          wdUuid guid = it.Value().Get<wdUuid>();
          void* pRefrencedObject = nullptr;
          if (pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner))
          {
            if (guid.IsValid())
            {
              auto* pNode = m_pGraph->GetNode(guid);
              WD_ASSERT_DEV(pNode != nullptr, "node must exist");
              pRefrencedObject = CreateObjectFromNode(pNode);
              if (pRefrencedObject == nullptr)
              {
                wdLog::Error("Failed to insert set element into property '{0}', type could not be created!", pProp->GetPropertyName());
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
            wdReflectionUtils::SetMapPropertyValue(pSpecific, pObject, it.Key(), it.Value());
          }
        }
        else if (pProp->GetFlags().IsAnySet(wdPropertyFlags::Class))
        {
          wdUuid temp;
          temp.CreateNewUuid();
          void* pValuePtr = m_pContext->CreateObject(temp, pPropType);

          for (auto it = dict.GetIterator(); it.IsValid(); ++it)
          {
            if (!it.Value().IsA<wdUuid>())
              continue;

            const wdUuid sourceGuid = it.Value().Get<wdUuid>();
            auto* pNode = m_pGraph->GetNode(sourceGuid);
            WD_ASSERT_DEV(pNode != nullptr, "node must exist");

            ApplyPropertiesToObject(pNode, pPropType, pValuePtr);
            pSpecific->Insert(pObject, it.Key(), pValuePtr);
          }

          m_pContext->DeleteObject(temp);
        }
      }
    }
    break;

    default:
      WD_ASSERT_NOT_IMPLEMENTED;
      break;
  }
}

void wdRttiConverterReader::CallOnObjectCreated(const wdAbstractObjectNode* pNode, const wdRTTI* pRtti, void* pObject)
{
  wdArrayPtr<wdAbstractFunctionProperty*> functions = pRtti->GetFunctions();
  for (wdAbstractFunctionProperty* pFunc : functions)
  {
    // TODO: Make this compare faster
    if (wdStringUtils::IsEqual(pFunc->GetPropertyName(), "OnObjectCreated"))
    {
      wdHybridArray<wdVariant, 1> params;
      params.PushBack(wdVariant(pNode));
      wdVariant ret;
      pFunc->Execute(pObject, params, ret);
    }
  }
}

WD_STATICLINK_FILE(Foundation, Foundation_Serialization_Implementation_RttiConverterReader);
