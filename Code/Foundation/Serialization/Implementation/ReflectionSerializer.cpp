#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/OpenDdlReader.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <Foundation/Serialization/BinarySerializer.h>
#include <Foundation/Serialization/DdlSerializer.h>
#include <Foundation/Serialization/ReflectionSerializer.h>
#include <Foundation/Serialization/RttiConverter.h>
#include <Foundation/Types/ScopeExit.h>
#include <Foundation/Types/VariantTypeRegistry.h>

////////////////////////////////////////////////////////////////////////
// wdReflectionSerializer public static functions
////////////////////////////////////////////////////////////////////////

void wdReflectionSerializer::WriteObjectToDDL(wdStreamWriter& inout_stream, const wdRTTI* pRtti, const void* pObject, bool bCompactMmode /*= true*/, wdOpenDdlWriter::TypeStringMode typeMode /*= wdOpenDdlWriter::TypeStringMode::Shortest*/)
{
  wdAbstractObjectGraph graph;
  wdRttiConverterContext context;
  wdRttiConverterWriter conv(&graph, &context, false, true);

  wdUuid guid;
  guid.CreateNewUuid();

  context.RegisterObject(guid, pRtti, const_cast<void*>(pObject));
  conv.AddObjectToGraph(pRtti, const_cast<void*>(pObject), "root");

  wdAbstractGraphDdlSerializer::Write(inout_stream, &graph, nullptr, bCompactMmode, typeMode);
}

void wdReflectionSerializer::WriteObjectToDDL(wdOpenDdlWriter& ref_ddl, const wdRTTI* pRtti, const void* pObject, wdUuid guid /*= wdUuid()*/)
{
  wdAbstractObjectGraph graph;
  wdRttiConverterContext context;
  wdRttiConverterWriter conv(&graph, &context, false, true);

  if (!guid.IsValid())
    guid.CreateNewUuid();

  context.RegisterObject(guid, pRtti, const_cast<void*>(pObject));
  conv.AddObjectToGraph(pRtti, const_cast<void*>(pObject), "root");

  wdAbstractGraphDdlSerializer::Write(ref_ddl, &graph, nullptr);
}

void wdReflectionSerializer::WriteObjectToBinary(wdStreamWriter& inout_stream, const wdRTTI* pRtti, const void* pObject)
{
  wdAbstractObjectGraph graph;
  wdRttiConverterContext context;
  wdRttiConverterWriter conv(&graph, &context, false, true);

  wdUuid guid;
  guid.CreateNewUuid();

  context.RegisterObject(guid, pRtti, const_cast<void*>(pObject));
  conv.AddObjectToGraph(pRtti, const_cast<void*>(pObject), "root");

  wdAbstractGraphBinarySerializer::Write(inout_stream, &graph);
}

void* wdReflectionSerializer::ReadObjectFromDDL(wdStreamReader& inout_stream, const wdRTTI*& ref_pRtti)
{
  wdOpenDdlReader reader;
  if (reader.ParseDocument(inout_stream, 0, wdLog::GetThreadLocalLogSystem()).Failed())
  {
    wdLog::Error("Failed to parse DDL graph");
    return nullptr;
  }

  return ReadObjectFromDDL(reader.GetRootElement(), ref_pRtti);
}

void* wdReflectionSerializer::ReadObjectFromDDL(const wdOpenDdlReaderElement* pRootElement, const wdRTTI*& ref_pRtti)
{
  wdAbstractObjectGraph graph;
  wdRttiConverterContext context;

  wdAbstractGraphDdlSerializer::Read(pRootElement, &graph).IgnoreResult();

  wdRttiConverterReader convRead(&graph, &context);
  auto* pRootNode = graph.GetNodeByName("root");

  WD_ASSERT_DEV(pRootNode != nullptr, "invalid document");

  ref_pRtti = wdRTTI::FindTypeByName(pRootNode->GetType());

  void* pTarget = context.CreateObject(pRootNode->GetGuid(), ref_pRtti);

  convRead.ApplyPropertiesToObject(pRootNode, ref_pRtti, pTarget);

  return pTarget;
}

void* wdReflectionSerializer::ReadObjectFromBinary(wdStreamReader& inout_stream, const wdRTTI*& ref_pRtti)
{
  wdAbstractObjectGraph graph;
  wdRttiConverterContext context;

  wdAbstractGraphBinarySerializer::Read(inout_stream, &graph);

  wdRttiConverterReader convRead(&graph, &context);
  auto* pRootNode = graph.GetNodeByName("root");

  WD_ASSERT_DEV(pRootNode != nullptr, "invalid document");

  ref_pRtti = wdRTTI::FindTypeByName(pRootNode->GetType());

  void* pTarget = context.CreateObject(pRootNode->GetGuid(), ref_pRtti);

  convRead.ApplyPropertiesToObject(pRootNode, ref_pRtti, pTarget);

  return pTarget;
}

void wdReflectionSerializer::ReadObjectPropertiesFromDDL(wdStreamReader& inout_stream, const wdRTTI& rtti, void* pObject)
{
  wdAbstractObjectGraph graph;
  wdRttiConverterContext context;

  wdAbstractGraphDdlSerializer::Read(inout_stream, &graph).IgnoreResult();

  wdRttiConverterReader convRead(&graph, &context);
  auto* pRootNode = graph.GetNodeByName("root");

  WD_ASSERT_DEV(pRootNode != nullptr, "invalid document");

  if (pRootNode == nullptr)
    return;

  convRead.ApplyPropertiesToObject(pRootNode, &rtti, pObject);
}

void wdReflectionSerializer::ReadObjectPropertiesFromBinary(wdStreamReader& inout_stream, const wdRTTI& rtti, void* pObject)
{
  wdAbstractObjectGraph graph;
  wdRttiConverterContext context;

  wdAbstractGraphBinarySerializer::Read(inout_stream, &graph);

  wdRttiConverterReader convRead(&graph, &context);
  auto* pRootNode = graph.GetNodeByName("root");

  WD_ASSERT_DEV(pRootNode != nullptr, "invalid document");

  convRead.ApplyPropertiesToObject(pRootNode, &rtti, pObject);
}


namespace
{
  static void CloneProperty(const void* pObject, void* pClone, wdAbstractProperty* pProp)
  {
    if (pProp->GetFlags().IsSet(wdPropertyFlags::ReadOnly))
      return;

    const wdRTTI* pPropType = pProp->GetSpecificType();

    const bool bIsValueType = wdReflectionUtils::IsValueType(pProp);

    wdVariant vTemp;
    switch (pProp->GetCategory())
    {
      case wdPropertyCategory::Member:
      {
        wdAbstractMemberProperty* pSpecific = static_cast<wdAbstractMemberProperty*>(pProp);

        if (pProp->GetFlags().IsSet(wdPropertyFlags::Pointer))
        {
          vTemp = wdReflectionUtils::GetMemberPropertyValue(pSpecific, pObject);

          void* pRefrencedObject = vTemp.ConvertTo<void*>();
          if (pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner) && pRefrencedObject)
          {
            pRefrencedObject = wdReflectionSerializer::Clone(pRefrencedObject, pPropType);
            vTemp = wdVariant(pRefrencedObject, pPropType);
          }

          wdVariant vOldValue = wdReflectionUtils::GetMemberPropertyValue(pSpecific, pClone);
          wdReflectionUtils::SetMemberPropertyValue(pSpecific, pClone, vTemp);
          if (pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner))
            wdReflectionUtils::DeleteObject(vOldValue.ConvertTo<void*>(), pProp);
        }
        else
        {
          if (bIsValueType || pProp->GetFlags().IsAnySet(wdPropertyFlags::IsEnum | wdPropertyFlags::Bitflags))
          {
            vTemp = wdReflectionUtils::GetMemberPropertyValue(pSpecific, pObject);
            wdReflectionUtils::SetMemberPropertyValue(pSpecific, pClone, vTemp);
          }
          else if (pProp->GetFlags().IsSet(wdPropertyFlags::Class))
          {
            void* pSubObject = pSpecific->GetPropertyPointer(pObject);
            // Do we have direct access to the property?
            if (pSubObject != nullptr)
            {
              void* pSubClone = pSpecific->GetPropertyPointer(pClone);
              wdReflectionSerializer::Clone(pSubObject, pSubClone, pPropType);
            }
            // If the property is behind an accessor, we need to retrieve it first.
            else if (pPropType->GetAllocator()->CanAllocate())
            {
              pSubObject = pPropType->GetAllocator()->Allocate<void>();
              pSpecific->GetValuePtr(pObject, pSubObject);
              pSpecific->SetValuePtr(pClone, pSubObject);
              pPropType->GetAllocator()->Deallocate(pSubObject);
            }
          }
        }
      }
      break;
      case wdPropertyCategory::Array:
      {
        wdAbstractArrayProperty* pSpecific = static_cast<wdAbstractArrayProperty*>(pProp);
        // Delete old values
        if (pProp->GetFlags().AreAllSet(wdPropertyFlags::Pointer | wdPropertyFlags::PointerOwner))
        {
          const wdInt32 iCloneCount = (wdInt32)pSpecific->GetCount(pClone);
          for (wdInt32 i = iCloneCount - 1; i >= 0; --i)
          {
            void* pOldSubClone = nullptr;
            pSpecific->GetValue(pClone, i, &pOldSubClone);
            pSpecific->Remove(pClone, i);
            if (pOldSubClone)
              wdReflectionUtils::DeleteObject(pOldSubClone, pProp);
          }
        }

        const wdUInt32 uiCount = pSpecific->GetCount(pObject);
        pSpecific->SetCount(pClone, uiCount);
        if (pSpecific->GetFlags().IsSet(wdPropertyFlags::Pointer))
        {
          for (wdUInt32 i = 0; i < uiCount; ++i)
          {
            vTemp = wdReflectionUtils::GetArrayPropertyValue(pSpecific, pObject, i);
            void* pRefrencedObject = vTemp.ConvertTo<void*>();
            if (pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner) && pRefrencedObject)
            {
              pRefrencedObject = wdReflectionSerializer::Clone(pRefrencedObject, pPropType);
              vTemp = wdVariant(pRefrencedObject, pPropType);
            }
            wdReflectionUtils::SetArrayPropertyValue(pSpecific, pClone, i, vTemp);
          }
        }
        else
        {
          if (bIsValueType)
          {
            for (wdUInt32 i = 0; i < uiCount; ++i)
            {
              vTemp = wdReflectionUtils::GetArrayPropertyValue(pSpecific, pObject, i);
              wdReflectionUtils::SetArrayPropertyValue(pSpecific, pClone, i, vTemp);
            }
          }
          else if (pProp->GetFlags().IsSet(wdPropertyFlags::Class) && pPropType->GetAllocator()->CanAllocate())
          {
            void* pSubObject = pPropType->GetAllocator()->Allocate<void>();

            for (wdUInt32 i = 0; i < uiCount; ++i)
            {
              pSpecific->GetValue(pObject, i, pSubObject);
              pSpecific->SetValue(pClone, i, pSubObject);
            }

            pPropType->GetAllocator()->Deallocate(pSubObject);
          }
        }
      }
      break;
      case wdPropertyCategory::Set:
      {
        wdAbstractSetProperty* pSpecific = static_cast<wdAbstractSetProperty*>(pProp);

        // Delete old values
        if (pProp->GetFlags().AreAllSet(wdPropertyFlags::Pointer | wdPropertyFlags::PointerOwner))
        {
          wdHybridArray<wdVariant, 16> keys;
          pSpecific->GetValues(pClone, keys);
          pSpecific->Clear(pClone);
          for (wdVariant& value : keys)
          {
            void* pOldClone = value.ConvertTo<void*>();
            if (pOldClone)
              wdReflectionUtils::DeleteObject(pOldClone, pProp);
          }
        }
        pSpecific->Clear(pClone);

        wdHybridArray<wdVariant, 16> values;
        pSpecific->GetValues(pObject, values);


        if (pProp->GetFlags().IsSet(wdPropertyFlags::Pointer))
        {
          for (wdUInt32 i = 0; i < values.GetCount(); ++i)
          {
            void* pRefrencedObject = values[i].ConvertTo<void*>();
            if (pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner) && pRefrencedObject)
            {
              pRefrencedObject = wdReflectionSerializer::Clone(pRefrencedObject, pPropType);
            }
            vTemp = wdVariant(pRefrencedObject, pPropType);
            wdReflectionUtils::InsertSetPropertyValue(pSpecific, pClone, vTemp);
          }
        }
        else if (bIsValueType)
        {
          for (wdUInt32 i = 0; i < values.GetCount(); ++i)
          {
            wdReflectionUtils::InsertSetPropertyValue(pSpecific, pClone, values[i]);
          }
        }
      }
      break;
      case wdPropertyCategory::Map:
      {
        wdAbstractMapProperty* pSpecific = static_cast<wdAbstractMapProperty*>(pProp);

        // Delete old values
        if (pProp->GetFlags().AreAllSet(wdPropertyFlags::Pointer | wdPropertyFlags::PointerOwner))
        {
          wdHybridArray<wdString, 16> keys;
          pSpecific->GetKeys(pClone, keys);
          for (const wdString& sKey : keys)
          {
            wdVariant value = wdReflectionUtils::GetMapPropertyValue(pSpecific, pClone, sKey);
            void* pOldClone = value.ConvertTo<void*>();
            pSpecific->Remove(pClone, sKey);
            if (pOldClone)
              wdReflectionUtils::DeleteObject(pOldClone, pProp);
          }
        }
        pSpecific->Clear(pClone);

        wdHybridArray<wdString, 16> keys;
        pSpecific->GetKeys(pObject, keys);

        for (wdUInt32 i = 0; i < keys.GetCount(); ++i)
        {
          if (bIsValueType ||
              (pProp->GetFlags().IsSet(wdPropertyFlags::Pointer) && !pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner)))
          {
            wdVariant value = wdReflectionUtils::GetMapPropertyValue(pSpecific, pObject, keys[i]);
            wdReflectionUtils::SetMapPropertyValue(pSpecific, pClone, keys[i], value);
          }
          else if (pProp->GetFlags().IsSet(wdPropertyFlags::Class))
          {
            if (pProp->GetFlags().IsSet(wdPropertyFlags::Pointer))
            {
              void* pValue = nullptr;
              pSpecific->GetValue(pObject, keys[i], &pValue);
              pValue = wdReflectionSerializer::Clone(pValue, pPropType);
              pSpecific->Insert(pClone, keys[i], &pValue);
            }
            else
            {
              if (pPropType->GetAllocator()->CanAllocate())
              {
                void* pValue = pPropType->GetAllocator()->Allocate<void>();
                WD_SCOPE_EXIT(pPropType->GetAllocator()->Deallocate(pValue););
                WD_VERIFY(pSpecific->GetValue(pObject, keys[i], pValue), "Previously retrieved key does not exist.");
                pSpecific->Insert(pClone, keys[i], pValue);
              }
              else
              {
                wdLog::Error("The property '{0}' can not be cloned as the type '{1}' cannot be allocated.", pProp->GetPropertyName(), pPropType->GetTypeName());
              }
            }
          }
        }
      }
      break;
      default:
        break;
    }
  }

  static void CloneProperties(const void* pObject, void* pClone, const wdRTTI* pType)
  {
    if (pType->GetParentType())
      CloneProperties(pObject, pClone, pType->GetParentType());

    for (auto* pProp : pType->GetProperties())
    {
      CloneProperty(pObject, pClone, pProp);
    }
  }
} // namespace

void* wdReflectionSerializer::Clone(const void* pObject, const wdRTTI* pType)
{
  if (!pObject)
    return nullptr;

  WD_ASSERT_DEV(pType != nullptr, "invalid type.");
  if (pType->IsDerivedFrom<wdReflectedClass>())
  {
    const wdReflectedClass* pRefObject = static_cast<const wdReflectedClass*>(pObject);
    pType = pRefObject->GetDynamicRTTI();
  }

  WD_ASSERT_DEV(pType->GetAllocator()->CanAllocate(), "The type '{0}' can't be cloned!", pType->GetTypeName());
  void* pClone = pType->GetAllocator()->Allocate<void>();
  CloneProperties(pObject, pClone, pType);
  return pClone;
}


void wdReflectionSerializer::Clone(const void* pObject, void* pClone, const wdRTTI* pType)
{
  WD_ASSERT_DEV(pObject && pClone && pType, "invalid type.");
  if (pType->IsDerivedFrom<wdReflectedClass>())
  {
    const wdReflectedClass* pRefObject = static_cast<const wdReflectedClass*>(pObject);
    pType = pRefObject->GetDynamicRTTI();
    WD_ASSERT_DEV(pType == static_cast<wdReflectedClass*>(pClone)->GetDynamicRTTI(), "Object '{0}' and clone '{1}' have mismatching types!", pType->GetTypeName(), static_cast<wdReflectedClass*>(pClone)->GetDynamicRTTI()->GetTypeName());
  }

  CloneProperties(pObject, pClone, pType);
}

WD_STATICLINK_FILE(Foundation, Foundation_Serialization_Implementation_ReflectionSerializer);
